---
description: "Architecture enforcement rules for the PoKeysHal project. Defines the role, responsibilities, and constraints for each key file and module. Applies to all C and header source files."
applyTo:
  - "**/*.c"
  - "**/*.h"
  - "experimental/*.c"
  - "experimental/*.h"
---

# PoKeysHal Architecture Reference & Enforcement Rules

**Standards**: ISO/IEC/IEEE 42010:2011 (Architecture Description), IEEE 1016-2009 (Software Design Descriptions)  
**Scope**: All C source (`.c`) and header (`.h`) files in this repository

---

## 🏗️ Architecture Overview

The PoKeysHal project is structured as a layered HAL (Hardware Abstraction Layer) driver for PoKeys USB/Ethernet devices. The architecture consists of distinct layers with well-defined responsibilities:

```
┌───────────────────────────────────────────────────────────┐
│            experimental/pokeys_async.c                    │
│  (LinuxCNC HAL component: entry points only)              │
├───────────────────────────────────────────────────────────┤
│            PoKeysLib**Async.c  (per subsystem)            │
│  (HAL pin export + subsystem async implementation)        │
├───────────────────────────────────────────────────────────┤
│    PoKeysLibAsync.h / PoKeysLibAsync.c                    │
│  (Async infrastructure: mailbox, dispatch, retry)         │
├───────────────────────────────────────────────────────────┤
│    PoKeysLibHal.h                                         │
│  (HAL-conform structs using hal_* types + canon ifaces)   │
├───────────────────────────────────────────────────────────┤
│    hal-canon/hal_canon.h / hal-canon/*.c                  │
│  (Canonical HAL device interface definitions)             │
└───────────────────────────────────────────────────────────┘
```

### Async Execution Model — Two Concurrent Flows

The async infrastructure separates the RT servo-thread from the UDP receive path. Both flows share the mailbox buffer, coordinated only via atomic flags (no mutexes in RT):

```plaintext
[RT-Thread — called every servo period]   [PK_ReceiveAndDispatch — non-blocking poll]
               │                                           │
               │ CreateRequestAsync()                      │
               │ SendRequestAsync()       UDP Packet ──────┤
               │ mailbox_insert()         arrives          │
               │                                           ├─► Find matching mailbox entry
               │                                           ├─► Copy payload → target_ptr
               │                                           ├─► Invoke parse callback
               │                                           └─► Set response_ready = true
               │
               ├─► Poll mailbox each cycle
               ├─► If response_ready: consume data, clear slot
               └─► PK_TimeoutAndRetryCheck()
                     ├─► timeout & retries_left > 0: resend, decrement retries
                     └─► timeout & retries_left == 0: set error status
```

---

## 📋 File Responsibilities

### `PoKeysLibHal.h` — HAL-Conform Type Definitions & Canonical Interfaces

**Purpose**: Defines all structs used in the HAL layer using LinuxCNC HAL-compatible types so that struct members can be used directly as HAL pins or parameters.

**MUST contain**:
- All device data structs using HAL-conform variable types:
  - `hal_u32_t` instead of `uint32_t`
  - `hal_s32_t` instead of `int32_t`
  - `hal_float_t` instead of `float`
  - `hal_bit_t` instead of `bool`
- Canonical interface struct includes from `hal-canon/hal_canon.h`:
  - `hal_digin_t` — digital input channel
  - `hal_digout_t` — digital output channel
  - `hal_adcin_t` — analog input channel
  - `hal_adcout_t` — analog output channel
  - `hal_encoder_t` — encoder channel
- Bitfields expanded to separate `hal_bit_t` members so each bit is individually available as a HAL pin (example: `encoderOptions` fields in `sPoKeysEncoder`)

**MUST NOT contain**:
- Use of raw C types (`uint32_t`, `int32_t`, `float`, `bool`) for HAL-exposed struct members
- Logic or function implementations
- Async infrastructure declarations (those belong in `PoKeysLibAsync.h`)

---

### `PoKeysLibAsync.h` — Async Infrastructure Declarations

