## Non-Functional Requirement

**Parent Stakeholder Requirement**: Traces to: #{{STR_NUM}} (Async Scheduler optimizations)

**Quality Attribute Category**: Real-Time (timing constraints, determinism)

## Requirement Description

The async scheduler **shall** not introduce any blocking operation in the LinuxCNC real-time
servo thread.  All mailbox slots, task table entries, and dirty-flag state **shall** be
pre-allocated as static compile-time arrays.  No `malloc()`, `free()`, mutex lock, condition
variable, or blocking socket call **shall** appear in any code path reachable from
`async_dispatcher()` or `register_async_task()` during steady-state operation.

## Measurable Criteria

| Metric                                          | Target                          | Measurement Method              |
|-------------------------------------------------|---------------------------------|---------------------------------|
| `async_dispatcher()` worst-case execution time  | < 5 µs                         | RT trace / `cyclictest`         |
| Dynamic memory allocation in RT path            | 0 calls                         | Static code inspection          |
| Blocking socket calls in RT path                | 0 calls                         | Static code inspection          |
| Max registered tasks (compile-time bound)       | ≤ `MAX_ASYNC_TASKS` (16)        | Code inspection                 |
| RT thread jitter introduced by scheduler        | < 50 µs additional             | `cyclictest -l 10000`           |

## Verification Method

Inspection + Analysis

## Priority

P0 – Critical

## Traceability

- **Traces to**: #{{STR_NUM}}
- **Refined by**: (ADR issues — see comment)
- **Implemented in**: `PoKeysLibAsync.c` (static `async_tasks[]` array, no heap allocation)
- **Verified by**: (TEST issues — add when created)
