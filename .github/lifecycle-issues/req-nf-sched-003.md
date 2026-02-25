## Non-Functional Requirement

**Parent Stakeholder Requirement**: Traces to: #{{STR_NUM}} (Async Scheduler optimizations)

**Quality Attribute Category**: Safety (fail-safe, IEC 61508)

## Requirement Description

Critical safety signals — E-Stop, limit switch states, and PulseEngine v2 position feedback
— **shall** be polled or transmitted within 10 ms of a state change.  Tasks carrying these
signals **shall** be assigned `SCHED_PRIORITY_CRITICAL` or `SCHED_PRIORITY_HIGH` and
**shall** never be blocked by load-based throttling or machine-on lock.

## Measurable Criteria

| Signal               | Task name       | Priority | Max polling period | Max latency |
|----------------------|-----------------|----------|--------------------|-------------|
| PEv2 position/status | `pev2_status`   | CRITICAL | 10 ms (100 Hz)     | < 10 ms     |
| PEv2 motion command  | `pev2_movepv`   | CRITICAL | 2 ms (500 Hz)      | < 10 ms     |
| E-Stop / limit switch| `digio_get`     | HIGH     | 5 ms (200 Hz)      | < 10 ms     |
| Encoder values       | `encoders`      | HIGH     | 2 ms (500 Hz)      | < 10 ms     |

## Verification Method

Demonstration + Test (manual)

## Priority

P0 – Critical

## Traceability

- **Traces to**: #{{STR_NUM}}
- **Refined by**: (ADR issues — see comment)
- **Implemented in**: `experimental/pokeys_async.c` (task priority assignments in `start_async_processing()`)
- **Verified by**: (TEST issues — add when created)
