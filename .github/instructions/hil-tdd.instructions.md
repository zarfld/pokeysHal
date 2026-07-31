---
description: "Mandatory safety and integrity rules for PoKeys hardware-in-the-loop tests."
applyTo: "tests/hil/**"
---

# PoKeys HIL-TDD Contract

HIL tests verify real hardware behavior; they do not replace host unit tests.

## Fixture gate

Never run a fixture whose YAML does not have `runnable: true` and
`fixture_status: verified`. Draft fixtures are not valid for automation.

## Safety

- Acquire exclusive fixture access before changing any output.
- Verify the per-channel safe state from the fixture YAML before each run.
- At most one endpoint of a physical loopback may be an active output.
- If the destination mode cannot be verified, do not energize the source.
- Direct GPIO loopbacks require confirmed series-current protection before unattended automation.
- Restore safe state on teardown and unload HAL components after the run.

## Scope

`tests/hil/basic/` is for primitive driver tests only; `tests/hil/machine/linuxcnc/` is for full machine integration.

## Outcomes and evidence terms

Use `.github/skills/hil-tdd/references/result-schema.md`.
A required HIL job that produces zero results must not be green.

## Oracle traceability

Unit tests for hardware-facing behavior must cite a verified observation or spec.
Include `/* Oracle: HIL-P57E-DIO-001 */`. Physical pin N maps to HAL channel index N−1; store and report both values separately.


