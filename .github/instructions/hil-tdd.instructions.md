---
description: "Mandatory safety and integrity rules for PoKeys hardware-in-the-loop tests."
applyTo: "tests/hil/**"
---

# PoKeys HIL-TDD Contract

## Purpose

The PoKeys57E HIL fixture verifies real hardware behavior through a fixed loopback
wiring harness. It validates that PoKeysHal exports the expected HAL pins, applies
the expected pin functions, drives physical outputs, and observes physical inputs.

HIL tests are acceptance/verification tests. They do not replace host unit tests.

## Fixture identity

- Every HIL test must reference a registered setup ID and revision (e.g., `pokeys57e-loopback-v1`).
- Never run against a device whose model or identity does not match the selected setup.
- A missing device, failed preflight, or unavailable runner is SKIPPED/NOT RUN, never PASS.

## Safety rules

- Acquire exclusive fixture access before changing any output.
- Establish and verify the safe output state (all outputs false/inactive) before and after every test.
- Tests must manipulate only outputs declared by the setup capability list.
- Motion, pulse-engine, and emergency tests are disabled unless explicitly declared safe by the setup.
- Restore all outputs to their safe state on teardown, even after failure.
- Disable any PEv2 operations started by the test on cleanup.
- Unload test HAL components and release the fixture lock after every run.

## Scope: primitive HIL vs integration

Primitive HIL tests (`tests/hil/basic/`) must verify driver hardware-facing contracts
with the smallest possible HAL setup. They must NOT load:

- `motmod`, kinematics (`trivkins`, gantry), or AXIS UI
- the full `DM542_XXYZ_mill` HAL/INI stack
- `pokeys_homing.hal`
- `wcomp` window-comparator simulated switches

The `DM542_XXYZ_mill` configuration and `wcomp` logic belong in
`tests/integration/linuxcnc/`, not in `tests/hil/basic/`.

## Status vocabulary

Use precise status terms only:

| Term | Meaning |
|---|---|
| `HIL-observed` | Real board behavior was observed and recorded as a unit-test oracle |
| `HIL-tested` | The implementation was run against the physical fixture via an automated test |
| `HIL-verified` | A named HIL test passed against the named setup — does not imply RT-validated |
| `RT-validated` | Timing was measured on the RT path with a documented threshold |

Never claim `HIL-tested` from a mock, simulator, userspace-only run, or static review.

## Oracle traceability

Unit tests for hardware-facing behavior must not be based on developer assumption.
Before writing or changing a unit test expectation for any of the following, cite
a HIL observation or protocol specification:

- physical pin number to HAL channel index mapping
- digin/digout polarity and inversion semantics
- PEv2 limit/home/emergency switch mapping
- PWM/adcout scaling and analog input settling
- async update latency and timeout behavior
- behavior after disconnect/reconnect

Include a traceability comment: `/* Oracle: HIL-P57E-DIO-001 */`

## Evidence required per HIL run

Every HIL run must record: commit SHA, setup ID and revision, device model and ID,
firmware version, LinuxCNC version, kernel/RT mode, HAL configuration used, test
command, test result, observed transitions, cleanup result.

## PWM to analog loopback constraint

A directly connected PWM output is a 0/3.3 V square wave, not a DC analog signal.
An RC low-pass filter (e.g., 4.7 kΩ + 1 µF) is required at each PWM→ADC connection
for stable duty-cycle-to-voltage tests. Do not assert exact ADC values without a
measured settling baseline and documented tolerance.

## Emergency loopback

The emergency loopback (Pin 33 → Pin 52) is **excluded from scope** until hardware
confirmation resolves: (a) the wiring vs `PEv2_EmergencyInputPin=54` discrepancy,
(b) whether the INI parameter is a physical pin or API index, and (c) the role of
Pin 54 at startup. See open item HW-1/HW-2 in issue #138.

## Prohibited shortcuts

- Do not treat physical pin numbers and HAL channel indices as identical (physical N = HAL index N−1).
- Do not use sleep as the only synchronization method where HAL pin polling is possible.
- Do not run HIL tests on normal GitHub-hosted CI runners.
- Do not modify production HAL or C source files solely to make a HIL test pass.
- Do not ignore inverted limit/home semantics — separate raw electrical state from logical PEv2 state.
