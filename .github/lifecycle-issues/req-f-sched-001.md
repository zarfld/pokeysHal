## Functional Requirement

**Parent Stakeholder Requirement**: Traces to: #{{STR_NUM}} (Async Scheduler optimizations)

## Requirement Description

The scheduler **shall** classify every registered async task into exactly one of four priority
levels:

| Level    | Constant                  | Value | Typical tasks                              | Skip rule                               |
|----------|---------------------------|-------|--------------------------------------------|------------------------------------------|
| Critical | `SCHED_PRIORITY_CRITICAL` | 0     | Position feedback, motion commands, e-stop | Never skipped                            |
| High     | `SCHED_PRIORITY_HIGH`     | 1     | Digital I/O, limit/home switches, encoders | Skipped only when device load > 95 %    |
| Normal   | `SCHED_PRIORITY_NORMAL`   | 2     | Secondary sensors (analog, PWM)            | Skipped when device load > 80 %         |
| Low      | `SCHED_PRIORITY_LOW`      | 3     | Config, PoNET, RTC                         | Skipped at load > 50 % AND machine-on   |

The priority level **shall** be assigned at task registration time via `register_async_task()`
and stored as a `task_priority_t` field in `periodic_async_task_t` (`PoKeysLibAsync.h`).

## Acceptance Criteria

### Scenario 1 — Priority stored at registration

**Given** a call to `register_async_task(func, dev, freq_hz, name, SCHED_PRIORITY_HIGH)`
**When** the internal task table is inspected
**Then** the stored `priority` field equals `SCHED_PRIORITY_HIGH`

### Scenario 2 — All four levels are distinct and compile

**Given** the `task_priority_t` enum in `PoKeysLibAsync.h`
**When** the code is compiled
**Then** constants 0–3 are present, unique, and no implicit-conversion warnings occur

## Verification Method

Test (automated)

## Priority

P0 – Critical

## Traceability

- **Traces to**: #{{STR_NUM}}
- **Refined by**: (ADR issues — see comment)
- **Implemented in**: `PoKeysLibAsync.h` (`task_priority_t`), `PoKeysLibAsync.c` (`register_async_task`)
- **Verified by**: (TEST issues — add when created)