**Purpose**: Contains all enums, structs, and function declarations for the asynchronous request/response infrastructure.

**MUST contain**:
- `transaction_status_t` enum
- `pokeys_command_t` enum (all PoKeys protocol command codes)
- `PEv2_command_t` enum
- `async_transaction_t` struct
- `mailbox_entry_t` struct — central data structure for pending async requests:

```c
typedef struct {
    uint8_t request_id;               // Cyclic request ID (0–255)
    pokeys_command_t command_sent;    // Sent command code
    uint8_t subindex;                 // Optional sub-index (e.g., encoder number)
    uint64_t timestamp_sent;          // Timestamp at send time (µs)
    int retries_left;                 // Remaining retry count
    bool response_ready;              // Set to true when response is received

    void *target_ptr;                 // Pointer to HAL pin or target memory;
                                      // if set, response payload (from byte 8)
                                      // is auto-copied here — no manual parsing needed
    size_t target_size;               // Expected size of response data

    uint8_t request_buffer[64];       // Outgoing packet buffer
    uint8_t response_buffer[64];      // Incoming response buffer
} mailbox_entry_t;
```

- Declarations for core async functions with canonical signatures:

```c
/**
 * Prepares a request packet and creates a mailbox entry.
 * Builds the UDP packet (header, command, request ID, checksum).
 * No network activity at this stage.
 *
 * @param dev         PoKeys device handle
 * @param cmd         Protocol command to send
 * @param params      Command parameters buffer (may be NULL)
 * @param params_len  Length of params buffer
 * @param target_ptr  Optional destination for auto-copy of response payload
 * @param target_size Size of target_ptr buffer
 * @return            Assigned request_id on success, negative on error
 */
int CreateRequestAsync(pokeys_device_t *dev, pokeys_command_t cmd,
                       const uint8_t *params, size_t params_len,
                       void *target_ptr, size_t target_size);

/**
 * Variant of CreateRequestAsync with an explicit additional payload buffer.
 */
int CreateRequestAsyncWithPayload(pokeys_device_t *dev, pokeys_command_t cmd,
                                  const uint8_t *params, size_t params_len,
                                  const uint8_t *payload, size_t payload_len,
                                  void *target_ptr, size_t target_size);

/**
 * Sends a previously prepared request over UDP (non-blocking sendto).
 * Sets timestamp_sent and increments the retry counter.
 *
 * @param dev        PoKeys device handle
 * @param request_id ID returned by CreateRequestAsync()
 * @return           0 on success, negative on error
 */
int SendRequestAsync(pokeys_device_t *dev, uint8_t request_id);

/**
 * Receives one UDP packet in non-blocking mode, extracts the request ID,
 * finds the matching mailbox entry, auto-copies the payload into target_ptr
 * (if set), invokes the registered parser callback, and sets response_ready.
 *
 * @param dev  PoKeys device handle
 * @return     1 if a packet was processed, 0 if nothing received, negative on error
 */
int PK_ReceiveAndDispatch(pokeys_device_t *dev);

/**
 * Checks all open mailbox entries for timeout.
 * If (current_time - timestamp_sent) > timeout_us and retries_left > 0,
 * re-sends the request; otherwise marks the entry as failed.
 *
 * @param dev        PoKeys device handle
 * @param timeout_us Timeout threshold in microseconds
 */
void PK_TimeoutAndRetryCheck(pokeys_device_t *dev, uint64_t timeout_us);
```

- Declarations for all `PK_*Async()` functions

**MUST NOT contain**:
- Function implementations
- HAL pin export logic
- Structs or types that should live in `PoKeysLibHal.h`

---

### `PoKeysLibAsync.c` — Async Infrastructure Implementation (**infrastructure only**)

> ⚠️ **This file is the async infrastructure layer. It MUST NOT contain any subsystem-specific logic, HAL exports, or command parsers.**

**Purpose**: Implements the core asynchronous request/response mailbox system that is shared by **all** `PoKeysLib**Async.c` subsystem modules. Think of it as the "transport layer" — it knows how to send, receive, queue, and retry packets, but it knows nothing about what is inside them.

