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

Produce exactly the issue burn-down fields defined by the classification schema:
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
- `claim_evidence_level`
- `hil_status` (when hardware evidence exists)
- `required_issue_decomposition`
- `required_tests_by_stream`
- `async_impact`
- `hil_applicability`
- `traceability_changes`
- `explicit_exclusions`
- `evidence_confidence`

## Required analysis behavior

- Distinguish `HAL-COMPAT`, `ASYNC-PARITY`, `HAL-AND-ASYNC`, and `NON-HAL` scope.
- Identify affected Phase 0 interface IDs.
- Identify conflicts and open decisions that govern behavior-changing work.
- Provide a decision gate result with no guessed resolution.
- Propose decomposition and verification work by stream:
  HAL-ABI, HAL-PROPAGATION, HAL-INTEGRATION, ASYNC-PARITY, HIL.
- Identify HIL applicability using the existing HIL skill/schema references.

## Outcome mapping

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

Also print the exact `decision_gate` value alongside the human-readable outcome.
