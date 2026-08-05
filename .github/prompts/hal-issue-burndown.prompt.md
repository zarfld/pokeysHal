---
agent: agent
description: >
  Read-only HAL compatibility issue burn-down workflow. Consumes the HAL-interface
  compatibility skill outputs and reports classification-schema fields without
  changing production code or issue metadata.
argument-hint: "Issue #N or issue URL"
---

# HAL Issue Burn-Down

Run a read-only issue analysis for HAL/async compatibility impact.

## Required inputs

1. Read the full issue body.
2. Read relevant issue comments needed for compatibility context.
3. Invoke:
   - `.github/skills/hal-interface-compatibility/SKILL.md`
   - `.github/skills/hal-interface-compatibility/references/classification-schema.md`

Use the skill output contract exactly. Do not restate the full procedure.

## Scope constraints

This prompt must remain read-only.

Do not:
- edit production code;
- create tests;
- mutate issue state, labels, comments, or metadata;
- invent unresolved compatibility decisions;
- copy Phase 0 catalogue rows;
- inspect `pokeys_homecomp` internals as pokeysHal scope.

Treat external counterpart references (including `joint.N.*` and
`pokeys_homecomp`) as external integration evidence only.

## Required output fields

Produce all required issue burn-down fields defined by the schema, followed by
the required human-readable outcome line:
- `issue_or_request`
- `task_scope`
- `affected_interface_ids`
- `contract_comparison`
- `canonical_relationship`
- `legacy_relationship`
- `interface_role`
- `implementation_state`
- `linked_conflicts`
- `linked_open_decisions`
- `decision_gate`
- `work_items`
- `claim_evidence_level`
- `hil_status` (when hardware evidence exists)
- `required_issue_decomposition`
- `required_tests_by_stream`
- `async_impact`
- `hil_applicability`
- `traceability_changes`
- `explicit_exclusions`
- `evidence_confidence`

Work-item requirement:
- `work_items` is mandatory.
- each work item must include:
  - `subject`
  - `affected_interface_ids`
  - `verification_streams`
  - `decision_gate`
  - `evidence_required`
  - `explicit_exclusions`

Use top-level `decision_gate` as strictest aggregate status, while preserving
actionable work items when another item is blocked.

## Required analysis behavior

- Distinguish `HAL-COMPAT`, `ASYNC-PARITY`, `HAL-AND-ASYNC`, and `NON-HAL` scope.
- Identify affected Phase 0 interface IDs.
- Identify conflicts and open decisions that govern behavior-changing work.
- Provide a decision gate result with no guessed resolution.
- Require one `claim_evidence_level` per material compatibility claim.
- Omit `hil_status` when hardware evidence is absent.
- When present, `hil_status` must use an exact value from
  `.github/skills/hil-tdd/references/result-schema.md`.
- Propose decomposition and verification work by stream:
  HAL-ABI, HAL-PROPAGATION, HAL-INTEGRATION, ASYNC-PARITY, HIL.
- Identify HIL applicability using the existing HIL skill/schema references.

Do not invent test oracles. Every expected behavior/value must cite authority.
If authority evidence is missing, mark `evidence-required` and add an
evidence-gathering work item.

## Outcome mapping

For `task_scope` `NON-HAL`, use:
- `decision_gate`: `out-of-scope`
- human outcome: `NOT APPLICABLE TO HAL/ASYNC COMPATIBILITY`

This means outside the HAL/async compatibility workflow. The issue may still
proceed through the normal repository workflow.

Keep `OUT OF SCOPE` for items genuinely outside pokeysHal scope, distinguishing
the two cases through `explicit_exclusions`.

Open product decisions and Phase 0 catalogue gaps do not block this burn-down
workflow itself; they become decision/evidence/erratum work items unless there
is an infrastructure contradiction.

End with one line containing one of:
- `READY FOR TDD` for
  `implementable-preserve-contract`,
  `implementable-recorded-correction`,
  `implementable-compatibility-alias`
- `READY FOR CHARACTERIZATION ONLY` for `characterization-only`
- `BLOCKED BY DECISION` for `decision-required`
- `BLOCKED BY EVIDENCE` for `evidence-required`
- `DEFERRED` for `deferred`
- `OUT OF SCOPE` for `out-of-scope`
- `NOT APPLICABLE TO HAL/ASYNC COMPATIBILITY` for `task_scope` `NON-HAL` with
  `decision_gate` `out-of-scope`

Also print the exact `decision_gate` value alongside the human-readable outcome.