**MUST contain**:

- Implementation of `CreateRequestAsync()` — allocate a mailbox slot and build a request packet
- Implementation of `CreateRequestAsyncWithPayload()` — same as above, with an additional payload buffer
- Implementation of `SendRequestAsync()` — transmit the prepared packet over UDP (non-blocking)
- Implementation of `PK_ReceiveAndDispatch()` — receive UDP response, match to mailbox entry, invoke the registered parser callback
- Implementation of `PK_TimeoutAndRetryCheck()` — detect timed-out transactions, retry or mark as failed
- Mailbox management: slot allocation, slot reset, request-ID cycling

**MUST NOT contain**:
- HAL pin export functions → belong in the subsystem `PoKeysLib**Async.c`
- Subsystem-specific response parsers (e.g., `PK_DigitalIOGetParse`) → belong in `PoKeysLibIOAsync.c`
- `export_<subsystem>_pins()` functions → belong in the subsystem `PoKeysLib**Async.c`
- Any `#include "hal.h"` or direct use of `hal_pin_*_new()` / `hal_param_*_new()`

**Relationship to `PoKeysLib**Async.c`**:  
`PoKeysLibAsync.c` is the **called** infrastructure; `PoKeysLib**Async.c` files are the **callers** that build on top of it.

**Step-by-step async request/response workflow** (all four functions in sequence):

```plaintext
Step 1 — CreateRequestAsync()
  • Build the UDP packet (header, command code, request ID, checksum)
  • Allocate a mailbox slot; store target_ptr/target_size for auto-copy
  • No network activity at this stage

Step 2 — SendRequestAsync()
  • Transmit via non-blocking UDP sendto()
  • Record timestamp_sent; increment retry counter

Step 3 — PK_ReceiveAndDispatch()   [called every RT cycle or via select()]
  • recvfrom() in non-blocking mode — returns immediately if no data
  • Extract request_id from incoming packet
  • Locate matching mailbox entry
  • If target_ptr is set: memcpy payload (bytes 8+) → target_ptr automatically
  • Invoke registered parse callback (subsystem-specific, if any)
  • Set response_ready = true; release mailbox slot

Step 4 — PK_TimeoutAndRetryCheck()   [called every RT cycle]
  • For every open mailbox entry:
      if (now − timestamp_sent) > timeout_us:
          if retries_left > 0:  re-SendRequestAsync(); update timestamp_sent; retries_left--
          else:                 mark slot as FAILED; release slot
```

---

### `PoKeysLib**Async.c` — Per-Subsystem Async Files (**subsystem implementations**)

> ⚠️ **These files are fundamentally different from `PoKeysLibAsync.c`.** They implement the async protocol for a specific hardware subsystem (IO, encoders, CAN, RTC, PulseEngine, etc.) and also own the HAL pin export and RT task registration for that subsystem.

**Examples**: `PoKeysLibIOAsync.c`, `PoKeysLibCANAsync.c`, `PoKeysLibEncodersAsync.c`, `PoKeysLibPulseEngine_v2Async.c`, `PoKeysLibRTCAsync.c`, `PoKeysLibPoNETHal.c`

**Purpose**: Each file provides three distinct responsibilities for its subsystem:

1. **HAL pin/parameter export** — registers HAL pins and parameters for the structs managed by this subsystem
2. **Send side** — builds and sends async requests to the device (calls `CreateRequestAsync` / `SendRequestAsync`)
3. **Parse side (response handler)** — parses the device response; this callback is invoked by `PK_ReceiveAndDispatch` in the infrastructure layer

This **Send / Parse split** is the defining characteristic of subsystem async files:
- The **Send function** (e.g., `PK_DigitalIOGetAsync()`) is called actively from the RT thread or scheduler to trigger a request.
- The **Parse function** (e.g., `PK_DigitalIOGetParse()`) is registered as a callback and called passively by `PK_ReceiveAndDispatch()` when the device response arrives.

**MUST contain**:

