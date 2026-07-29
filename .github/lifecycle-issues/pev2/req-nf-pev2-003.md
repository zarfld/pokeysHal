## Non-Functional Requirement

**Parent Stakeholder Requirement**: Traces to: #{{STR_NUM}} (HAL Interface: PEv2)

**Quality Attribute Category**: Correctness (enumeration and API compliance)

## Requirement Description

The PEv2 HAL implementation **shall** use the correct PoKeysLib enumerations and constants
when interpreting bitmapped fields.  No magic numbers **shall** be used where a named
enumeration constant exists in `PoKeysLib.h`.

Required enumeration usage:
- `ePK_PEv2_AxisConfig` — for `AxesConfig[]` bit interpretation
- `ePK_PEv2_AxisSwitchOptions` — for `AxesSwitchConfig[]` bit interpretation
- `ePK_PEv2_AxisSignalOptions` — for `SignalConfig[]` bit interpretation
- `ePoKeysPEState` — for `PulseEngineState` value interpretation
- `ePK_PEAxisState` — for `AxesState[]` value interpretation

## Measurable Criteria

| Metric | Target | Measurement Method |
|--------|--------|--------------------|
| Magic numeric literals in bitmask operations | 0 | Code inspection (`grep` for raw hex/integer masks) |
| PoKeysLib enumeration constants used | All bitmask ops use named constants | Code review |
| Compile warnings about implicit enum conversions | 0 | Build with `-Wall -Wextra` |

## Verification Method

Code inspection + Build verification

## Priority

P1 – High

## Traceability

- **Traces to**: #{{STR_NUM}}
- **Refined by**: (ADR issues — see comment)
- **Implemented in**: `PoKeysLibPulseEngine_v2Async.c`
- **Verified by**: (TEST issues — add when created)
