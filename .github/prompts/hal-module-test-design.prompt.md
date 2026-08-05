---
agent: TestingSpecialist
description: >
  Design module-level HAL/async compatibility test work using the HAL-interface
  compatibility skill outputs and classification schema, without changing
  production behavior.
argument-hint: "module, issue number, or interface IDs"
---

# HAL Module Test Design

Design test work for HAL/async compatibility using existing evidence and
classification outputs.

## Required workflow

1. Invoke:
   - `.github/skills/hal-interface-compatibility/SKILL.md`
   - `.github/skills/hal-interface-compatibility/references/classification-schema.md`
2. Consume the module-test-design fields from the schema.
3. Keep HAL and async streams separated throughout the test design.

Scope vs streams rule:
- `task_scope` describes the requested behavior/change.
- `verification_streams` describe required evidence/test coverage.
- `HAL-COMPAT` may still require ASYNC-PARITY verification.

## Scope constraints

This prompt designs verification work only.

Do not:
- modify production code;
- invent acceptance criteria;
- invent compatibility decisions;
- copy Phase 0 catalogue rows into prompt output.

You may propose test files and TEST issues when needed.

Treat `pokeys_homecomp` and `joint.N.*` as external integration context only.
Do not inspect or require homecomp internals as pokeysHal implementation scope.

## Required output fields

Use the module-test-design fields defined in the classification schema:
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

Work-item requirement:
- `work_items` is mandatory.
- each work item must include:
  - `subject`
  - `affected_interface_ids`
  - `verification_streams`
  - `decision_gate`
  - `evidence_required`
  - `explicit_exclusions`

## Test design requirements

State applicability and rationale for each stream:
- HAL-ABI
- HAL-PROPAGATION
- HAL-INTEGRATION
- ASYNC-PARITY
- HIL

For each applicable test group, define:
- deterministic oracle;
- preconditions;
- expected behavior source authority;
- required mocks/fixtures (if any);
- expected `claim_evidence_level` at that layer.

Do not invent test oracles. If an expected behavior lacks authority evidence,
mark it `evidence-required` and add an evidence-gathering work item.

`hil_status` handling:
- omit `hil_status` when no hardware evidence exists;
- do not use placeholders such as `not applicable` or `none`;
- when present, use an exact value from
  `.github/skills/hil-tdd/references/result-schema.md`.

Hardware-dependent work must be routed to:
- `.github/skills/hil-tdd/SKILL.md`
- `.github/skills/hil-tdd/references/result-schema.md`

Do not redefine HIL status or outcome tables in this prompt.