```c
/* 1. HAL pin export (called once during component setup) */
/**
 * Export HAL pins and parameters for the <subsystem> subsystem.
 *
 * @param prefix  HAL component name prefix (e.g., "pokeys")
 * @param comp_id HAL component ID returned by hal_init()
 * @param device  Pointer to the PoKeys device struct
 * @return 0 on success, negative HAL error code on failure
 */
int export_<subsystem>_pins(const char *prefix, long comp_id, sPoKeysDevice *device);

/* 2. Send function — called from RT thread / scheduler to issue a request */
int PK_<Subsystem>GetAsync(sPoKeysDevice *device);   /* example */

/* 3. Parse function — registered as callback, called by PK_ReceiveAndDispatch */
static int PK_<Subsystem>GetParse(sPoKeysDevice *device, const uint8_t *response);

/* 4. (Optional) RT task registration — registers Send function with async_scheduler */
int register_<subsystem>_tasks(sPoKeysDevice *device);
```

The `export_<subsystem>_pins()` function uses:
- `hal_pin_*_new()` / `hal_param_*_new()` for PoKeys-specific pins
- `hal_export_digin()` / `hal_export_digout()` / `hal_export_adcin()` / `hal_export_adcout()` / `hal_export_encoder()` from `hal-canon/hal_canon.h` for canonical channel types

**Functional flow inside a subsystem async file**:
```
[RT Thread / Scheduler]
       │
       ▼
   PK_<Subsystem>GetAsync(device)          ← Send side
       │  CreateRequestAsync(cmd, parser_fn)
       │  SendRequestAsync(req_id)
       ▼
   (packet in flight …)

[PK_ReceiveAndDispatch — in PoKeysLibAsync.c]
       │  receives UDP response
       │  looks up mailbox entry by request_id
       ▼
   PK_<Subsystem>GetParse(device, response) ← Parse side (callback)
       │  fills device->Subsystem.* fields (HAL pins updated)
```

**MUST NOT contain**:
- Implementations of `CreateRequestAsync`, `SendRequestAsync`, `PK_ReceiveAndDispatch`, or `PK_TimeoutAndRetryCheck` — those implementations live in `PoKeysLibAsync.c`; subsystem files only **call** them
- Direct exports of HAL pins that belong to another subsystem
- Definitions of structs/enums/constants that belong in `PoKeysLibHal.h` or `PoKeysLibAsync.h`

**Command dispatching via function pointers** (optional but recommended for clean extension):

Subsystem parse callbacks can be registered in a dispatch table keyed on the command code, so `PK_ReceiveAndDispatch` calls the right handler automatically:

```c
/* In PoKeysLibAsync.h — typedef for per-command response handlers */
typedef int (*pokeys_response_handler_t)(mailbox_entry_t *entry, const uint8_t *rx_buffer);

/* In PoKeysLibAsync.c — dispatch table indexed by pokeys_command_t */
pokeys_response_handler_t response_handlers[] = {
    [PK_CMD_ENCODER_VALUES_GET]  = handle_encoder_response,   /* PoKeysLibEncodersAsync.c */
    [PK_CMD_DIGITAL_INPUTS_GET]  = handle_inputs_response,    /* PoKeysLibIOAsync.c */
    /* ... additional subsystem handlers registered here ... */
};
```

Each subsystem `PoKeysLib**Async.c` provides its own `handle_*_response()` function and registers it during `EXTRA_SETUP()`. The infrastructure layer calls `response_handlers[cmd](entry, rx_buffer)` — it never needs to know the subsystem internals.

---

### `experimental/pokeys_async.c` — LinuxCNC HAL Component Entry Points

**Purpose**: This is the autogenerated-compatible LinuxCNC HAL component source. It acts as the **integration shell** that ties together all subsystem modules. It is the top-level component file compiled by `halcompile`.

**MUST contain** (and ONLY these):

1. **`static int export(char *prefix, long extra_arg)`**  
   Calls `export_<subsystem>_pins()` from each `PoKeysLib**Async.c`:
   ```c
   static int export(char *prefix, long extra_arg) {
       int r;
       r = export_IO_pins(prefix, comp_id, inst->dev);
       if (r) return r;
       r = export_encoder_pins(prefix, comp_id, inst->dev);
       if (r) return r;
       r = export_pev2_pins(prefix, comp_id, inst->dev);  // in PoKeysLibPulseEngine_v2Async.c
       // ... etc.
       return 0;
   }
   ```

