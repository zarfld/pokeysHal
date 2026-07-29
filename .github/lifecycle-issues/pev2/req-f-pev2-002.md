## Functional Requirement

**Parent Stakeholder Requirement**: Traces to: #{{STR_NUM}} (HAL Interface: PEv2)

## Requirement Description

The HAL component **shall** export per-axis PEv2 pins for each axis index 0–7, using the
naming convention `pokeys.[DevID].PEv2.[AxisID].<pin-name>`.

**Per-axis output pins** (updated from `device->PEv2.*[axis]`):
- `pokeys.[DevID].PEv2.[N].AxesState` — axis state (u32, HAL_OUT)
- `pokeys.[DevID].PEv2.[N].AxesConfig` — axis configuration flags (u32, HAL_OUT)
- `pokeys.[DevID].PEv2.[N].AxesSwitchConfig` — switch configuration flags (u32, HAL_OUT)
- `pokeys.[DevID].PEv2.[N].SoftLimitMaximum` — soft limit max (s32, HAL_OUT)
- `pokeys.[DevID].PEv2.[N].SoftLimitMinimum` — soft limit min (s32, HAL_OUT)
- `pokeys.[DevID].PEv2.[N].HomingSpeed` — homing speed % (u32, HAL_OUT)
- `pokeys.[DevID].PEv2.[N].HomingReturnSpeed` — homing return speed % (u32, HAL_OUT)
- `pokeys.[DevID].PEv2.[N].HomingAlgorithm` — homing algorithm (u32, HAL_OUT)
- `pokeys.[DevID].PEv2.[N].HomeOffsets` — home offset (s32, HAL_OUT)
- `pokeys.[DevID].PEv2.[N].ProbePosition` — probe detected position (s32, HAL_OUT)
- `pokeys.[DevID].PEv2.[N].ProbeMaxPosition` — max probe travel (s32, HAL_OUT)
- `pokeys.[DevID].PEv2.[N].CurrentPosition` — current position in pulses (s32, HAL_OUT)
- `pokeys.[DevID].PEv2.[N].MaxSpeed` — max speed pulses/ms (float, HAL_OUT)
- `pokeys.[DevID].PEv2.[N].MaxAcceleration` — max accel pulses/ms² (float, HAL_OUT)
- `pokeys.[DevID].PEv2.[N].MPGjogMultiplier` — MPG jog multiplier (s32, HAL_OUT)
- `pokeys.[DevID].PEv2.[N].MPGjogEncoder` — MPG encoder ID (u32, HAL_OUT)
- `pokeys.[DevID].PEv2.[N].MPGjogDivider` — MPG divider (u32, HAL_OUT)
- `pokeys.[DevID].PEv2.[N].digin.LimitN.in` — limit- switch (bit, HAL_OUT)
- `pokeys.[DevID].PEv2.[N].digin.LimitN.in-not` — limit- inverted (bit, HAL_OUT)
- `pokeys.[DevID].PEv2.[N].digin.LimitN.DedicatedInput` — dedicated limit- (bit, HAL_OUT)
- `pokeys.[DevID].PEv2.[N].digin.LimitP.in` — limit+ switch (bit, HAL_OUT)
- `pokeys.[DevID].PEv2.[N].digin.LimitP.in-not` — limit+ inverted (bit, HAL_OUT)
- `pokeys.[DevID].PEv2.[N].digin.LimitP.DedicatedInput` — dedicated limit+ (bit, HAL_OUT)
- `pokeys.[DevID].PEv2.[N].digin.Home.in` — home switch (bit, HAL_OUT)
- `pokeys.[DevID].PEv2.[N].digin.Home.in-not` — home switch inverted (bit, HAL_OUT)
- `pokeys.[DevID].PEv2.[N].digin.Home.DedicatedInput` — dedicated home (bit, HAL_OUT)
- `pokeys.[DevID].PEv2.[N].digin.Error.in` — driver error (bit, HAL_OUT)
- `pokeys.[DevID].PEv2.[N].digin.Error.in-not` — driver error inverted (bit, HAL_OUT)
- `pokeys.[DevID].PEv2.[N].digin.Probe.in` — probe per-axis (bit, HAL_OUT)
- `pokeys.[DevID].PEv2.[N].digin.Probe.in-not` — probe per-axis inverted (bit, HAL_OUT)
- `pokeys.[DevID].PEv2.[N].digin.SoftLimit.in` — soft limit status (bit, HAL_OUT)
- `pokeys.[DevID].PEv2.[N].digin.AxisEnabled.in` — axis enabled status (bit, HAL_OUT)
- `pokeys.[DevID].PEv2.[N].BacklashWidth` — backlash half-width (u32, HAL_OUT)
- `pokeys.[DevID].PEv2.[N].BacklashRegister` — backlash register (s32, HAL_OUT)
- `pokeys.[DevID].PEv2.[N].BacklashAcceleration` — backlash accel (float, HAL_OUT)
- `pokeys.[DevID].PEv2.[N].HomeBackOffDistance` — home back-off distance (s32, HAL_OUT)

