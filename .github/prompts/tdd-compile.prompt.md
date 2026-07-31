---
mode: agent
description: >
  Narrow workflow entry point for approved PoKeysHal implementation work.
  Routes to TDDDriver. Applicable to C99 sources, async subsystems, HAL exports,
  protocol handling, and RT-safe implementation.
applyTo:
  - "**/*.c"
  - "**/*.h"
  - "**/05-implementation/**/*"
---

# TDD Compile – PoKeysHal Implementation Workflow

Entry point for implementing an approved GitHub issue through Red-Green-Refactor.
Implementation behavior is owned by the **TDDDriver** agent; invoke it with the issue number.
This prompt defines the required sequence, evidence contract, and routing.

## Prepare

Before editing any file:

1. Read the issue, its acceptance criteria, and linked architecture decisions.
2. Run the narrowest relevant build or test and record pass/fail.
3. Identify the affected boundary: protocol/parser, async mailbox or scheduler,
   PoKeysLib subsystem, HAL export, integration shell, or RT-reachable path.
4. Establish expected behavior from the PoKeys protocol specification, existing
   verified behavior, or a recorded HIL observation. Do not invent hardware-facing
   expectations.

```bash
bash test_compile.sh          # compile-check baseline
make -f Makefile.noqmake      # library build baseline
```

Separate pre-existing failures from the new Red result before proceeding.

## Red

Write or update a deterministic check before changing production code.

The Red result must:
- fail on the current implementation;
- fail for the intended missing or defective behavior — not a build or fixture problem;
- carry traceability to the issue or acceptance criterion (`/* Verifies: #N */`).

Valid Red checks for PoKeysHal:
- C unit test for parsing, conversion, state transition, or mapping;
- protocol test for command ID, response offset, retry, timeout, or error;
- async mailbox or scheduler regression test;
- HAL export or userspace component check;
- compile or link regression when compilation is the requirement.

A missing device, unavailable HIL fixture, or failed preflight is not a valid Red result.
If issue, architecture, protocol specification, or implementation conflict, resolve the
conflict before changing behavior.

## Green

Implement the smallest complete change that makes the Red check pass without
weakening or deleting any existing check.

Preserve:
- C99 compatibility;
- PoKeysLib subsystem boundaries and the async infrastructure / subsystem /
  integration-shell separation defined in `c-architecture-realtime.instructions.md`;
- protocol command codes, byte offsets, masks, and response semantics;
- existing HAL names and ABI unless the issue explicitly changes them;
- bounded execution and the absence of dynamic allocation on RT-reachable paths.

No speculative abstractions, unrelated cleanup, or parallel variants.

## Refactor

After Green: remove local duplication, improve naming, simplify control flow.
Do not introduce additional behavior. Keep all applicable checks green.

## Verify outward

Run checks narrowest to broadest. Stop at the first failure and diagnose before
proceeding outward.

1. focused unit or protocol test;
2. affected library or component build (`make -f Makefile.noqmake`);
3. repository compile check (`bash test_compile.sh`);
4. userspace HAL smoke test, when applicable (`halrun -f <test.hal>`);
5. RT-environment validation, when applicable;
6. HIL confirmation for hardware-dependent acceptance criteria — use the **hil-tdd** skill.

## Hardware-dependent behavior

For acceptance criteria that require real hardware, use the **hil-tdd** skill.
Do not duplicate that procedure here.

HIL status vocabulary is defined in
`.github/skills/hil-tdd/references/result-schema.md`:
`HIL-observed`, `HIL-test-executed`, `HIL-verified`, `RT-validated`, `Timing-validated`.

Do not claim `HIL-verified` from a mock, simulator, or userspace-only run.

## Traceability

- Source: `/* Implements: #N (REQ-F-xxx) */`
- Tests:  `/* Verifies: #N */`
- PRs:    `Fixes #N` or `Implements #N`

## Handoff evidence

Report exactly:
- files changed and requirement addressed;
- tests added or changed and commands executed with results;
- unavailable checks and why;
- remaining risks;
- precise status from:
  **Implemented · Compiled · Tested · HIL-test-executed · HIL-verified ·
  RT-validated · Timing-validated**
