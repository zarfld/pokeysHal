## Architecture Decision Record

**Status**: Accepted

## Requirements Addressed

- Traces to: #{{F003}} (REQ-F-SCHED-003: Prime-Number Stagger Offsets)
- Traces to: #{{NF002}} (REQ-NF-SCHED-002: Interface Load Distribution)

## Context

Without staggering, all 14 tasks have `next_call_time = now` at startup.  On the first
scheduler cycle every task is simultaneously overdue.  `async_dispatcher()` fires only the
earliest-due task per call, so one packet is sent per call — but all 14 are queued
immediately, creating a startup burst.  Worse, tasks with the same period will continue to
coincide on every subsequent cycle.

A simple uniform offset (e.g. 1 ms per task) resolves the startup burst but does not
prevent periodic coincidence for tasks whose periods are multiples of each other.

## Decision

Use the first 16 prime numbers, converted to milliseconds, as initial offset values:

```c
static const int64_t PRIME_STAGGER_MS[MAX_ASYNC_TASKS] = {
     0,  2,  3,  5,  7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47
};
```

Task `i` (0-indexed registration order) receives
`next_call_time = now + PRIME_STAGGER_MS[i] * 1_000_000` nanoseconds.

Prime offsets are chosen because tasks with prime periods are coprime, minimising future
coincidence.  The sequence spans 0–47 ms, well within a single 1-Hz task period (1000 ms),
so startup latency remains negligible.

## Alternatives Considered

### Alternative — Uniform 1 ms / 2 ms steps

**Pros**: Simple.
**Cons**: Tasks with equal or harmonic periods re-converge rapidly.  Rejected.

### Alternative — Random jitter (seeded `rand()`)

**Pros**: Statistically unlikely to converge.
**Cons**: Non-deterministic; different between runs; harder to reason about in RT context.
Rejected.

## Consequences

**Positive**: Tasks with prime-number periods never coincide; startup burst eliminated;
deterministic and RT-safe.

**Negative**: The stagger table has 16 entries — cannot register more than 16 tasks without
extending the table.

## Traceability

- **Traces to**: #{{F003}}, #{{NF002}}
- **Implemented by**: `PoKeysLibAsync.c` (`register_async_task()` stagger table)
- **Verified by**: (TEST issues — add when created)
