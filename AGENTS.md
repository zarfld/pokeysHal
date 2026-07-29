# PoKeysHal — Agent Standing Rules

Applies to: GitHub Copilot, Claude Code, Codex, and any other AI agent operating on this repository.

Extended rationale for these rules is in `docs/engineering-discipline.md`.
Detailed per-file architecture contracts are in `.github/instructions/pokeyshal-architecture.instructions.md`.
Full 13-rule engineering discipline for C/implementation work is in `.github/instructions/engineering-discipline.instructions.md`.

---

## Repository Identity

- **PoKeysLibHal** is a C99 LinuxCNC HAL driver for PoKeys USB/Ethernet devices.
- Supports userspace and real-time (RTAI / PREEMPT_RT) operation.
- Communication is asynchronous and non-blocking; the mailbox/dispatch layer is in `PoKeysLibAsync.c`.
- `pokeyslib/` is the upstream synchronous reference; `hal-canon/` provides canonical HAL channel interfaces.
- Primary build: `sudo make -f Makefile.noqmake install`

---

## Critical Invariants

These apply unconditionally to all work in this repository:

- **No blocking calls, no dynamic allocation, no unbounded loops** in any code path reachable from a LinuxCNC real-time function.
- **Preserve async infrastructure / subsystem / integration-shell boundaries** — see `pokeyshal-architecture.instructions.md`.
- **Use HAL-compatible types** (`hal_u32_t`, `hal_s32_t`, `hal_bit_t`, `hal_float_t`) for all HAL-visible struct members.
- **Do not claim timing compliance without measurement.** "Compiles" ≠ "RT-safe". "Looks non-blocking" ≠ "timing-validated".
- **Do not change protocol behavior without checking the PoKeys protocol specification** for command codes, byte offsets, masks, and response semantics.

---

## Engineering Discipline

### 1. Understand Before Modifying

Before changing code, establish the applicable contract:

1. The linked GitHub issue, requirement, acceptance criteria, or explicit user request.
2. Existing architecture and file-responsibility instructions.
3. The current implementation and its callers.
4. The corresponding synchronous `PoKeysLib<Subsystem>.c` reference implementation, where applicable.
5. The PoKeys protocol specification for command codes, byte layouts, masks, limits, and response semantics.
6. Existing async subsystem implementations that demonstrate the repository's current pattern.
7. Existing tests, build workflows, and known limitations.

Do not infer protocol behavior from function names, comments, or apparently similar subsystems.
Do not silently choose between contradictory sources — identify the conflict and determine which source is authoritative.
First resolve questions by inspecting available sources. Ask the user only when a material ambiguity remains and different interpretations would change external behavior, protocol handling, HAL interfaces, safety, or real-time characteristics.

### 2. Establish a Baseline

Before modifying behavior:

- Run the narrowest relevant existing build or test.
- Record whether the baseline passes or fails.
- Separate pre-existing failures from failures introduced by the change.
- Do not describe an untested state as working.
- Do not weaken, delete, skip, or mark a failing check as non-fatal merely to obtain a green result.

When hardware, LinuxCNC, or the real-time kernel is unavailable, state exactly which verification could not be executed.

### 3. Make the Smallest Complete Change

Implement the smallest coherent change that satisfies the requested behavior. A small change is not an incomplete change. It must still include all required production behavior, error handling, state transitions, protocol parsing, HAL exports, declarations, build integration, tests, documentation, and traceability references.

Do not broaden the task into unrelated cleanup, restructuring, generic framework creation, or speculative future support.
Do not create parallel implementations (`new`, `old`, `fixed`, `v2`) when the existing implementation should be corrected.

### 4. No Fake Completion

The following do not constitute a completed implementation:

- A stub returning success; a placeholder body; a TODO standing in for required behavior.
- Hardcoded sample values; simulated device responses in production code.
- A parser that accepts data without validating it.
- A function that compiles but is never registered or called.
- A HAL pin that is exported but never updated.
- A send function without its corresponding response handling.
- A test that cannot fail; a test that only checks compilation.
- A test changed to match incorrect implementation behavior.
- Commented-out failing code or tests; a workflow changed to ignore failures.
- Documentation claiming behavior that has not been implemented and verified.

### 6. No Real-Time Shortcuts

Code reachable from a LinuxCNC real-time function must remain deterministic and bounded. Do not introduce:
blocking device communication, synchronous PoKeys requests, blocking socket operations, mutex waits, dynamic allocation after RT execution begins, file access, sleeps, unbounded loops, uncontrolled retries, or expensive logging in a high-frequency path.

An asynchronous function is not RT-safe merely because its name ends in `Async`. Verify the complete call path.
Do not claim a timing guarantee from code inspection — timing compliance requires measurement in the appropriate runtime environment.

### 10. Explicit Evidence Is Required

Before claiming completion, report: files changed, requirement addressed, behavior implemented, architecture implications, tests added or changed, exact build and test commands executed, results of those commands, hardware or RT checks performed, checks not performed and why, remaining risks and out-of-scope work.

Use precise status terms:
- **Implemented** — production behavior exists.
- **Compiled** — only compilation succeeded.
- **Tested** — named tests were executed and passed.
- **Hardware-verified** — behavior observed with the relevant PoKeys device.
- **RT-validated** — tested in the LinuxCNC real-time environment.
- **Timing-validated** — timing was measured with an identified method and acceptance threshold.

Do not use "complete," "working," "fixed," "RT-safe," or "verified" without the corresponding evidence.

### 13. Completion Gate

A task is complete only when all applicable items are satisfied:

- [ ] The requested behavior and acceptance criteria are implemented.
- [ ] Architecture boundaries are preserved.
- [ ] Protocol behavior checked against an authoritative source.
- [ ] RT paths remain non-blocking, bounded, and allocation-free.
- [ ] Relevant tests or reproducible checks exist and pass.
- [ ] Required builds succeed.
- [ ] No required behavior is represented by a stub, TODO, simulation, or hardcoded result.
- [ ] Documentation reflects actual behavior.
- [ ] Issue and requirement traceability is present.
- [ ] Failed, skipped, unavailable, or hardware-dependent checks are reported honestly.

If an item is not applicable, it need not be performed. If applicable but cannot be completed, report the task as **partially verified** rather than complete.

---

## Workflow Defaults

- For behavior changes, establish a failing test or reproducible failing check first (TDD).
- Keep changes small and scoped to an issue or clearly stated task.
- Run the relevant build and verification commands before claiming completion.
- Report failed, skipped, and unavailable checks explicitly.
- Update existing documentation rather than creating redundant documents.
- All PRs link to an implementing issue using `Fixes #N` or `Implements #N`.

---

## Traceability

Every significant artifact references a GitHub issue using `#N` syntax:

```c
/* Implements: #123 (REQ-F-IO-001) */
```

Requirements trace to stakeholder issues. Tests trace to requirements. PRs trace to implementing issues. Architecture decisions have ADR issues.

---

## Known Gaps (Accepted, Not Enforced by CI)

- `git submodule update --remote` in build CI mutates the pinned SHA — accepted as an early-warning mechanism; do not "fix" by removing it without a replacement PR-based bump workflow.
