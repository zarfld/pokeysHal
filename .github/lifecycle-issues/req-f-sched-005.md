## Functional Requirement

**Parent Stakeholder Requirement**: Traces to: #{{STR_NUM}} (Async Scheduler optimizations)

## Requirement Description

The scheduler **shall** provide a `scheduler_set_machine_on(int machine_on)` API.  While
`machine_on != 0`, all tasks with priority `SCHED_PRIORITY_LOW` **shall** be suppressed by
`async_dispatcher()` regardless of their due time, reserving full interface bandwidth for
real-time motion data.

The machine-on state **shall** be driven by the PulseEngine v2 state: the HAL component
**shall** call `scheduler_set_machine_on(pulse_engine_state > 0)` from
`update_device_cache()` on every RT cycle.

Tasks with priority CRITICAL, HIGH, or NORMAL **shall** be unaffected by the machine-on
flag.

## Acceptance Criteria

### Scenario 1 — LOW task suppressed when machine is on

**Given** `scheduler_set_machine_on(1)` has been called
**And** a LOW-priority task is overdue and device load < 50 %
**When** `async_dispatcher()` is called
**Then** the LOW task is **not** fired

### Scenario 2 — LOW task fires when machine is off

**Given** `scheduler_set_machine_on(0)` has been called
**And** a LOW-priority task is overdue and device load < 50 %
**When** `async_dispatcher()` is called
**Then** the LOW task **is** fired

### Scenario 3 — CRITICAL task unaffected by machine-on

**Given** `scheduler_set_machine_on(1)` has been called
**And** a CRITICAL-priority task is overdue
**When** `async_dispatcher()` is called
**Then** the CRITICAL task **is** fired

## Verification Method

Test (automated)

## Priority

P0 – Critical

## Traceability

- **Traces to**: #{{STR_NUM}}
- **Refined by**: (ADR issues — see comment)
- **Implemented in**: `PoKeysLibAsync.c` (`scheduler_set_machine_on()`), `experimental/pokeys_async.c` (`update_device_cache()`)
- **Verified by**: (TEST issues — add when created)
