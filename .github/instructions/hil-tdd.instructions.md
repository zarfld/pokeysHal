---
description: "Mandatory safety and integrity rules for PoKeys hardware-in-the-loop tests."
applyTo: "tests/hil/**"
---

# PoKeys HIL-TDD Contract

HIL tests verify real hardware behavior; they do not replace host unit tests.

## Fixture runnable gate

Never run a fixture whose YAML does not have `runnable: true` and
`fixture_status: verified`. A draft fixture must not be used for automated runs.

## Anti-contention rule

- Before enabling a source as an output, verify the destination is configured as
  an input, counter, analog input, or declared peripheral input.
- At most one endpoint of a physical loopback may be an active output at any time.
- If endpoint direction cannot be verified, do not energize the loopback.
- Series resistors (1–4.7 kΩ) are required at each direct GPIO loopback.

## Safety rules

- Acquire exclusive fixture access before changing any output.
- Safe state is defined per channel in the fixture YAML (`safe_logical_value`,
  `expected_safe_physical_level`). `false` ≠ electrically inactive for inverted outputs.
- Establish and verify the safe state on all source channels before any test.
- Manipulate only outputs declared in the setup capability list.
- Motion, pulse-engine, and emergency tests require explicit setup declaration.
- Restore all source channels to safe state on teardown, even after failure.
- Unload HAL components and release the fixture lock after every run.

## Primitive HIL — prohibited loads

`tests/hil/basic/` must not load: `motmod`, kinematics, AXIS UI,
`DM542_XXYZ_mill`, `pokeys_homing.hal`, or `wcomp`.
Machine config tests belong in `tests/hil/machine/linuxcnc/`.

## Preflight outcomes

| Condition | Result |
|---|---|
| `POKEYS_HIL` not set | SKIPPED |
| Non-HIL runner | SKIPPED |
| HIL requested but device absent / identity mismatch / revision mismatch | ERROR |
| Continuity/self-test fails | ERROR |
| Behavior differs from expectation | FAIL |
| Success | PASS |

A required HIL job that produces zero test results must not be green.

## Oracle traceability

Unit tests for hardware-facing behavior must cite a verified observation or spec.
Include: `/* Oracle: HIL-P57E-DIO-001 */`
Physical pin N = HAL channel index N−1. Never treat them as identical.

## Status terms — see `skills/hil-tdd/references/result-schema.md`

`HIL-observed` · `HIL-test-executed` · `HIL-verified` · `Timing-validated`
Do not redefine `RT-validated`; the engineering contract owns that term.


