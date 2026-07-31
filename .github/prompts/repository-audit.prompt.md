---
mode: agent
description: >
  Read-only audit workflow for the PoKeysHal repository. Assesses traceability,
  C99/PoKeysLib structure, async boundaries, HAL integration, RT constraints,
  protocol evidence, test layers, HIL fixture gating, and documentation consistency.
applyTo:
  - "**/.github/**/*.md"
  - "**/docs/**/*.md"
  - "**/README.md"
---

# Repository Audit – PoKeysHal

Focused read-only audit of the PoKeysHal repository.
Produce findings and a corrective plan only.

Do not edit source files, issue templates, customization files, or tests.
Do not execute build commands, hardware operations, or CI workflows.
Do not redesign or replace the approved HIL baseline from issues #138 and #139.
Do not duplicate complete content from agents, instructions, or skills.

## Audit areas

### 1. Issue and acceptance-criterion traceability

- Confirm issue types StR, REQ-F, REQ-NF, ADR, ARC-C, TEST exist with labels.
- Confirm REQ issues link upward to StR; TEST issues carry `Verifies: #N`.
- Confirm source uses `/* Implements: #N (REQ-F-xxx) */`; tests use `/* Verifies: #N */`.
- Confirm PRs use `Fixes #N` or `Implements #N`.
- Flag orphaned requirements, untraced tests, and PRs without an issue link.

### 2. C99 and PoKeysLib structure

- Confirm file responsibilities match `c-architecture-realtime.instructions.md`.
- Flag VLA, `malloc`, `calloc`, `realloc`, or unbounded loops on any RT-reachable path.
- Flag use of prohibited C features (dynamic allocation, blocking calls, non-constant
  bounds) in files included on RT-reachable call paths.

### 3. Async infrastructure / subsystem / integration-shell boundaries

- `PoKeysLibAsync.c`: mailbox and dispatch infrastructure only — no subsystem logic.
- `PoKeysLib*Async.c`: per-subsystem HAL export, send, parse, and optional scheduler
  registration only — no cross-subsystem calls, no transport logic.
- `experimental/pokeys_async.c`: integration shell only — no business logic.
- Report any cross-boundary leakage as a finding with file and line reference.

### 4. LinuxCNC HAL exports and component integration

- HAL-visible struct members must use `hal_u32_t`, `hal_s32_t`, `hal_bit_t`, or
  `hal_float_t`. Flag any raw C type used where a HAL type is required.
- Confirm pin names follow the documented naming convention.
- Confirm exported pins are updated on every HAL function invocation.
- Confirm the userspace and RT component entry points match the integration shell.

### 5. Verification layers

Assess what evidence exists at each layer:

| Layer | How assessed |
|---|---|
| Compile and link | `bash test_compile.sh`, `make -f Makefile.noqmake` results |
| Unit tests | parsing, conversion, state transitions in `tests/` |
| Protocol tests | command IDs, offsets, retry, timeout coverage |
| Async mailbox tests | mailbox matching, stale response, timeout exhaustion |
| Userspace HAL smoke | `halrun` component load, pin export, basic update |
| RT-environment | LinuxCNC RT load and execution evidence |
| HIL tests | named tests on verified/runnable fixtures |
| Timing validation | measured result against documented threshold |

Note which layers have no evidence; mark them as gaps.

### 6. RT-path constraints

- No blocking calls, dynamic allocation, or unbounded loops on any path reachable
  from a LinuxCNC real-time function.
- Claims of RT-safety require RT-validated evidence. A non-blocking appearance or
  successful compilation does not constitute RT-validated status.
- Flag any `RT-safe` or equivalent claim without supporting RT-validated evidence.

### 7. Protocol-specification evidence

- Each command code, byte offset, mask, and response field must be traceable to the
  PoKeys protocol specification (comment, issue, or documented reference).
- Flag undocumented constants, inferred offsets, and values copied from apparently
  similar subsystems without a spec citation.

### 8. HIL fixture gating

- A fixture must have `runnable: true` and `fixture_status: verified` before
  it may be used in automation.
- Draft fixtures must not be treated as verified.
- Use `.github/skills/hil-tdd/references/result-schema.md` for outcome vocabulary:
  `HIL-observed`, `HIL-test-executed`, `HIL-verified`.
- A required HIL job that produces zero test results must not be reported as green.

### 9. Documentation consistency

- Documentation must describe implemented behavior only. Claims beyond
  Implemented/Compiled/Tested are findings unless supported by named evidence.
- Flag documentation that describes planned or aspirational behavior as fact.
- Flag any active customization file (`.github/agents/`, `.github/instructions/`,
  `.github/skills/`, `.github/prompts/`) with duplicate or conflicting guidance.
- Each layer has a defined scope: prompts are entry points; agents own role behavior;
  instructions own mandatory invariants; skills own detailed repeatable procedures.

## Evidence and claim vocabulary

Use these terms consistently in findings:

| Term | Meaning |
|---|---|
| Implemented | Production behavior exists in source |
| Compiled | Compilation succeeded only |
| Tested | Named tests executed and passed |
| HIL-observed | Exploratory observation recorded as oracle evidence |
| HIL-test-executed | Named HIL test ran to completion |
| HIL-verified | Named HIL test passed on a named fixture at a stated revision |
| RT-validated | Tested in the applicable LinuxCNC RT environment |
| Timing-validated | Measured against a documented threshold with an identified method |

Claims without corresponding evidence are overstatements; record them as findings.

## Unavailable checks

State explicitly which layers could not be assessed and why (no hardware, no RT
kernel, no registered fixture, no CI output). Do not substitute compilation for
hardware or timing validation.

## Output format

1. **Findings** — one entry per issue, with: area number, file or issue reference,
   observed state, required state, risk level.

2. **Corrective plan** — prioritized list with recommended issue type (REQ-F, TEST,
   ARC-C, or process change) and link to the finding it addresses.
