# Decision: Separate Invariants, Procedures, and Rationale

## Context

The original `.github/copilot-instructions.md` contained approximately 857 always-loaded lines — abstract XP/DDD philosophy, nine lifecycle-phase summaries, generic JWT/TypeScript/REST examples, and 60-item Always/Never lists — mixed with the repository-specific identity and build commands that legitimately belong in root instructions.

This caused two problems: irrelevant design suggestions (JWT auth, SQL injection, web sessions) for a C/LinuxCNC driver, and agent instructions too large to process effectively.

## Decision

Separate the guidance into four layers:

1. **Short mandatory invariants in instructions** — what the agent must never forget for any matching-file edit (no blocking in RT, preserve architecture boundaries, no fake completion).
2. **Detailed execution procedures in skills** — loaded only when performing a specific recognizable task (async subsystem conversion, submodule bump, README rewrite).
3. **Rationale and extended explanation in documentation** — `docs/engineering-discipline.md`, not in agent context.
4. **Cross-tool operational contract in `AGENTS.md`** — technology-independent rules for Copilot, Claude Code, Codex, and similar tools.

## Consequences

- Automatic context for an ordinary C task reduced from ~1,416 lines (pre-PR #134) to ~272 lines (post corrective pass).
- Agent instructions load detailed procedures only when performing the relevant task (skill selection).
- Engineering discipline contract is now concrete and PoKeys-specific rather than abstract XP/DDD philosophy.
- Generic web/auth examples removed; C/HAL/RT/protocol-specific examples retained.

## Historical Source

This decision was analyzed in `docs/repo_review.md` (snapshot at commit `cf3902a6`) and implemented in PR #134, with corrective consolidation in the subsequent corrective-pass branch. The original 13-rule engineering contract proposal is preserved in `docs/engineering-discipline.md`.
