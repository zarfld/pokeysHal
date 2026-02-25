## Architecture Decision Record

**Status**: Accepted

## Requirements Addressed

- Traces to: #{{F005}} (REQ-F-SCHED-005: Machine-On Configuration Lock)
- Traces to: #{{F006}} (REQ-F-SCHED-006: Periodic System Load Monitoring)

## Context

When the CNC machine is actively moving (`PulseEngineState > 0`), configuration updates
(PoNET module config, RTC sync, LED matrix) are not required and compete with motion data
for interface bandwidth.  A mechanism is needed to automatically silence these tasks without
requiring explicit per-task logic.

The scheduler already has the `SCHED_PRIORITY_LOW` bucket which is targeted at these tasks.

## Decision

Extend the scheduler with a single `static int scheduler_machine_on` flag toggled by
`scheduler_set_machine_on(int)`.  Inside `async_dispatcher()`, LOW-priority tasks are
skipped when this flag is set:

```c
if (prio == SCHED_PRIORITY_LOW && scheduler_machine_on) continue;
```

The flag is driven from `update_device_cache()` in the HAL component:

```c
scheduler_set_machine_on(device_cache.pulse_engine_state > 0);
```

This keeps the lock in sync with actual device state without additional polling.

## Alternatives Considered

### Alternative — HAL pin `machine-on` read by scheduler

**Pros**: Operator-controllable from HAL.
**Cons**: Creates a dependency from the infrastructure library on HAL pin access;
complicates library reuse outside LinuxCNC.  Rejected.

### Alternative — Lock only during specific PEv2 commands

**Pros**: Finer-grained.
**Cons**: Requires per-command state awareness in the scheduler, violating single
responsibility.  Rejected.

## Consequences

**Positive**: Zero overhead when machine is off; seamless transition; no extra HAL pins.

**Negative**: Any task that MUST run during machine-on must use a priority higher than LOW —
callers must be aware of this constraint.

## Traceability

- **Traces to**: #{{F005}}, #{{F006}}
- **Implemented by**: `PoKeysLibAsync.c` (`scheduler_set_machine_on()`), `experimental/pokeys_async.c` (`update_device_cache()`)
- **Verified by**: (TEST issues — add when created)
