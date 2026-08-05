---
agent: TDDDriver
description: >
  Narrow workflow entry point for approved PoKeysHal implementation work.
  Routes to TDDDriver.
argument-hint: "Issue #N"
---

# TDD Compile – PoKeysHal

Invoke the **TDDDriver** agent for the approved issue.

The detailed Red-Green-Refactor procedure, file-boundary rules, protocol
constraints, and HIL procedure are owned by TDDDriver and the hil-tdd skill.
This prompt states the orchestration contract only.

## Contract

The TDDDriver must:

0. **HAL/Async compatibility gate** — detect whether the issue can affect
   HAL-visible behavior or async parity. When applicable, invoke
   `.github/skills/hal-interface-compatibility/SKILL.md` and report
   `affected_interface_ids` and `decision_gate` (per
   `.github/skills/hal-interface-compatibility/references/classification-schema.md`).
   Do not proceed into production Red-Green-Refactor when gate outcome is:
   `characterization-only`, `decision-required`, `evidence-required`,
   `deferred`, or `out-of-scope`.
   For `characterization-only`, deterministic characterization tests are
   allowed, but no production behavior change.

1. **Baseline** — run the narrowest relevant build or test before editing;
   record pass/fail; separate pre-existing failures from the new Red result.

2. **Red** — write or update a deterministic check that:
   - fails on the current implementation;
   - fails for the intended missing or defective behavior, not a build or
     fixture problem;
   - carries traceability to the issue or acceptance criterion
     (`/* Verifies: #N */`).

3. **Green** — implement the smallest complete change that makes Red pass
   without weakening or removing any existing check. Preserve C99,
   PoKeysLib subsystem boundaries, async infrastructure / subsystem /
   integration-shell separation, protocol semantics, HAL ABI, and bounded
   execution on RT-reachable paths.

4. **Refactor** — remove duplication and simplify without changing observable
   behavior. Keep all applicable checks green.

5. **Verify outward** — through the applicable layers, from narrowest to
   broadest: unit or protocol test, library build, repository compile check,
   userspace HAL smoke test, RT-environment validation, HIL confirmation for
   hardware-dependent criteria. Stop at the first failure and diagnose.

6. **Hardware-dependent criteria** — use the **hil-tdd** skill.
   Do not treat hardware absence as a functional pass.

## Traceability

- Source: `/* Implements: #N (REQ-F-xxx) */`
- Tests:  `/* Verifies: #N */`
- PRs:    `Fixes #N` or `Implements #N`

## Required handoff

Report: files changed, requirement addressed, tests added or changed, exact
commands and results, unavailable checks and why, remaining risks, and
precise status:

**Implemented · Compiled · Tested · HIL-observed · HIL-test-executed ·
HIL-verified · RT-validated · Timing-validated**

When a required verification layer is unavailable, report as
**partially verified**. HIL status vocabulary:
`.github/skills/hil-tdd/references/result-schema.md`.
