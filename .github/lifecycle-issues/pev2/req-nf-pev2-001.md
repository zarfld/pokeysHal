## Non-Functional Requirement

**Parent Stakeholder Requirement**: Traces to: #{{STR_NUM}} (HAL Interface: PEv2)

**Quality Attribute Category**: Real-Time (timing constraints, determinism)

## Requirement Description

All PEv2 HAL update functions (`pev2_read_device_to_hal()`, `pev2_write_hal_to_device()`)
**shall** complete within the LinuxCNC servo thread deadline.  No blocking socket calls,
mutex locks, `malloc()`, or `free()` **shall** appear in any code path executed from the
RT servo thread for PEv2 operations.

All PoKeysLib function calls in the RT path **shall** use the `*Async` variants exclusively.

## Measurable Criteria

| Metric | Target | Measurement Method |
|--------|--------|--------------------|
| `pev2_read_device_to_hal()` worst-case time | < 2 µs | RT trace |
| `pev2_write_hal_to_device()` worst-case time | < 2 µs | RT trace |
| Blocking socket calls in PEv2 RT path | 0 | Static code inspection |
| Dynamic memory allocation in PEv2 RT path | 0 | Static code inspection |
| Mutex/condition variable usage in PEv2 RT path | 0 | Static code inspection |

## Verification Method

Inspection + Analysis

## Priority

P0 – Critical

## Traceability

- **Traces to**: #{{STR_NUM}}
- **Refined by**: (ADR issues — see comment)
- **Implemented in**: `PoKeysLibPulseEngine_v2Async.c` (async variants only)
- **Verified by**: (TEST issues — add when created)
