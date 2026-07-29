## Functional Requirement

**Parent Stakeholder Requirement**: Traces to: #{{STR_NUM}} (HAL Interface: PEv2)

## Requirement Description

The HAL component **shall** create per-axis HAL pins only for axes that are required and
enabled.  An axis is considered enabled when its `AxesConfig` field has the `PK_AC_ENABLED`
bit set (`(1 << 0)`).

The number of axes to process is determined by `device->PEv2.info.nrOfAxes`.

**Rules**:
1. Per-axis pin groups (index 0 to `nrOfAxes - 1`) **shall** be created.
2. Per-axis pin groups for indices `nrOfAxes` to 7 **shall not** be created.
3. The global `pokeys.[DevID].PEv2.nrOfAxes` pin **shall** reflect the actual axis count
   returned by the device.
4. If `nrOfAxes` is 0 (device not responding), no per-axis pins **shall** be created and a
   warning **shall** be emitted.

## Acceptance Criteria

### Scenario 1 — 4-axis device

**Given** `device->PEv2.info.nrOfAxes` equals `4`
**When** `export_pev2_pins()` completes
**Then** pins `pokeys.0.PEv2.0.*` through `pokeys.0.PEv2.3.*` exist
**And** pins `pokeys.0.PEv2.4.*` through `pokeys.0.PEv2.7.*` do NOT exist

### Scenario 2 — All 8 axes

**Given** `device->PEv2.info.nrOfAxes` equals `8`
**When** `export_pev2_pins()` completes
**Then** pins `pokeys.0.PEv2.0.*` through `pokeys.0.PEv2.7.*` all exist

### Scenario 3 — Zero axes (error/unconnected)

**Given** `device->PEv2.info.nrOfAxes` equals `0`
**When** `export_pev2_pins()` completes
**Then** no per-axis pins are created
**And** a warning message is emitted to the HAL log

## Verification Method

Test (automated) + Inspection

## Priority

P1 – High

## Traceability

- **Traces to**: #{{STR_NUM}}
- **Refined by**: (ADR issues — see comment)
- **Implemented in**: `PoKeysLibPulseEngine_v2Async.c` (`export_pev2_pins()`)
- **Verified by**: (TEST issues — add when created)