2. **`EXTRA_SETUP()` / `static int extra_setup(...)`**  
   Connects to the PoKeys device and registers async tasks with the scheduler.

3. **`user_mainloop(void)`**  
   Userspace main loop (called only when built as ULAPI).

4. **`FUNCTION(_)` / `static void _(struct __comp_state *__comp_inst, long period)`**  
   RT servo-thread function that:
   - Calls `PK_ReceiveAndDispatch(__comp_inst->dev)`
   - Calls `PK_TimeoutAndRetryCheck(__comp_inst->dev, 1000)`
   - Calls `rt_update_performance_monitoring(__comp_inst, start_time)`

**MUST NOT contain**:
- Direct HAL pin exports (`hal_pin_*_new()`, `hal_param_*_new()`, `hal_export_*()`) — these belong in `export_<subsystem>_pins()` in `PoKeysLib**Async.c`
- Definitions of structs (e.g., `typedef struct { ... } my_type_t;`) — these belong in `PoKeysLibHal.h` or `PoKeysLibAsync.h`
- Definitions of enumerations (e.g., `typedef enum { ... } my_enum_t;`) — these belong in `PoKeysLibHal.h` or `PoKeysLibAsync.h`
- Named constants (`#define MY_CONSTANT value`) specific to a subsystem — these belong in the relevant header

**Allowed** in `experimental/pokeys_async.c`:
- `#include` directives
- The `struct __comp_state` definition (required by halcompile infrastructure — the HAL component instance state; this is an **exception** since halcompile requires it in the component file)
- Helper macros generated by halcompile (`FUNCTION`, `EXTRA_SETUP`, `FOR_ALL_INSTS`, pin accessor macros)
- Static helper functions that are purely glue/integration logic and not tied to a specific subsystem
- `#ifdef RTAPI` / `#ifndef RTAPI` conditional blocks

---

### `experimental/async_scheduler.h` & `experimental/async_scheduler.c` — Async Scheduler

**Current status**: Lives in `experimental/`. Content **should be migrated** to `PoKeysLibAsync.h` / `PoKeysLibAsync.c` to make it generally available.

**Until migration**:
- `register_async_task()` — registers periodic async tasks
- `async_dispatcher()` — dispatches due tasks
- `async_task_set_active()` / `async_task_count()` — task management

**Migration target**:
- `periodic_async_task_t`, `async_func_t` → `PoKeysLibAsync.h`
- `register_async_task()`, `async_dispatcher()` implementations → `PoKeysLibAsync.c`

---

### `hal-canon/hal_canon.h` & `hal-canon/*.c` — Canonical HAL Interfaces

**Purpose**: Provides canonical interface structs, export macros, and register functions for standard device types following LinuxCNC's canonical device interface definitions.

**Canonical types provided**:
- `hal_digin_t` + `hal_export_digin()` — digital input
- `hal_digout_t` + `hal_export_digout()` — digital output
- `hal_adcin_t` + `hal_export_adcin()` — analog input
- `hal_adcout_t` + `hal_export_adcout()` — analog output
- `hal_encoder_t` + `hal_export_encoder()` — encoder

**Rule**: All HAL pin exports for digital/analog/encoder channels MUST use these canonical export functions rather than calling `hal_pin_*_new()` directly.

---

## ⚙️ Async Implementation Notes

These rules apply to all code in `PoKeysLibAsync.c` and `PoKeysLib**Async.c`:

