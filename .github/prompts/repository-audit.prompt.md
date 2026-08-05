---
agent: agent
description: >
  Read-only audit workflow for the PoKeysHal repository. Assesses traceability,
  C99/PoKeysLib structure, async boundaries, HAL integration, RT constraints,
  protocol evidence, test layers, HIL fixture gating, and documentation consistency.
argument-hint: "Audit scope or issue number"
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

- Confirm issue types StR, REQ-F, REQ-NF, ADR, ARC-C, QA-SC, TEST exist with labels.
- Confirm REQ issues link upward to StR; TEST issues carry `Verifies: #N`.
- Confirm source uses `/* Implements: #N (REQ-F-xxx) */`; tests use `/* Verifies: #N */`.
- Confirm PRs use `Fixes #N` or `Implements #N`.
- For each in-scope approved acceptance criterion, identify the implementing
  artifact and verification evidence; flag missing or ambiguous links.
- Flag orphaned requirements, untraced tests, and PRs without an issue link.

### 2. C99 and PoKeysLib structure

- Confirm file responsibilities match `c-architecture-realtime.instructions.md`.
- Flag `malloc`, `calloc`, `realloc`, `free`, blocking calls, or unbounded
  loops on any RT-reachable path.

### 3. Async infrastructure / subsystem / integration-shell boundaries

- `PoKeysLibAsync.c`: mailbox and dispatch infrastructure only — no subsystem logic.
- `PoKeysLib*Async.c`: per-subsystem HAL export, send, parse, and optional scheduler
  registration — no transport logic. Flag logic that violates the file-responsibility
  contract in `c-architecture-realtime.instructions.md`.
- `experimental/pokeys_async.c`: integration shell only — no business logic.
- Report any cross-boundary leakage as a finding with file and line reference.

### 4. LinuxCNC HAL exports and component integration

For HAL-compatibility-focused audits, use:
- `.github/skills/hal-interface-compatibility/SKILL.md`
- `.github/skills/hal-interface-compatibility/references/classification-schema.md`
- `docs/hal-interface/phase-0/requirement-catalogue.yaml`
- `docs/hal-interface/phase-0/source-register.yaml`
- `docs/hal-interface/phase-0/legacy-pev2-parity.yaml`
- `docs/hal-interface/phase-0/conflict-register.md`
- `docs/hal-interface/phase-0/open-decisions.md`

Audit checks must confirm:
- changed HAL-facing behavior identifies affected interface IDs;
- authoritative, legacy, and current-observed contracts remain distinguished;
- recorded conflicts and decisions are respected;
- no accidental drift in naming, type, direction/access, cardinality,
  defaults, or scaling;
- HAL compatibility and async parity evidence are separated;
- homecomp internals are not introduced as pokeysHal requirements;
- compatibility claims identify `claim_evidence_level`;
- hardware claims use existing HIL result-schema terminology.

- HAL-visible struct members must use `hal_u32_t`, `hal_s32_t`, `hal_bit_t`, or
  `hal_float_t`. Flag any raw C type used where a HAL type is required.
- Confirm pin names follow the documented naming convention.
- Confirm each exported pin has a reachable update path and is updated according to
  the subsystem or component contract.
- Confirm the userspace and RT component entry points match the integration shell.

### 5. Verification layers

Inspect existing evidence for each layer. Do not execute build commands or hardware
operations; when no recorded evidence exists for a layer, report it as unavailable.

| Layer | Evidence to inspect |
|---|---|
| Compile and link | CI build logs, recorded `test_compile.sh` or `make` output |
| Unit tests | Test reports in `tests/`, CI run results |
| Protocol tests | Test reports covering command IDs, offsets, retry, timeout |
| Async mailbox tests | Test reports covering mailbox matching, stale response, timeout |
| Userspace HAL smoke | Recorded `halrun` output, CI component-load results |
| RT-environment | RT validation records, CI RT job results |
| HIL tests | HIL reports on fixtures with `runnable: true`, `fixture_status: verified` |
| Timing validation | Measurement records against a documented threshold |

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
- HIL evidence must name the registered setup ID and fixture revision; anonymous
  or revision-less results are insufficient.
- Status terms (`HIL-observed`, `HIL-test-executed`, `HIL-verified`) and test
  outcomes (`PASS`, `FAIL`, `ERROR`, `SKIPPED`) are defined in
  `.github/skills/hil-tdd/references/result-schema.md`.
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

Apply these terms consistently in findings. Claims without corresponding evidence
are overstatements; record them as findings.

Repository-level status terms:

| Term | Meaning |
|---|---|
| Implemented | Production behavior exists in source |
| Compiled | Compilation succeeded only |
| Tested | Named tests executed and passed |
| RT-validated | Tested in the applicable LinuxCNC RT environment |
| Timing-validated | Measured against a documented threshold with an identified method |

HIL status terms (`HIL-observed`, `HIL-test-executed`, `HIL-verified`) are defined in
`.github/skills/hil-tdd/references/result-schema.md` — that file is the single authority;
do not redefine them here.

## Unavailable checks

State explicitly which layers could not be assessed and why (no hardware, no RT
kernel, no registered fixture, no CI output). Do not substitute compilation for
hardware or timing validation.

## Output format

1. **Findings** — one entry per observation, with:
   - area number;
   - exact citation: `file:line`, issue or PR reference, CI run or log, test
     report, or HIL artifact, as applicable;
   - observed state;
   - required state;
   - risk level.

   When a required verification layer could not be assessed, note it as
   **unavailable** and mark the overall assessment **partially verified**.

2. **Corrective plan** — prioritized list of corrective actions, each with an
   appropriate existing issue type (for example: REQ-F, REQ-NF, ADR, QA-SC,
   TEST, bug, integration, or documentation) and a link to the finding it
   addresses.
