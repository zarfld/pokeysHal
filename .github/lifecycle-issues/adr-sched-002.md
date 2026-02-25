## Architecture Decision Record

**Status**: Accepted

## Requirements Addressed

- Traces to: #{{F002}} (REQ-F-SCHED-002: Load-Based Dynamic Throttling)
- Traces to: #{{NF001}} (REQ-NF-SCHED-001: Real-Time Timing Safety)
- Traces to: #{{NF002}} (REQ-NF-SCHED-002: Interface Load Distribution)

## Context

Given four priority levels (ADR-SCHED-001), specific numeric CPU-load thresholds must be
chosen.  The PoKeys device reports CPU load as a 0–100 integer via protocol command 0x05.
Thresholds must:

- Keep safety/position traffic alive at all load levels.
- Start shedding non-essential work before the interface saturates.
- Be deterministic and RT-safe (pure integer comparisons, no floating point).

## Decision

The following static threshold table is used inside `async_dispatcher()`:

```c
if (prio == SCHED_PRIORITY_LOW    && scheduler_system_load >  50) continue;
if (prio == SCHED_PRIORITY_NORMAL && scheduler_system_load >  80) continue;
if (prio == SCHED_PRIORITY_HIGH   && scheduler_system_load >  95) continue;
/* CRITICAL: no check — always fires */
```

Thresholds are intentionally asymmetric: LOW tasks shed aggressively (> 50 %) to create
headroom for motion; CRITICAL tasks are unconditional.

## Alternatives Considered

### Alternative — Thresholds 70 / 85 / 99

**Pros**: More conservative — keeps more tasks running longer.
**Cons**: At 80 % load, LOW tasks (PoNET, RTC) still consume bandwidth competing with
motion tasks.  Rejected — 50 % provides sufficient margin.

### Alternative — Configurable thresholds via HAL parameters

**Pros**: Tunable at runtime.
**Cons**: Requires extra HAL pins; thresholds must be validated to prevent unsafe
configurations.  Deferred — can be added later without breaking this decision.

## Consequences

**Positive**: Pure integer comparisons; zero floating-point in RT path; satisfies #{{NF001}}.

**Negative**: Hard-coded thresholds may need tuning for different hardware generations.

## Traceability

- **Traces to**: #{{F002}}, #{{NF001}}, #{{NF002}}
- **Implemented by**: `PoKeysLibAsync.c` (`async_dispatcher()` threshold block)
- **Verified by**: (TEST issues — add when created)
