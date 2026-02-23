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
- `mailbox_entry_t` struct
- Declarations for core async functions:
  - `CreateRequestAsync()`
  - `CreateRequestAsyncWithPayload()`
  - `SendRequestAsync()`
  - `PK_ReceiveAndDispatch()`
  - `PK_TimeoutAndRetryCheck()`
- Declarations for all `PK_*Async()` functions

**MUST NOT contain**:
- Function implementations
- HAL pin export logic
- Structs or types that should live in `PoKeysLibHal.h`

---

### `PoKeysLibAsync.c` — Async Infrastructure Implementation

**Purpose**: Implements the core asynchronous request/response mailbox system used by all `PoKeysLib**Async.c` modules.

**MUST contain**:
- Implementation of `CreateRequestAsync()`
- Implementation of `CreateRequestAsyncWithPayload()`
- Implementation of `SendRequestAsync()`
- Implementation of `PK_ReceiveAndDispatch()`
- Implementation of `PK_TimeoutAndRetryCheck()`
- Mailbox management (buffer allocation, slot tracking)

**MUST NOT contain**:
- HAL pin export functions (those belong in `PoKeysLib**Async.c`)
- Subsystem-specific parsing (belongs in the respective `PoKeysLib**Async.c`)

---

### `PoKeysLib**Async.c` — Per-Subsystem Async Files

**Examples**: `PoKeysLibIOAsync.c`, `PoKeysLibEncodersAsync.c`, `PoKeysLibPulseEngine_v2Async.c`, `PoKeysLibRTCAsync.c`, `PoKeysLibPoNETHal.c`, etc.

**Purpose**: Each file provides the async implementation for its corresponding subsystem (IO, encoders, PulseEngine, etc.) plus the HAL pin/parameter export for that subsystem.

**MUST contain**:
```c
/**
 * Export HAL pins and parameters for this subsystem.
 *
 * @param prefix  HAL component name prefix (e.g., "pokeys")
 * @param comp_id HAL component ID
 * @param device  Pointer to the PoKeys device struct
 * @return 0 on success, negative HAL error code on failure
 */
int export_<subsystem>_pins(const char *prefix, long comp_id, sPoKeysDevice *device);
```

The function uses `hal_pin_*_new()` / `hal_param_*_new()` and `hal_export_digin()` / `hal_export_digout()` / `hal_export_adcin()` / `hal_export_adcout()` / `hal_export_encoder()` from `hal-canon/hal_canon.h`.

- Async request functions for the subsystem (e.g., `PK_DigitalIOGetAsync()`)
- Response parser functions for the subsystem
- (Optionally) a register function that schedules async tasks

**MUST NOT contain**:
- Direct exports of HAL pins that belong to another subsystem
- Definitions of structs/enums/constants that belong in `PoKeysLibHal.h` or `PoKeysLibAsync.h`

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
