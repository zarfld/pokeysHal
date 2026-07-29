## Functional Requirement

**Parent Stakeholder Requirement**: Traces to: #{{STR_NUM}} (HAL Interface: PEv2)

## Requirement Description

The PEv2 HAL component **shall** call the correct PoKeysLib functions in the correct order
each servo cycle, using the async (non-blocking) variants:

### Per-cycle read sequence (device → HAL):
1. `PK_PEv2_StatusGetAsync(device)` — updates `AxesState[]`, `CurrentPosition[]`,
   `ExternalRelayOutputs`, `ExternalOCOutputs`, `ProbeStatus`, `BacklashRegister[]`,
   pulse engine status fields
2. `PK_PEv2_Status2GetAsync(device)` — updates `DedicatedLimitNInputs`,
   `DedicatedLimitPInputs`, `DedicatedHomeInputs`
3. `PK_PEv2_AdditionalParametersGetAsync(device)` — updates `EmergencyInputPin`

### Per-cycle write sequence (HAL → device):
1. `PK_PEv2_PulseEngineSetupAsync(device)` — sends `PulseEngineEnabled`,
   `ChargePumpEnabled`, `PulseGeneratorType`, `EmergencySwitchPolarity`
2. `PK_PEv2_AxisConfigurationSetAsync(device)` — sends per-axis configuration for any
   axis whose configuration HAL inputs have changed
3. `PK_PEv2_ExternalOutputsSetAsync(device)` — sends `ExternalRelayOutputs`,
   `ExternalOCOutputs`

### On-demand operations (triggered by HAL pins):
- `PK_PEv2_HomingStartAsync(device)` — triggered when `HomingStartMaskSetup` changes from 0
- `PK_PEv2_PulseEngineMovePVAsync(device)` — triggered each cycle when motion is active
- `PK_PEv2_BufferFillAsync(device)` — triggered when motion buffer mode is active

All device function calls **shall** use the async non-blocking variants (ending in `Async`)
to maintain RT compatibility.

## Acceptance Criteria

### Scenario 1 — Status get called each cycle

**Given** the RT servo thread is active
**When** 100 servo cycles elapse
**Then** `PK_PEv2_StatusGetAsync()` has been scheduled via `register_async_task()` at the
configured frequency (default: 1 kHz)

### Scenario 2 — Correct command sequence on startup

**Given** the HAL component starts successfully
**When** the first 3 servo cycles execute
**Then** `PK_PEv2_PulseEngineSetupAsync()` has been called at least once

### Scenario 3 — No blocking calls in RT path

**Given** the RT servo thread is running
**When** the PEv2 update function is called
**Then** no `SendRequest()` (blocking) calls are present in the call chain (verified by
code inspection)

## Verification Method

Inspection + Test

## Priority

P0 – Critical

## Traceability

- **Traces to**: #{{STR_NUM}}
- **Refined by**: (ADR issues — see comment)
- **Implemented in**: `PoKeysLibPulseEngine_v2Async.c`
- **Verified by**: (TEST issues — add when created)
