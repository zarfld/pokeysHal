## Functional Requirement

**Parent Stakeholder Requirement**: Traces to: #{{STR_NUM}} (Async Scheduler optimizations)

## Requirement Description

The scheduler **shall** assign each newly registered task an initial `next_call_time` that is
offset from the current time by a distinct prime-number millisecond value drawn from the
pre-defined sequence:

```
0, 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47  (ms)
```

Offsets **shall** be applied in registration order (task index 0 → 0 ms, index 1 → 2 ms, …).
This prevents multiple tasks from becoming simultaneously overdue on the first scheduler
cycle, eliminating startup bursts on the USB/network interface.

## Acceptance Criteria

### Scenario 1 — First task: zero offset

**Given** no tasks have been registered yet
**When** the first task is registered
**Then** `next_call_time == now + 0 ms`

### Scenario 2 — Second task: 2 ms offset

**Given** exactly one task is already registered
**When** a second task is registered
**Then** `next_call_time == now + 2 ms`

### Scenario 3 — 14 tasks produce no simultaneous firing

**Given** 14 tasks are registered in sequence
**When** `async_dispatcher()` is called continuously from t=0 to t=50 ms
**Then** at most one task fires per dispatcher invocation during the 50 ms window

## Verification Method

Test (automated)

## Priority

P1 – High

## Traceability

- **Traces to**: #{{STR_NUM}}
- **Refined by**: (ADR issues — see comment)
- **Implemented in**: `PoKeysLibAsync.c` (`register_async_task()` stagger table)
- **Verified by**: (TEST issues — add when created)
