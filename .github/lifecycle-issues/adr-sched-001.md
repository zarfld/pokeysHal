## Architecture Decision Record

**Status**: Accepted

## Requirements Addressed

- Traces to: #{{F001}} (REQ-F-SCHED-001: Task Priority Classification)
- Traces to: #{{F002}} (REQ-F-SCHED-002: Load-Based Dynamic Throttling)
- Traces to: #{{F005}} (REQ-F-SCHED-005: Machine-On Configuration Lock)
- Traces to: #{{NF001}} (REQ-NF-SCHED-001: Real-Time Timing Safety)
- Traces to: #{{NF003}} (REQ-NF-SCHED-003: Safety Signal Latency < 10 ms)

## Context

The async scheduler fires 14 tasks at different rates over a shared 64-byte-packet
USB/Ethernet interface.  Without differentiation, all tasks compete equally for the
interface, and a load spike can delay position feedback just as much as a non-critical LED
update — causing LinuxCNC following errors.

A mechanism is needed to guarantee that under load, the most safety-relevant tasks always
execute while less critical work is deferred.

## Decision

Classify every async task into one of four priority levels encoded as a `task_priority_t`
enum in `PoKeysLibAsync.h`:

```c
typedef enum {
    SCHED_PRIORITY_CRITICAL = 0,  /* never skipped */
    SCHED_PRIORITY_HIGH     = 1,  /* skipped only at load > 95 % */
    SCHED_PRIORITY_NORMAL   = 2,  /* skipped at load > 80 % */
    SCHED_PRIORITY_LOW      = 3   /* skipped at load > 50 % + machine-on */
} task_priority_t;
```

The level is assigned at registration time and checked in `async_dispatcher()`.

## Alternatives Considered

### Alternative 1 — Binary critical/non-critical flag

**Pros**: Simpler to implement.
**Cons**: Cannot distinguish secondary sensors (should still run at moderate load) from
non-critical config tasks.  Rejected — too coarse.

### Alternative 2 — Dynamic priority computed from task age

**Pros**: Self-adjusting.
**Cons**: Non-trivial computation in RT path; heap-allocated state violates safety
requirements.  Rejected.

### Alternative 3 — Single global on/off throttle at 80 %

**Pros**: Trivial.
**Cons**: Drops encoder and digital-I/O tasks which carry safety signals.  Rejected —
violates #{{NF003}}.

## Consequences

**Positive**

- Safety signals (CRITICAL/HIGH) are never dropped at any load level.
- Config and PoNET tasks (LOW) are automatically silenced when bandwidth is scarce.
- The enum can be extended without changing the dispatcher algorithm.

**Negative**

- Callers must assign a meaningful priority; incorrect assignment silently degrades
  behaviour.

## Traceability

- **Traces to**: #{{F001}}, #{{F002}}, #{{F005}}, #{{NF001}}, #{{NF003}}
- **Implemented by**: `PoKeysLibAsync.h` (`task_priority_t`), `PoKeysLibAsync.c` (`async_dispatcher()`)
- **Verified by**: (TEST issues — add when created)
