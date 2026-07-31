# tests/hil/basic — Primitive HIL Tests

## Scope

These tests verify PoKeysHal driver behavior against the physical PoKeys57E
loopback fixture using the **smallest possible HAL configuration**. They test
driver hardware contracts — not machine behavior.

## What belongs here

- Direct digital output → input loopback (HIL-010)
- PWM/ADC loopback (HIL-020, requires RC filter hardware, not yet in v1)
- Pulse engine → fast encoder feedback (HIL-030, pending HW-3 resolution)

## What does NOT belong here

The following must not be loaded from any test in this directory:

- `motmod` or any LinuxCNC kinematics component
- `trivkins`, `gantrykins`, or AXIS/Touchy UI
- The full `DM542_XXYZ_mill` HAL/INI stack
- `pokeys_homing.hal`
- `wcomp` window-comparator simulated switches

If a test requires any of those, it belongs in `tests/hil/machine/linuxcnc/`.

## Directory layout

```
tests/hil/basic/
  README.md               — this file
  hal/                    — minimal HAL files (one per test category)
  pytest/                 — pytest test cases
  conftest.py             — fixture identity check, exclusive lock, safe-state setup/teardown
```

## Fixture

All tests in this directory must use setup ID: `pokeys57e-loopback-v1`

The fixture definition is at: `tests/hil/setups/pokeys57e-loopback-v1.yaml`

Tests must be guarded by `POKEYS_HIL=1` environment variable. When the variable
is absent, record `SKIPPED`. When the test suite is discovered on a non-HIL runner
without HIL being requested, record `SKIPPED`. When `POKEYS_HIL=1` but the runner
lacks the fixture, the fixture is draft or not runnable, or the device identity
cannot be confirmed, record `ERROR`. Use `FAIL` only when the hardware behavior
diverges from the expected oracle.

## Test order

Tests must run in ID order. Each layer depends on the previous passing.

| ID | Name | Status |
|---|---|---|
| HIL-000 | Identity and HAL export | Not yet implemented |
| HIL-010 | Direct digital loopback | Not yet implemented |
| HIL-020 | PWM/ADC loopback | Excluded from v1 (RC filter required) |
| HIL-030 | PE to fast encoder loopback | Excluded from v1 (HW-3 open) |
| HIL-040 | Emergency loopback | Excluded from v1 (HW-1/HW-2 open) |

## Running

```bash
# Requires physical PoKeys57E fixture and self-hosted runner
POKEYS_HIL=1 POKEYS_DEVICE_ID=<verified-expected-device-id-from-fixture> pytest -m hil tests/hil/basic -v
```

A self-hosted HIL workflow is planned but not implemented in this revision.
Run no automated HIL tests until the fixture is verified (`runnable: true`)
and a runner with the physical device is available.
Do not run on standard GitHub-hosted CI.