**Per-axis input pins** (written to `device->PEv2.*[axis]`):
- `pokeys.[DevID].PEv2.[N].PositionSetup` — position to set as current (s32, HAL_IN)
- `pokeys.[DevID].PEv2.[N].ReferencePositionSpeed` — reference pos/speed (s32, HAL_IN)
- `pokeys.[DevID].PEv2.[N].digout.AxisEnable.out` — axis enable output (bit, HAL_IN)
- `pokeys.[DevID].PEv2.[N].digout.AxisEnabled.out` — axis enabled output (bit, HAL_IN)
- `pokeys.[DevID].PEv2.[N].digout.LimitOverride.out` — limit override (bit, HAL_IN)

**Per-axis parameters**:
- `pokeys.[DevID].PEv2.[N].digin.SoftLimit.PosMin` — soft limit min position
- `pokeys.[DevID].PEv2.[N].digin.SoftLimit.PosMax` — soft limit max position
- `pokeys.[DevID].PEv2.[N].digin.Home.Offset` — home offset
- `pokeys.[DevID].PEv2.[N].digin.LimitN.Pin` — limit- switch pin
- `pokeys.[DevID].PEv2.[N].digin.LimitN.Filter` — limit- filter
- `pokeys.[DevID].PEv2.[N].digin.LimitN.invert` — invert limit-
- `pokeys.[DevID].PEv2.[N].digin.LimitP.Pin` — limit+ switch pin
- `pokeys.[DevID].PEv2.[N].digin.LimitP.Filter` — limit+ filter
- `pokeys.[DevID].PEv2.[N].digin.LimitP.invert` — invert limit+
- `pokeys.[DevID].PEv2.[N].digout.AxisEnable.Pin` — axis enable output pin
- `pokeys.[DevID].PEv2.[N].digout.AxisEnable.invert` — invert axis enable
- `pokeys.[DevID].PEv2.[N].digin.Home.Pin` — home switch pin
- `pokeys.[DevID].PEv2.[N].digin.Home.Filter` — home switch filter
- `pokeys.[DevID].PEv2.[N].digin.Home.invert` — invert home switch

## Acceptance Criteria

### Scenario 1 — All per-axis pins exported

**Given** `export_pev2_pins()` is called for a device with 4 enabled axes
**When** `halcmd show pin` is run
**Then** pins for axes 0–3 with all names above are visible; axes 4–7 have no pins exported

### Scenario 2 — CurrentPosition updated from device

**Given** `device->PEv2.CurrentPosition[2]` is set to `12345`
**When** the RT servo thread fires and updates HAL outputs
**Then** `pokeys.0.PEv2.2.CurrentPosition` reads `12345`

### Scenario 3 — ReferencePositionSpeed written to device

**Given** `pokeys.0.PEv2.0.ReferencePositionSpeed` is set to `500`
**When** the RT servo thread calls the input update function
**Then** `device->PEv2.ReferencePositionSpeed[0]` equals `500`

## Verification Method

Test (automated) + Inspection

## Priority

P0 – Critical

## Traceability

- **Traces to**: #{{STR_NUM}}
- **Refined by**: (ADR issues — see comment)
- **Implemented in**: `PoKeysLibPulseEngine_v2Async.c` (`export_pev2_pins()`)
- **Verified by**: (TEST issues — add when created)
