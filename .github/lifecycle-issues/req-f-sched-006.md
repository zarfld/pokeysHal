## Functional Requirement

**Parent Stakeholder Requirement**: Traces to: #{{STR_NUM}} (Async Scheduler optimizations)

## Requirement Description

The scheduler **shall** include a dedicated `pk_load_monitor_task` registered at 5 Hz with
priority `SCHED_PRIORITY_LOW`.  On each invocation the task **shall**:

1. Read `dev->deviceLoadStatus.CPUload` (result of the last completed protocol command 0x05
   response) and forward it via `scheduler_update_system_load()`.
2. Issue a new async request for command 0x05 (`PK_DeviceLoadStatusAsync()`) so a fresh
   value is available on the next cycle.

The scheduler **shall** expose `scheduler_get_system_load()` returning the last cached CPU
load as `uint8_t` (range 0–100).

## Acceptance Criteria

### Scenario 1 — Load value propagated to scheduler

**Given** `dev->deviceLoadStatus.CPUload` is 73
**When** `pk_load_monitor_task(dev)` is called
**Then** `scheduler_get_system_load()` returns 73

### Scenario 2 — New device request issued each call

**When** `pk_load_monitor_task(dev)` executes
**Then** a `PK_DeviceLoadStatusAsync` request is enqueued to the device

### Scenario 3 — Task registered at correct frequency

**Given** `start_async_processing()` has completed
**When** the task table is inspected
**Then** an entry named `"load_monitor"` exists with `interval_ns = 200 000 000` (5 Hz)

## Verification Method

Inspection + Test (manual)

## Priority

P1 – High

## Traceability

- **Traces to**: #{{STR_NUM}}
- **Refined by**: (ADR issues — see comment)
- **Implemented in**: `experimental/pokeys_async.c` (`pk_load_monitor_task`), `PoKeysLibAsync.c` (`scheduler_update_system_load`, `scheduler_get_system_load`)
- **Verified by**: (TEST issues — add when created)
