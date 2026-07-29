## Functional Requirement

**Parent Stakeholder Requirement**: Traces to: #{{STR_NUM}} (HAL Interface: PEv2)

## Requirement Description

The HAL component **shall** export all global PEv2 status and configuration pins as defined
in the PoKeys HAL interface specification:

**Output pins** (updated from `device->PEv2.*`):
- `pokeys.[DevID].PEv2.nrOfAxes` — number of axes (u32, HAL_OUT)
- `pokeys.[DevID].PEv2.maxPulseFrequency` — max pulse frequency (u32, HAL_OUT)
- `pokeys.[DevID].PEv2.bufferDepth` — motion buffer depth (u32, HAL_OUT)
- `pokeys.[DevID].PEv2.slotTiming` — slot timing (u32, HAL_OUT)
- `pokeys.[DevID].PEv2.PulseEngineEnabled` — pulse engine enabled status (u32, HAL_OUT)
- `pokeys.[DevID].PEv2.PulseGeneratorType` — generator type (u32, HAL_OUT)
- `pokeys.[DevID].PEv2.PG_swap_stepdir` — swap step/dir (bit, HAL_OUT)
- `pokeys.[DevID].PEv2.PG_extended_io` — extended IO (bit, HAL_OUT)
- `pokeys.[DevID].PEv2.ChargePumpEnabled` — charge pump enabled (bit, HAL_OUT)
- `pokeys.[DevID].PEv2.PulseEngineActivated` — activation status (u32, HAL_OUT)
- `pokeys.[DevID].PEv2.PulseEngineState` — state (u32, HAL_OUT)
- `pokeys.[DevID].PEv2.MiscInputStatus` — misc digital inputs bitmask (u32, HAL_OUT)
- `pokeys.[DevID].PEv2.digin.Misc-#.in` (8 pins) — individual misc inputs (bit, HAL_OUT)
- `pokeys.[DevID].PEv2.digin.Misc-#.in-not` (8 pins) — inverted misc inputs (bit, HAL_OUT)
- `pokeys.[DevID].PEv2.LimitOverride` — limit override status (u32, HAL_OUT)
- `pokeys.[DevID].PEv2.digin.Probed.in` — probe detected (bit, HAL_OUT)
- `pokeys.[DevID].PEv2.AxisEnabledMask` — enabled axes bitmask (u32, HAL_OUT)
- `pokeys.[DevID].PEv2.AxisEnabledStatesMask` — enabled states bitmask (u32, HAL_OUT)
- `pokeys.[DevID].PEv2.ExternalRelayOutputs` — relay outputs bitmask (u32, HAL_OUT)
- `pokeys.[DevID].PEv2.ExternalOCOutputs` — OC outputs bitmask (u32, HAL_OUT)
- `pokeys.[DevID].PEv2.ProbeStatus` — probe completion status (u32, HAL_OUT)
- `pokeys.[DevID].PEv2.BacklashCompensationEnabled` — backlash enabled bitmask (u32, HAL_OUT)
- `pokeys.[DevID].PEv2.digin.Emergency.in` — emergency input (bit, HAL_OUT)
- `pokeys.[DevID].PEv2.digin.Emergency.in-not` — emergency input inverted (bit, HAL_OUT)

**Input pins** (written to `device->PEv2.*`):
- `pokeys.[DevID].PEv2.digout.Emergency.out` — emergency output (bit, HAL_IN)
- `pokeys.[DevID].PEv2.LimitOverrideSetup` — limit override config (u32, HAL_IN)
- `pokeys.[DevID].PEv2.HomingStartMaskSetup` — homing start bitmask (u32, HAL_IN)
- `pokeys.[DevID].PEv2.ProbeStartMaskSetup` — probe start bitmask (u32, HAL_IN)
- `pokeys.[DevID].PEv2.ProbeSpeed` — probe speed ratio (float, HAL_IN)
- `pokeys.[DevID].PEv2.digout.ExternalRelay-#.out` (4 pins) — relay outputs (bit, HAL_IN)
- `pokeys.[DevID].PEv2.digout.ExternalOC-#.out` (4 pins) — OC outputs (bit, HAL_IN)

## Acceptance Criteria

### Scenario 1 — All global pins exported

**Given** `export_pev2_pins()` is called with a valid prefix and comp_id
**When** `halcmd show pin` is run
**Then** all pins listed above are present with the correct HAL type and direction

### Scenario 2 — Output pins updated from device struct

**Given** `device->PEv2.PulseEngineState` is set to `3`
**When** the RT servo thread fires `update_pev2_hal_outputs()`
**Then** `pokeys.0.PEv2.PulseEngineState` reads `3`

### Scenario 3 — Input pins written to device struct

**Given** `pokeys.0.PEv2.HomingStartMaskSetup` is set to `0x03`
**When** the RT servo thread calls `apply_pev2_hal_inputs()`
**Then** `device->PEv2.HomingStartMaskSetup` equals `0x03`

## Verification Method

Test (automated) + Inspection

## Priority

P0 – Critical

## Traceability

- **Traces to**: #{{STR_NUM}}
- **Refined by**: (ADR issues — see comment)
- **Implemented in**: `PoKeysLibPulseEngine_v2Async.c` (`export_pev2_pins()`)
- **Verified by**: (TEST issues — add when created)
