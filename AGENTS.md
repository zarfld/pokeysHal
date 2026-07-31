# PoKeysHal — Agent Standing Rules

Applies to: GitHub Copilot, Claude Code, Codex, and any other AI agent operating on this repository.

Full 13-rule contract: `docs/engineering-discipline.md`. C/RT architecture: `.github/instructions/c-architecture-realtime.instructions.md`. Detailed conversion procedures: `.github/skills/convert-to-hal-rtapi/SKILL.md`.

---

## Repository Identity

- **PoKeysLibHal** is a C99 LinuxCNC HAL driver for PoKeys USB/Ethernet devices.
- Supports userspace and real-time (RTAI / PREEMPT_RT) operation.
- Communication is asynchronous and non-blocking; the mailbox/dispatch layer is in `PoKeysLibAsync.c`.
- `pokeyslib/` is the upstream synchronous reference; `hal-canon/` provides canonical HAL channel interfaces.
- Primary build: `sudo make -f Makefile.noqmake install`

---

## Critical Invariants

- **No blocking calls, no dynamic allocation, no unbounded loops** in any code path reachable from a LinuxCNC real-time function.
- **Preserve async infrastructure / subsystem / integration-shell boundaries** — see `c-architecture-realtime.instructions.md`.
- **Use HAL-compatible types** (`hal_u32_t`, `hal_s32_t`, `hal_bit_t`, `hal_float_t`) for all HAL-visible struct members.
- **Do not claim timing compliance without measurement.** Compiles ≠ RT-safe. Looks non-blocking ≠ timing-validated.
- **Do not change protocol behavior without checking the PoKeys protocol specification** for command codes, byte offsets, masks, and response semantics.

---

## Engineering Discipline

1. **Inspect before modifying** — establish the applicable contract from issue, architecture rules, current implementation, protocol spec, and existing patterns before changing anything.
2. **Establish a baseline** — run the narrowest relevant build/test first; record pass/fail; separate pre-existing failures from new ones.
3. **Make the smallest complete change** — complete means all required behavior, tests, docs, and traceability are included.
4. **No fake completion** — stubs, TODOs for required behavior, untested exports, tests that cannot fail, and undocumented assumptions are not done.
6. **No RT shortcuts** — verify the complete call path; do not trust the name `Async`.
10. **Report exact evidence** — use precise status terms: Implemented / Compiled / Tested / HIL-observed / HIL-test-executed / HIL-verified / RT-validated / Timing-validated. For HIL status definitions and outcomes, use `.github/skills/hil-tdd/references/result-schema.md`.
13. **Completion gate** — architecture boundaries preserved; protocol checked; RT paths bounded; tests pass; no stubs; docs accurate; traceability present; unavailable checks disclosed.

---

## Workflow Defaults

- Establish a failing test or reproducible check before changing behavior.
- Keep changes scoped to an issue or clearly stated task.
- Run relevant build and verification commands before claiming completion.
- Report failed, skipped, and unavailable checks explicitly.
- Update existing documentation rather than creating redundant documents.
- All PRs link to an implementing issue using `Fixes #N` or `Implements #N`.

---

## Traceability

```c
/* Implements: #123 (REQ-F-IO-001) */
```

Requirements trace to stakeholder issues. Tests trace to requirements. PRs trace to implementing issues.

---

## Known Gaps (Accepted, Not Enforced by CI)

- `git submodule update --remote` in build CI mutates the pinned SHA — accepted as an early-warning mechanism; do not remove without a replacement PR-based bump workflow.

---
