## Functional Requirement

**Parent Stakeholder Requirement**: Traces to: #{{STR_NUM}} (HAL Interface: PEv2)

## Requirement Description

Every HAL **input** pin (direction HAL_IN) for PEv2 **shall** be read and the corresponding
`device->PEv2.*` pointer updated each RT servo cycle, before the relevant PoKeysLib send
function is called.

**Mapping rules** (HAL pin → PoKeysLib pointer):

| HAL Pin | PoKeysLib Pointer | Conversion |
|---------|-------------------|------------|
| `PEv2.digout.Emergency.out` | `device->PEv2.EmergencyOutput` | direct bit |
| `PEv2.LimitOverrideSetup` | `device->PEv2.LimitOverride` | direct u32 |
| `PEv2.HomingStartMaskSetup` | `device->PEv2.HomingStartMaskSetup` | direct u32 |
| `PEv2.ProbeStartMaskSetup` | `device->PEv2.ProbeStartMaskSetup` | direct u32 |
| `PEv2.ProbeSpeed` | `device->PEv2.ProbeSpeed` | float ratio 0.0–1.0 |
| `PEv2.digout.ExternalRelay-#.out` | `device->PEv2.ExternalRelayOutputs` (bit N) | bitmask |
| `PEv2.digout.ExternalOC-#.out` | `device->PEv2.ExternalOCOutputs` (bit N) | bitmask |
| `PEv2.[N].PositionSetup` | `device->PEv2.PositionSetup[N]` | direct s32 |
| `PEv2.[N].ReferencePositionSpeed` | `device->PEv2.ReferencePositionSpeed[N]` | direct s32 |
| `PEv2.[N].digout.AxisEnable.out` | `device->PEv2.AxisEnableOutputs[N]` | direct bit |
| `PEv2.[N].digout.AxisEnabled.out` | `device->PEv2.AxisEnabledOutputs[N]` | direct bit |
| `PEv2.[N].digout.LimitOverride.out` | `device->PEv2.LimitOverridePerAxis[N]` | direct bit |

The update **shall** occur in a dedicated function `pev2_write_hal_to_device()` called at
the start of each servo cycle before any PEv2 send commands.

## Acceptance Criteria

### Scenario 1 — Emergency output propagation

**Given** `pokeys.0.PEv2.digout.Emergency.out` is set to `TRUE`
**When** `pev2_write_hal_to_device()` is called
**Then** `device->PEv2.EmergencyOutput` is non-zero

### Scenario 2 — ExternalRelay bitmask assembly

**Given** pins `PEv2.digout.ExternalRelay-0.out` = TRUE, `PEv2.digout.ExternalRelay-1.out` = FALSE
**When** `pev2_write_hal_to_device()` is called
**Then** `device->PEv2.ExternalRelayOutputs` has bit 0 set and bit 1 clear

### Scenario 3 — Per-axis ReferencePositionSpeed update

**Given** `pokeys.0.PEv2.3.ReferencePositionSpeed` is set to `1000`
**When** `pev2_write_hal_to_device()` is called
**Then** `device->PEv2.ReferencePositionSpeed[3]` equals `1000`

## Verification Method

Test (automated)

## Priority

P0 – Critical

## Traceability

- **Traces to**: #{{STR_NUM}}
- **Refined by**: (ADR issues — see comment)
- **Implemented in**: `PoKeysLibPulseEngine_v2Async.c`
- **Verified by**: (TEST issues — add when created)
