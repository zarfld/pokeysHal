---
name: hal-interface-compatibility
description: >
  Reusable procedure for HAL-interface impact triage, contract comparison,
  decision gating, and compatibility-oriented test decomposition using the
  Phase 0 HAL-interface evidence baseline.
---

# HAL-Interface Compatibility Skill

## Purpose

Use this skill to perform a consistent HAL-interface compatibility procedure for:
- issue burn-down planning;
- module-test design;
- TDD preparation for HAL-facing changes;
- HAL-facing review and verification planning.

This skill defines process and vocabulary. It does not modify Phase 0 artifacts.

## Phase 0 Evidence Inputs

Direct operational inputs:
- `docs/hal-interface/phase-0/requirement-catalogue.yaml`
- `docs/hal-interface/phase-0/source-register.yaml`
- `docs/hal-interface/phase-0/legacy-pev2-parity.yaml`
- `docs/hal-interface/phase-0/canonical-vs-legacy-matrix.md`
- `docs/hal-interface/phase-0/conflict-register.md`
- `docs/hal-interface/phase-0/open-decisions.md`
- `docs/hal-interface/phase-0/traceability.md`
- `docs/hal-interface/phase-0/integration-links.yaml`

Supporting context:
- `docs/hal-interface/phase-0/issue-inventory.md`
- `docs/hal-interface/phase-0/lifecycle-ownership-matrix.md`
- `docs/hal-interface/phase-0/README.md`
- `docs/hal-interface/phase-0/phase-0-completion-report.md`

Maintenance tooling only (not compatibility authority):
- `docs/hal-interface/phase-0/tools/scan_current_exports.py`
- `docs/hal-interface/phase-0/tools/validate_phase0_closure.py`
- `docs/hal-interface/phase-0/tools/extract_legacy_pev2_exports.py`

## Authority Precedence

When sources disagree, apply this precedence order:
1. current official LinuxCNC authority;
2. exact pinned hal-canon source;
3. explicit repository requirements;
4. legacy component source;
5. current pokeysHal source;
6. examples and issue evidence.

The requirement catalogue indexes evidence references. It does not override a
higher-authority source.

## Scope Boundary

In scope:
- pokeysHal library and RT `pokeys.comp` replacement behavior;
- PoKeys-side HAL pins, parameters, functions, naming, typing, cardinality,
  creation conditions, units, scaling/default semantics, lifecycle, and
  PoKeys-side propagation;
- compatibility with legacy `pokeys.comp`.

External evidence only:
- HAL/INI net examples;
- `joint.N.*` counterpart objects;
- `pokeys_homecomp` endpoint observations.

Out of scope:
- `pokeys_homecomp` implementation/lifecycle internals;
- homecomp local state and internal read/write behavior;
- `joint.N.*` objects as pokeysHal exports.

Keep HAL compatibility and asynchronous protocol parity as separate verification
streams.

## Classification Vocabulary

Use:
- `.github/skills/hal-interface-compatibility/references/classification-schema.md`

This reference is the single authority for skill classification labels and
required output field names.

## Workflow

### A. Establish task context

1. Identify the issue, requirement, or requested change.
2. Identify affected modules and file-responsibility boundaries.
3. Assign preliminary `task_scope` from the classification schema.
4. State explicit inclusions and exclusions for this task.

Minimum context record:
- task identifier;
- requested behavior;
- changed surface area candidates;
- expected verification layers.

### B. Detect HAL-interface impact

Evaluate potential impact on each dimension below:
- existence;
- exact name and prefix;
- object kind;
- HAL type;
- direction/access;
- cardinality;
- creation conditions;
- units;
- scaling;
- initial/default value claims;
- enums and bitmaps;
- lifecycle behavior;
- PoKeys-side propagation;
- external HAL/INI compatibility expectations.

If no HAL-interface impact is found, classify as `NON-HAL` and stop this skill.

### C. Resolve Phase 0 evidence

1. Identify affected interface IDs in
   `docs/hal-interface/phase-0/requirement-catalogue.yaml`.
