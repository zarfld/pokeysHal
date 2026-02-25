## Software Design Description (IEEE 1016-2009)

**Architecture Component**: #{{ARC}} (ARC-C-ASYNC-SCHED)
**Architecture Decisions**: #{{ADR1}}, #{{ADR2}}, #{{ADR3}}, #{{ADR4}}, #{{ADR5}}
**Requirements**: #{{F001}}, #{{F002}}, #{{F003}}, #{{F004}}, #{{F005}}, #{{F006}}, #{{NF001}}

---

## 1. Logical Design View

### 1.1 Data Types (`PoKeysLibAsync.h`)

#### `task_priority_t` (enum)

```c
typedef enum {
    SCHED_PRIORITY_CRITICAL = 0,
    SCHED_PRIORITY_HIGH     = 1,
    SCHED_PRIORITY_NORMAL   = 2,
    SCHED_PRIORITY_LOW      = 3
} task_priority_t;
```

Implements: #{{ADR1}}

#### `periodic_async_task_t` (struct)

```c
typedef struct {
    async_func_t    func;           /* task function pointer */
    sPoKeysDevice  *dev;            /* device handle */
    int64_t         interval_ns;    /* firing period in nanoseconds */
    int64_t         next_call_time; /* absolute RTAPI time of next fire */
    const char     *name;           /* unique name for set_active/logging */
    int             active;         /* 1 = enabled, 0 = disabled */
    task_priority_t priority;       /* scheduling priority */
} periodic_async_task_t;
```

### 1.2 Static Storage (`PoKeysLibAsync.c`)

```c
static periodic_async_task_t async_tasks[MAX_ASYNC_TASKS]; /* MAX=16 */
static size_t  async_task_count_internal = 0;
static uint8_t scheduler_system_load     = 0;  /* 0–100, from cmd 0x05 */
static int     scheduler_machine_on      = 0;  /* 0=off, 1=on */
```

All storage is pre-allocated at compile time: satisfies #{{NF001}} (no `malloc` in RT path).

---

## 2. Algorithms

### 2.1 `register_async_task()` — Registration with Prime Stagger

**Precondition**: `async_task_count_internal < MAX_ASYNC_TASKS` and `freq_hz > 0`

```
PRIME_STAGGER_MS[] = {0, 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47}

register_async_task(func, dev, freq_hz, name, priority):
    i    ← async_task_count_internal
    slot ← async_tasks[i]
    slot.func           ← func
    slot.dev            ← dev
    slot.interval_ns    ← (1_000_000_000 / freq_hz)   [integer ns]
    slot.next_call_time ← rtapi_get_time()
                          + PRIME_STAGGER_MS[i] * 1_000_000
    slot.name           ← name
    slot.active         ← 1
    slot.priority       ← priority
    async_task_count_internal ← i + 1
    return 0
```

Implements: #{{F001}} (priority field), #{{F003}} (stagger), #{{ADR3}}

### 2.2 `async_dispatcher()` — Load-Aware Single-Task Dispatch

**Postcondition**: At most one task is fired per invocation.

```
async_dispatcher():
    now      ← rtapi_get_time()
    selected ← none
    earliest ← MAX_INT64

    for each task T in async_tasks[0 .. count-1]:
        if NOT T.active            : skip
        if T.next_call_time > now  : skip   // not yet due

        // Load-based throttle (implements #{{F002}}, #{{ADR2}})
        if T.priority == LOW    AND scheduler_system_load >  50: skip
        if T.priority == NORMAL AND scheduler_system_load >  80: skip
        if T.priority == HIGH   AND scheduler_system_load >  95: skip
        // CRITICAL: never skipped

        // Machine-on lock (implements #{{F005}}, #{{ADR5}})
        if T.priority == LOW AND scheduler_machine_on: skip

        if T.next_call_time < earliest:
            selected ← T
            earliest ← T.next_call_time

    if selected == none: return 0

    selected.func(selected.dev)
    selected.next_call_time ← now + selected.interval_ns
    return 1
```

