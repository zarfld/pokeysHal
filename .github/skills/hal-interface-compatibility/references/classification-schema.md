# HAL-Interface Compatibility Classification Schema

This file is the single authority for classification labels and output
vocabulary used by the HAL-interface compatibility workflow.

Use this schema with:
- `.github/skills/hal-interface-compatibility/SKILL.md`

For HIL status and outcomes, use:
- `.github/skills/hil-tdd/references/result-schema.md`

Do not redefine HIL status or outcome terminology here.

## 1. task_scope

Allowed labels:
- `HAL-COMPAT`
- `ASYNC-PARITY`
- `HAL-AND-ASYNC`
- `NON-HAL`

Interpretation:
- `HAL-COMPAT`: HAL-interface behavior is directly affected.
- `ASYNC-PARITY`: protocol/async behavior is affected without HAL contract change.
- `HAL-AND-ASYNC`: both streams are materially affected.
- `NON-HAL`: no relevant HAL or async compatibility impact.

Scope vs streams rule:
- `task_scope` classifies the requested behavior/change.
- `verification_streams` classify what evidence/testing is needed.
- `HAL-COMPAT` may still require `ASYNC-PARITY` verification.
- `HAL-AND-ASYNC` is used only when both HAL contract and async/protocol
  behavior are requested or changed.

## 2. canonical_relationship

Allowed labels:
- `compatible`
- `conflict`
- `not-applicable`
- `unknown`

Interpretation rules:
- classify canonical relationship independently from legacy relationship;
- do not infer compatibility by naming similarity alone.

## 3. legacy_relationship

Allowed labels:
- `compatible`
- `conflict`
- `not-applicable`
- `unknown`

Interpretation rules:
- classify legacy relationship independently from canonical relationship;
- preserve documented legacy behavior when measuring compatibility.

## 4. interface_role

Allowed labels:
- `normal`
- `current-only-extension`
- `required-absent`
- `external-integration-only`
- `unknown`

Interpretation rules:
- use `external-integration-only` for HAL/INI counterpart mapping context;
- do not treat external counterpart behavior as pokeysHal implementation scope.

## 5. implementation_state

Allowed labels:
- `implemented-matching`
- `implemented-mismatching`
- `implemented-untraced`
- `partially-implemented`
- `absent`
- `not-applicable`
- `unknown`

Interpretation rules:
- `implemented-untraced` means behavior appears present but propagation evidence is
  incomplete;
- `partially-implemented` means only part of required behavior is implemented;
- `unknown` requires explicit evidence follow-up.

## 6. decision_gate

Allowed labels:
- `implementable-preserve-contract`
- `implementable-recorded-correction`
- `implementable-compatibility-alias`
- `characterization-only`
- `decision-required`
- `evidence-required`
- `deferred`
- `out-of-scope`

Interpretation rules:
- choose `decision-required` when conflicting contracts require a policy choice;
- choose `evidence-required` when required source evidence is missing;
- choose `characterization-only` when current behavior can be documented/tested
  without contract-selection decisions.
- `out-of-scope` means either:
  - outside the HAL/async compatibility procedure; or
  - outside pokeysHal scope,
  as stated explicitly in `explicit_exclusions`.
- For `NON-HAL` tasks, use `out-of-scope` for compatibility-workflow routing and
  return the issue to the normal repository workflow.

## 7. verification_stream

Allowed labels:
- `HAL-ABI`
- `HAL-PROPAGATION`
- `ASYNC-PARITY`
- `HAL-INTEGRATION`
- `HIL`

Interpretation rules:
- plan and report each stream separately;
- do not merge HAL compatibility and async parity into one pass/fail claim.

`HAL-INTEGRATION` specifically covers component load/readiness, object
resolution, naming/prefix behavior, representative HAL operations, function
registration behavior where relevant, ownership compatibility, conditional
creation visibility, HAL/INI loading behavior, and external counterpart wiring
as configuration evidence.

## 8. evidence_confidence

Allowed labels:
- `source-confirmed`
- `normative-source-confirmed`
- `configuration-observed`
- `issue-supported`
- `inferred`
- `unknown`

Interpretation rules:
- `inferred` and `unknown` are insufficient for behavior-changing decisions;
- use higher-authority evidence precedence before concluding compatibility.

## 9. claim_evidence_level

Allowed non-HIL labels:
- `source-inspected`
- `statically-characterized`
- `module-tested`
- `HAL-integration-tested`

Definitions:
- `source-inspected`: source and declarations were reviewed.
- `statically-characterized`: contract tuple was derived without executing it.
- `module-tested`: deterministic module-level test passed.
- `HAL-integration-tested`: component was loaded and the relevant HAL contract
  was exercised in LinuxCNC HAL.

Do not place HIL statuses in this field.

## 10. hil_status

Optional field used only when hardware evidence exists.

Its value must be taken exactly from:
- `.github/skills/hil-tdd/references/result-schema.md`

Do not reproduce the HIL status table in this schema.

## Required Output Fields

## Work Item Schema

`work_items` is a required field for issue burn-down and module-test-design
outputs.

Each work item must contain:
- `subject`
- `affected_interface_ids`
- `verification_streams`
- `decision_gate`
- `evidence_required`
- `explicit_exclusions`

Decision-gate behavior:
- top-level `decision_gate` is the strictest aggregate issue gate;
- per-work-item gates govern what can proceed immediately;
- blocked work items must not make independent characterization/test work appear
  non-actionable.

## Contract Comparison Structure

For each object, record exactly one value for each:
- `canonical_relationship`
- `legacy_relationship`
- `interface_role`
- `implementation_state`

Do not use combined values like `compatible / conflict`.

Dimension-level details belong in:
- `contract_comparison.contract_dimensions`

Example dimensions:
- `existence`
- `name`
- `kind`
- `type`
- `direction_or_access`
- `cardinality`
- `scaling_semantics`
- `default_or_initialization`
- `propagation`

### Issue burn-down output

Required fields:
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

### Module-test-design output

Required fields:
- `module`
- `task_scope`
- `affected_interface_ids`
- `canonical_relationship`
- `legacy_relationship`
- `interface_role`
- `hal_abi_tests`
- `hal_propagation_tests`
- `async_parity_tests`
- `hal_integration_tests`
- `verification_streams`
- `work_items`
- `expected_claim_evidence_level_by_layer`
- `hil_applicability`
- `hil_status` (only when existing hardware evidence is available)
- `mocks_and_fixtures`
- `assertion_evidence_sources`
- `known_gaps`
- `decision_blockers`
- `evidence_confidence`

## Claim and Evidence Rules

- `claim_evidence_level` is required in issue burn-down outputs.
- `hil_status` is optional and must be omitted when hardware evidence is absent.
- `hil_status` must never be set to placeholders such as `not applicable` or `none`.
- when present, `hil_status` must use an exact value from the HIL result schema.

Evidence confidence must be recorded per material claim and must distinguish:
- evidence for observed current behavior;
- evidence for authoritative/target requirement.

Do not classify directly visible source expressions as `inferred` only because
their correctness is uncertain.

When expected behavior lacks supporting authority evidence:
- classify as `evidence-required`;
- add an evidence-gathering work item;
- do not present an invented oracle as expected behavior.

## Normalization Rules

- Keep labels exact and case-sensitive.
- Use one label per dimension unless explicitly modeled as multi-valued.
- Record unresolved dimensions as `unknown`, never as implied pass.
- Cite source paths and IDs for every non-unknown classification.
