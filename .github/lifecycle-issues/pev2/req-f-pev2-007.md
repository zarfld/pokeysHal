## Functional Requirement

**Parent Stakeholder Requirement**: Traces to: #{{STR_NUM}} (HAL Interface: PEv2)

## Requirement Description

The HAL component **shall** export the following HAL **parameters** (not pins) for PEv2
configuration values that are set once at startup and do not change during operation:

**Global parameters**:
- `pokeys.[DevID].PEv2.digin.Emergency.Pin` — emergency input pin ID (u32 param)
- `pokeys.[DevID].PEv2.digin.Emergency.invert` — emergency switch polarity invert (bit param)
- `pokeys.[DevID].PEv2.digout.Emergency.Pin` — emergency output pin ID (u32 param)
- `pokeys.[DevID].PEv2.digin.Probe.Pin` — probe input pin (0=disabled, 1–8=external, 9+=pin ID-9) (u32 param)
- `pokeys.[DevID].PEv2.digin.Probe.invert` — probe input polarity invert (bit param)

**Per-axis parameters** (for each axis 0–7 that is enabled):
- `pokeys.[DevID].PEv2.[N].digin.SoftLimit.PosMin` — soft limit minimum position (float param)
- `pokeys.[DevID].PEv2.[N].digin.SoftLimit.PosMax` — soft limit maximum position (float param)
- `pokeys.[DevID].PEv2.[N].digin.Home.Offset` — home offset (float param)
- `pokeys.[DevID].PEv2.[N].digin.LimitN.Pin` — limit- switch pin ID (0=dedicated external) (u32 param)
- `pokeys.[DevID].PEv2.[N].digin.LimitN.Filter` — digital filter for limit- (u32 param)
- `pokeys.[DevID].PEv2.[N].digin.LimitN.invert` — invert limit- (bit param)
- `pokeys.[DevID].PEv2.[N].digin.LimitP.Pin` — limit+ switch pin ID (u32 param)
- `pokeys.[DevID].PEv2.[N].digin.LimitP.Filter` — digital filter for limit+ (u32 param)
- `pokeys.[DevID].PEv2.[N].digin.LimitP.invert` — invert limit+ (bit param)
- `pokeys.[DevID].PEv2.[N].digout.AxisEnable.Pin` — axis enable output pin ID (u32 param)
- `pokeys.[DevID].PEv2.[N].digout.AxisEnable.invert` — invert axis enable signal (bit param)
- `pokeys.[DevID].PEv2.[N].digin.Home.Pin` — home switch pin ID (u32 param)
- `pokeys.[DevID].PEv2.[N].digin.Home.Filter` — digital filter for home switch (u32 param)
- `pokeys.[DevID].PEv2.[N].digin.Home.invert` — invert home switch (bit param)

Parameters **shall** be created using `hal_param_*_newf()` (not `hal_pin_*_newf()`).

## Acceptance Criteria

### Scenario 1 — Parameters visible in halcmd

**Given** `export_pev2_pins()` completes for a 2-axis device
**When** `halcmd show param` is run
**Then** `pokeys.0.PEv2.digin.Emergency.Pin` and `pokeys.0.PEv2.0.digin.LimitN.Pin`
are present as parameters (not pins)

### Scenario 2 — Parameter values persist

**Given** `pokeys.0.PEv2.0.digin.LimitN.Pin` is set to `5` via halcmd
**When** the component reads the parameter
**Then** `device->PEv2.PinLimitMSwitch[0]` is written with `5` during configuration setup

### Scenario 3 — Filter parameter present

**Given** `export_pev2_pins()` for axis 1
**When** `halcmd show param pokeys.0.PEv2.1.digin.Home.Filter` is run
**Then** the parameter exists with type `u32` and default value `0`

## Verification Method

Inspection + Test

## Priority

P1 – High

## Traceability

- **Traces to**: #{{STR_NUM}}
- **Refined by**: (ADR issues — see comment)
- **Implemented in**: `PoKeysLibPulseEngine_v2Async.c` (`export_pev2_pins()`)
- **Verified by**: (TEST issues — add when created)
