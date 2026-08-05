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

## 2. contract_relationship

Allowed labels:
- `canonical-compatible`
- `legacy-compatible`
- `canonical-and-legacy-compatible`
- `canonical-conflict`
- `legacy-conflict`
- `current-only-extension`
- `required-absent`
- `external-integration-only`
- `unknown`

Interpretation rules:
- classify against explicit evidence, not naming similarity;
- use `external-integration-only` for HAL/INI counterpart mapping context;
- do not treat external counterpart behavior as pokeysHal implementation scope.

## 3. implementation_state

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

## 4. decision_gate

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

## 5. verification_stream

Allowed labels:
- `HAL-ABI`
- `HAL-PROPAGATION`
- `ASYNC-PARITY`
- `HAL-INTEGRATION`
- `HIL`

Interpretation rules:
- plan and report each stream separately;
- do not merge HAL compatibility and async parity into one pass/fail claim.

## 6. evidence_confidence

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

## Required Output Fields

### Issue burn-down output

Required fields:
- `issue_or_request`
- `task_scope`
- `affected_interface_ids`
- `contract_comparison`
- `contract_relationship`
- `implementation_state`
- `linked_conflicts`
- `linked_open_decisions`
- `decision_gate`
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
- `hal_abi_tests`
- `hal_propagation_tests`
- `async_parity_tests`
- `hal_integration_tests`
- `hil_applicability`
- `mocks_and_fixtures`
- `assertion_evidence_sources`
- `known_gaps`
- `decision_blockers`
- `evidence_confidence`

## Normalization Rules

- Keep labels exact and case-sensitive.
- Use one label per dimension unless explicitly modeled as multi-valued.
- Record unresolved dimensions as `unknown`, never as implied pass.
- Cite source paths and IDs for every non-unknown classification.
