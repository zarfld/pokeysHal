## Functional Requirement

**Parent Stakeholder Requirement**: Traces to: #{{STR_NUM}} (HAL Interface: PEv2)

## Requirement Description

Every HAL **output** pin (direction HAL_OUT) for PEv2 **shall** be written from the
corresponding `device->PEv2.*` pointer each RT servo cycle, after the relevant PoKeysLib
receive/parse function has completed.

**Mapping rules** (PoKeysLib pointer → HAL pin):

| PoKeysLib Pointer | HAL Pin | Conversion |
|-------------------|---------|------------|
| `device->PEv2.info.nrOfAxes` | `PEv2.nrOfAxes` | direct u32 |
| `device->PEv2.info.maxPulseFrequency` | `PEv2.maxPulseFrequency` | direct u32 |
| `device->PEv2.info.bufferDepth` | `PEv2.bufferDepth` | direct u32 |
| `device->PEv2.info.slotTiming` | `PEv2.slotTiming` | direct u32 |
| `device->PEv2.PulseEngineEnabled` | `PEv2.PulseEngineEnabled` | direct u32 |
| `device->PEv2.PulseGeneratorType` | `PEv2.PulseGeneratorType` | direct u32 |
| `device->PEv2.ChargePumpEnabled` | `PEv2.ChargePumpEnabled` | bit cast |
| `device->PEv2.PulseEngineActivated` | `PEv2.PulseEngineActivated` | direct u32 |
| `device->PEv2.PulseEngineState` | `PEv2.PulseEngineState` | direct u32 |
| `device->PEv2.MiscInputStatus` (bit N) | `PEv2.digin.Misc-N.in` | bitmask extract |
| `device->PEv2.MiscInputStatus` (bit N) | `PEv2.digin.Misc-N.in-not` | inverted bit |
| `device->PEv2.LimitOverride` | `PEv2.LimitOverride` | direct u32 |
| `device->PEv2.EmergencyInputStatus` | `PEv2.digin.Emergency.in` | direct bit |
| `device->PEv2.EmergencyInputStatus` | `PEv2.digin.Emergency.in-not` | inverted bit |
| `device->PEv2.ProbeStatus` | `PEv2.digin.Probed.in` | any-bit-set |
| `device->PEv2.ExternalRelayOutputs` | `PEv2.ExternalRelayOutputs` | direct u32 |
| `device->PEv2.ExternalOCOutputs` | `PEv2.ExternalOCOutputs` | direct u32 |
| `device->PEv2.ProbeStatus` | `PEv2.ProbeStatus` | direct u32 |
| `device->PEv2.BacklashCompensationEnabled` | `PEv2.BacklashCompensationEnabled` | direct u32 |
| `device->PEv2.AxisEnabledMask` | `PEv2.AxisEnabledMask` | direct u32 |
| `device->PEv2.AxisEnabledStatesMask` | `PEv2.AxisEnabledStatesMask` | direct u32 |

**Per-axis output updates** (`device->PEv2.*[N]` → `PEv2.[N].*`):