| Rule | Rationale |
|------|-----------|
| Configure UDP socket with `O_NONBLOCK` | `recvfrom()` / `sendto()` must return immediately — blocking calls violate RT deadlines |
| Call `mlockall(MCL_CURRENT \| MCL_FUTURE)` before starting RT threads | Prevents page faults inside the RT path; all mailbox and HAL memory stays in RAM |
| Manage `request_id` cyclically over 0–255 | Wrap-around is expected; the mailbox must tolerate reuse after a full cycle |
| Receive path must complete in **< 5 µs** (hard RT) | `PK_ReceiveAndDispatch()` is called every servo period — it must not loop or block |
| Use **atomic flags** (not mutexes) for `response_ready` | Mutexes can block the RT thread; `_Atomic bool` or `volatile` + memory barriers are sufficient |
| Receiving strategy: cyclic poll **or** `select()`-based | Cyclic polling (every 1–5 ms) is simpler; `select()` reduces CPU load but adds latency jitter |
| No `malloc`/`free` in RT paths | All mailbox slots must be pre-allocated at startup; use fixed-size arrays |

---

## 🔧 Build & Test Quick Reference

> Per [GitHub Copilot best practices](https://gh.io/copilot-coding-agent-tips), instructions files for specific file types should include how to build and test them.

### Build C/H files

```bash
# Primary build (library + HAL component)
sudo make -f Makefile.noqmake install

# CMake build
mkdir -p build && cd build && cmake .. && make

# Build userspace HAL component
sudo halcompile --install --userspace \
    --extra-link-args="-L/usr/lib -lPoKeysHal" \
    experimental/pokeys_async.c

# Build RT component
cd experimental/build && make -f Submakefile.rt all && sudo make -f Submakefile.rt install
```

### Validate C/H files compile

```bash
# Compile test (checks all .c/.h files for syntax errors)
bash test_compile.sh
```

### Run HAL component (smoke test)

```bash
# Userspace
halrun <<'EOF'
loadusr -W pokeys_async
show pin && show funct && start && show pin && show param && exit
EOF

# Real-time
halrun <<'EOF'
loadrt threads name1=pokeys-test-thread period1=1000000
loadrt pokeys_async
addf pokeys-async.0 pokeys-test-thread
start && show pin && show param && exit
EOF
```

---

## 🚨 Architecture Violation Checklist

When reviewing or writing code, check for these violations:

### Critical Violations (MUST fix)

| # | Violation | File | Correct Location |
|---|-----------|------|-----------------|
| V1 | `typedef struct` or `typedef enum` defined in `experimental/pokeys_async.c` | `experimental/pokeys_async.c` | `PoKeysLibHal.h` or `PoKeysLibAsync.h` |
| V2 | `#define MY_CONSTANT` (non-halcompile) defined in `experimental/pokeys_async.c` | `experimental/pokeys_async.c` | Relevant `PoKeysLib*.h` |
| V3 | `hal_pin_*_new()` or `hal_param_*_new()` called directly in `experimental/pokeys_async.c` | `experimental/pokeys_async.c` | `export_<subsystem>_pins()` in `PoKeysLib**Async.c` |
| V4 | `export_<subsystem>_pins()` defined in `experimental/pokeys_async.c` | `experimental/pokeys_async.c` | Corresponding `PoKeysLib**Async.c` |
| V5 | Raw C types (`uint32_t`, `int32_t`, `float`, `bool`) in HAL-exposed struct members in `PoKeysLibHal.h` | `PoKeysLibHal.h` | Replace with `hal_u32_t`, `hal_s32_t`, `hal_float_t`, `hal_bit_t` |
| V6 | Bitfield members not expanded to individual `hal_bit_t` in `PoKeysLibHal.h` | `PoKeysLibHal.h` | Expand bitfields to separate `hal_bit_t` members |
| V7 | Direct HAL pin export in `PoKeysLibAsync.c` (infrastructure file) | `PoKeysLibAsync.c` | Move to corresponding `PoKeysLib**Async.c` |

### Known Violations Requiring GitHub Issues

The following violations were identified during code review and should be tracked as GitHub issues:

1. **[V1] `struct __comp_state` nested structs defined in `experimental/pokeys_async.c`**
   - Lines ~384–444 contain `rt_motion_data_t`, `async_cmd_type_t`, `async_command_t`, `async_command_queue_t`, device status cache struct
   - These should be moved to `PoKeysLibHal.h` or a new `PoKeysLibPulseEngine_v2Hal.h`

2. **[V2] `export_pev2_hal_pins()` defined in `experimental/pokeys_async.c`**  
   - Line ~518 defines `export_pev2_hal_pins()` which directly exports HAL pins for PulseEngine v2
   - This function should be moved to `PoKeysLibPulseEngine_v2Async.c` as `export_pev2_pins()`

3. **[V3] `#define TRUE`, `#define FALSE`, `#define true`, `#define false` in `experimental/pokeys_async.c`**
   - Lines ~157–163 redefine standard boolean constants
   - These shadow standard `<stdbool.h>` values and should be removed

4. **[V4] `#define MAX_ASYNC_COMMANDS 32` in `experimental/pokeys_async.c`**
   - Line ~415 defines a subsystem constant in the component file
   - This should be moved to `PoKeysLibAsync.h` or the relevant subsystem header

5. **[V5] `async_scheduler.h` / `async_scheduler.c` not yet integrated into `PoKeysLibAsync`**
   - Content of `experimental/async_scheduler.h` and `experimental/async_scheduler.c` should be migrated to `PoKeysLibAsync.h` / `PoKeysLibAsync.c`

---

## ✅ Always Do

✅ **Define data types in headers, not in component files** — structs, enums, and constants belong in `PoKeysLibHal.h` or `PoKeysLibAsync.h`  
✅ **Export HAL pins in `export_<subsystem>_pins()` functions** — always in the corresponding `PoKeysLib**Async.c` file  
✅ **Use canonical export functions** — `hal_export_digin()`, `hal_export_digout()`, `hal_export_adcin()`, `hal_export_adcout()`, `hal_export_encoder()` from `hal-canon/hal_canon.h`  
✅ **Use HAL-conform types in `PoKeysLibHal.h`** — `hal_u32_t`, `hal_s32_t`, `hal_float_t`, `hal_bit_t`  
✅ **Expand bitfields to individual `hal_bit_t` members** in `PoKeysLibHal.h` for direct HAL pin availability  
✅ **Keep `experimental/pokeys_async.c` as an integration shell only** — it should only call subsystem functions, not implement them  

## ❌ Never Do

❌ **Never define structs/enums/constants in `experimental/pokeys_async.c`** (except the required halcompile `struct __comp_state`)  
❌ **Never call `hal_pin_*_new()` or `hal_param_*_new()` directly in `experimental/pokeys_async.c`**  
❌ **Never define `export_<subsystem>_pins()` in `experimental/pokeys_async.c`**  
❌ **Never use raw C scalar types (`uint32_t`, `int32_t`, `float`, `bool`) for HAL-exposed struct members**  
❌ **Never mix async infrastructure code with subsystem-specific code** in `PoKeysLibAsync.c`  
❌ **Never define per-command parsers in `PoKeysLibAsync.c`** — keep these in the subsystem `PoKeysLib**Async.c` files  

---

## 🔍 Code Review Pattern

When reviewing any `PoKeysLib**Async.c` file, verify:

```
✓ File has export_<subsystem>_pins(const char *prefix, long comp_id, sPoKeysDevice *device)
✓ All HAL pins exported using hal_export_digin/digout/adcin/adcout/encoder or hal_pin_*_new
✓ No struct/enum definitions (use PoKeysLibHal.h or PoKeysLibAsync.h)
✓ No #define constants (use PoKeysLibHal.h or PoKeysLibAsync.h)
✓ Implements PK_*Async() functions for its subsystem
```

When reviewing `experimental/pokeys_async.c`, verify:

```
✓ Contains static int export(char *prefix, long extra_arg) — calls subsystem export_*_pins()
✓ Contains EXTRA_SETUP() / extra_setup() — device connection and task registration
✓ Contains user_mainloop() — userspace main loop
✓ Contains FUNCTION(_) / RT servo thread function
✗ No typedef struct/enum definitions (except struct __comp_state)
✗ No #define constants except halcompile-required macros and standard includes
✗ No hal_pin_*_new() or hal_param_*_new() direct calls
✗ No export_*_pins() function definitions
```