2. Retrieve legacy PEv2 parity rows from
   `docs/hal-interface/phase-0/legacy-pev2-parity.yaml` where applicable.
3. Resolve every cited source ID through
   `docs/hal-interface/phase-0/source-register.yaml`.
4. Inspect
   `docs/hal-interface/phase-0/canonical-vs-legacy-matrix.md` for relationship context.
5. Inspect linked conflicts in
   `docs/hal-interface/phase-0/conflict-register.md`.
6. Inspect linked decisions in
   `docs/hal-interface/phase-0/open-decisions.md`.
7. Inspect
   `docs/hal-interface/phase-0/integration-links.yaml` only for external mapping
   context; do not convert external counterparts into implementation scope.
8. Record missing evidence as `evidence-required` instead of guessing.

### D. Build a contract comparison

For each affected object, record separate fields:
- authoritative contract;
- legacy contract;
- current observed contract;
- decided target contract (when one exists);
- evidence sources;
- linked conflicts;
- linked open decisions;
- evidence confidence.

Do not collapse these into one ambiguous contract statement.

### E. Apply the decision gate

Allowed `decision_gate` outcomes:
- `implementable-preserve-contract`
- `implementable-recorded-correction`
- `implementable-compatibility-alias`
- `characterization-only`
- `decision-required`
- `evidence-required`
- `deferred`
- `out-of-scope`

Rules:
- A conflict blocks only changes that require choosing between conflicting
  contracts.
- If no recorded decision selects between conflicting contracts, choose
  `decision-required`.
- Never invent a decision to proceed.

### F. Separate verification streams

Plan independently per stream.

HAL-ABI stream:
- existence;
- exact name;
- kind;
- type;
- direction/access;
- cardinality;
- creation condition;
- default/initial state where relevant;
- representative `net` or `setp` behavior.

HAL-PROPAGATION stream:
- HAL object -> library/device field -> device operation; or
- device/parser result -> library/device field -> HAL object.

For each path, classify as complete, partial, missing, or untraced.

ASYNC-PARITY stream:
- synchronous PoKeysLib function or protocol authority;
- command/subcommand;
- payload mapping;
- response offsets;
- parser mapping;
- transaction identifier/matching;
- callback path;
- queue/scheduler behavior;
- timeout/retry/stale-response behavior.

HIL stream:
- route through `.github/skills/hil-tdd/SKILL.md`;
- use `.github/skills/hil-tdd/references/result-schema.md`;
- do not redefine HIL status or outcome terms in this skill.

### G. Produce issue-burn-down output

Required output fields:
- issue and scope;
- affected interface IDs;
- contract comparison per object;
- conflicts and open decisions;
- decision-gate result;
- required issue decomposition;
- required tests by verification stream;
- async impact statement;
- HIL applicability statement;
- traceability changes;
- explicit exclusions.

### H. Produce module-test-design output

Required output fields:
- module;
- affected interface IDs;
- HAL-ABI tests;
- propagation tests;
- async-parity tests;
- HIL applicability;
- mocks and fixtures;
- evidence source for each assertion;
- known gaps and decision blockers.

### I. Completion and traceability

At completion, include repository-traceable references for:
- implementation (`Implements #N` style traceability);
- verification (`Verifies #N` style traceability).

Compatibility statements must include verification layer:
- `source-inspected`;
- `statically characterized`;
- `module-tested`;
- `HAL-integration-tested`;
- `HIL-observed` or stronger status from
  `.github/skills/hil-tdd/references/result-schema.md`.

## Guardrails

- Do not copy Phase 0 catalogue rows into this skill.
- Do not rewrite conflict or decision content here.
- Do not claim compatibility from inferred evidence alone.
- `inferred` and `unknown` confidence are insufficient for behavior-changing
  implementation decisions.

## Suggested Procedure Skeleton

Use this compact sequence when executing the skill:
1. classify task scope;
2. detect HAL impact;
3. map interface IDs;
4. resolve authorities and conflicts;
5. build per-object contract comparison;
6. apply decision gate;
7. split verification streams;
8. produce issue-burn-down or module-test-design output;
9. attach traceability and explicit exclusions.