| PoKeysLib Pointer | HAL Pin | Conversion |
|-------------------|---------|------------|
| `device->PEv2.AxesState[N]` | `PEv2.[N].AxesState` | direct u32 |
| `device->PEv2.AxesConfig[N]` | `PEv2.[N].AxesConfig` | direct u32 |
| `device->PEv2.AxesSwitchConfig[N]` | `PEv2.[N].AxesSwitchConfig` | direct u32 |
| `device->PEv2.CurrentPosition[N]` | `PEv2.[N].CurrentPosition` | direct s32 |
| `device->PEv2.SoftLimitMinimum[N]` | `PEv2.[N].SoftLimitMinimum` | direct s32 |
| `device->PEv2.SoftLimitMaximum[N]` | `PEv2.[N].SoftLimitMaximum` | direct s32 |
| `device->PEv2.HomingSpeed[N]` | `PEv2.[N].HomingSpeed` | direct u32 |
| `device->PEv2.HomingReturnSpeed[N]` | `PEv2.[N].HomingReturnSpeed` | direct u32 |
| `device->PEv2.MaxSpeed[N]` | `PEv2.[N].MaxSpeed` | direct float |
| `device->PEv2.MaxAcceleration[N]` | `PEv2.[N].MaxAcceleration` | direct float |
| `device->PEv2.BacklashWidth[N]` | `PEv2.[N].BacklashWidth` | direct u32 |
| `device->PEv2.BacklashRegister[N]` | `PEv2.[N].BacklashRegister` | direct s32 |
| `device->PEv2.BacklashAcceleration[N]` | `PEv2.[N].BacklashAcceleration` | direct float |
| `device->PEv2.LimitStatusN` (bit N) | `PEv2.[N].digin.LimitN.in` | bitmask extract |
| `device->PEv2.LimitStatusN` (bit N) | `PEv2.[N].digin.LimitN.in-not` | inverted bit |
| `device->PEv2.LimitStatusP` (bit N) | `PEv2.[N].digin.LimitP.in` | bitmask extract |
| `device->PEv2.LimitStatusP` (bit N) | `PEv2.[N].digin.LimitP.in-not` | inverted bit |
| `device->PEv2.HomeStatus` (bit N) | `PEv2.[N].digin.Home.in` | bitmask extract |
| `device->PEv2.HomeStatus` (bit N) | `PEv2.[N].digin.Home.in-not` | inverted bit |
| `device->PEv2.ErrorInputStatus` (bit N) | `PEv2.[N].digin.Error.in` | bitmask extract |
| `device->PEv2.DedicatedLimitNInputs` (bit N) | `PEv2.[N].digin.LimitN.DedicatedInput` | bitmask extract |
| `device->PEv2.DedicatedLimitPInputs` (bit N) | `PEv2.[N].digin.LimitP.DedicatedInput` | bitmask extract |
| `device->PEv2.DedicatedHomeInputs` (bit N) | `PEv2.[N].digin.Home.DedicatedInput` | bitmask extract |
| `device->PEv2.SoftLimitStatus` (bit N) | `PEv2.[N].digin.SoftLimit.in` | bitmask extract |

The update **shall** occur in a dedicated function `pev2_read_device_to_hal()` called after
each `PK_PEv2_StatusGetAsync()` parse cycle completes.

## Acceptance Criteria

### Scenario 1 — CurrentPosition propagation

**Given** `device->PEv2.CurrentPosition[0]` is set to `5000`
**When** `pev2_read_device_to_hal()` is called
**Then** `pokeys.0.PEv2.0.CurrentPosition` reads `5000`

### Scenario 2 — LimitN bitmask extraction

**Given** `device->PEv2.LimitStatusN` is `0x05` (bits 0 and 2 set)
**When** `pev2_read_device_to_hal()` is called
**Then** `PEv2.0.digin.LimitN.in` = TRUE, `PEv2.1.digin.LimitN.in` = FALSE,
`PEv2.2.digin.LimitN.in` = TRUE

### Scenario 3 — Misc input individual pins

**Given** `device->PEv2.MiscInputStatus` is `0x03` (bits 0 and 1 set)
**When** `pev2_read_device_to_hal()` is called
**Then** `PEv2.digin.Misc-0.in` = TRUE, `PEv2.digin.Misc-1.in` = TRUE,
`PEv2.digin.Misc-0.in-not` = FALSE, `PEv2.digin.Misc-2.in` = FALSE

## Verification Method

Test (automated)

## Priority

P0 – Critical

## Traceability

- **Traces to**: #{{STR_NUM}}
- **Refined by**: (ADR issues — see comment)
- **Implemented in**: `PoKeysLibPulseEngine_v2Async.c`
- **Verified by**: (TEST issues — add when created)
