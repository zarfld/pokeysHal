## Functional Requirement

**Parent Stakeholder Requirement**: Traces to: #{{STR_NUM}} (Async Scheduler optimizations)

## Requirement Description

The scheduler **shall** skip lower-priority tasks when the PoKeys device reports elevated CPU
load, according to the following threshold table:

| Priority | Skip condition             |
|----------|----------------------------|
| CRITICAL | Never skipped              |
| HIGH     | Device CPU load > 95 %     |
| NORMAL   | Device CPU load > 80 %     |
| LOW      | Device CPU load > 50 %     |

The check **shall** occur inside `async_dispatcher()` for every candidate task immediately
before it is fired.  The CPU load value is the one most recently stored by
`scheduler_update_system_load()`.

## Acceptance Criteria

### Scenario 1 — NORMAL task skipped at 85 % load

**Given** `scheduler_update_system_load(85)` has been called
**And** a NORMAL-priority task is overdue
**When** `async_dispatcher()` is called
**Then** the NORMAL task is **not** fired

### Scenario 2 — CRITICAL task fires at 100 % load

**Given** `scheduler_update_system_load(100)` has been called
**And** a CRITICAL-priority task is overdue
**When** `async_dispatcher()` is called
**Then** the CRITICAL task **is** fired

### Scenario 3 — HIGH task fires at 94 % load

**Given** `scheduler_update_system_load(94)` has been called
**And** a HIGH-priority task is overdue
**When** `async_dispatcher()` is called
**Then** the HIGH task **is** fired

### Scenario 4 — LOW task skipped at 51 % load

**Given** `scheduler_update_system_load(51)` has been called
**And** a LOW-priority task is overdue
**When** `async_dispatcher()` is called
**Then** the LOW task is **not** fired

## Verification Method

Test (automated)

## Priority

P0 – Critical

## Traceability

- **Traces to**: #{{STR_NUM}}
- **Refined by**: (ADR issues — see comment)
- **Implemented in**: `PoKeysLibAsync.c` (`async_dispatcher()` threshold block)
- **Verified by**: (TEST issues — add when created)