**Complexity**: O(n), n = registered task count (≤ 16)

### 2.3 Dirty-Flag in `PK_DigitalIOSetAsync()` (`PoKeysLibIOAsync.c`)

```
static last_dio_out[7], last_mask_out[7], initialized = false

PK_DigitalIOSetAsync(device):
    build dio[7], mask[7] from device->Pins[]

    if initialized
       AND memcmp(dio,  last_dio_out,  7) == 0
       AND memcmp(mask, last_mask_out, 7) == 0:
        return PK_OK          // nothing changed (#{{F004}})

    copy dio  → last_dio_out
    copy mask → last_mask_out
    initialized ← true
    send 64-byte packet (cmd 0xCC, param1=1)
    return result
```

Implements: #{{F004}}, #{{ADR4}}

---

## 3. Interaction View — Sequence Diagram (textual)

```
EXTRA_SETUP()                   async_dispatcher()        pk_load_monitor_task()
     |                                 |                          |
     |-- register_async_task(pev2, CRITICAL) ──────────────────> |
     |-- register_async_task(digio, HIGH)    ──────────────────> |
     |-- register_async_task(load,  LOW)     ──────────────────> |
     |                                 |                          |
     |   [RT servo thread, 1 kHz]      |                          |
     |                                 |                          |
     |-- async_dispatcher() ---------> |                          |
     |       load=0, machine_on=0      |                          |
     |       pev2 overdue, CRITICAL    |                          |
     |       → fire pev2_status()      |                          |
     |<-- return 1                     |                          |
     |                                 |                          |
     |  [200 ms later, load=73, on=1]  |                          |
     |-- async_dispatcher() ---------> |                          |
     |       load_monitor overdue but  |                          |
     |       LOW + machine_on → skip   |                          |
     |       pev2_status overdue       |                          |
     |       → fire pev2_status()      |                          |
     |<-- return 1                     |                          |
     |                                 |                          |
     |  [machine_on=0, load drops 30]  |                          |
     |-- async_dispatcher() ---------> |                          |
     |       load_monitor overdue      |  pk_load_monitor_task() |
     |       LOW, load=30, machine=0   |------------------------->|
     |       → fire load_monitor()     |  read CPUload=30         |
     |                                 |  scheduler_update(30)    |
     |                                 |  PK_DeviceLoadStatusAsync|
     |<-- return 1                     |<-------------------------|
```

---

## 4. Implementation View — File Mapping

| File                              | Responsibility                                                                      |
|-----------------------------------|-------------------------------------------------------------------------------------|
| `PoKeysLibAsync.h`                | `task_priority_t`, `periodic_async_task_t`, all API declarations                   |
| `PoKeysLibAsync.c`                | `register_async_task()`, `async_dispatcher()`, `scheduler_set_machine_on()`, load APIs |
| `PoKeysLibIOAsync.c`              | `PK_DigitalIOSetAsync()` dirty-flag block                                           |
| `experimental/pokeys_async.c`     | `pk_load_monitor_task()`, `start_async_processing()`, `update_device_cache()`       |

---

## 5. Design Rationale

All design choices trace directly to the ADRs:

- Four priority levels → #{{ADR1}}
- Threshold values (50 / 80 / 95 %) → #{{ADR2}}
- Prime stagger table → #{{ADR3}}
- Dirty-flag `memcmp` → #{{ADR4}}
- Machine-on via PEv2 state → #{{ADR5}}

## Traceability

- **Architecture Component**: #{{ARC}}
- **Implements**: #{{F001}}, #{{F002}}, #{{F003}}, #{{F004}}, #{{F005}}, #{{F006}}, #{{NF001}}
- **Based on ADRs**: #{{ADR1}}, #{{ADR2}}, #{{ADR3}}, #{{ADR4}}, #{{ADR5}}
- **Implemented by**: PR on branch `copilot/optimize-async-scheduler`
- **Verified by**: (TEST issues — add when created)
