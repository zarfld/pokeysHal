---
name: hil-tdd-guidance-synthesize
description: >
  Synthesize the PoKeys57E HIL/TDD thought documents into a conflict-resolved,
  evidence-classified decision basis. Do not edit repository files or implement HIL.
agent: 'plan'
tools: ['read', 'search']
---

# Objective

Produce a decision-quality synthesis of the existing PoKeys57E HIL/TDD material.

This is an analysis task only.

Do not:

- create HIL tests;
- create HAL or INI files;
- create workflows;
- modify instructions, agents, prompts, or skills;
- operate hardware;
- silently select one proposal because it is longer or repeated.

## Source documents

Read each source completely:

- [HIL guidance suggestion](../../docs/HIL_Setup_4_TDD/HIL_TDD_guidance_suggestion1.md)
- [HIL module-test thoughts](../../docs/HIL_Setup_4_TDD/HIL_moduletests_thoughts.md)
- [PoKeys57E pin modes](../../docs/HIL_Setup_4_TDD/PoKeys57E%20pin%20modes.md)
- [TDD Guidance 1](../../docs/HIL_Setup_4_TDD/TDD%20Guidance%20for%20PokeysHal1.md)
- [TDD Guidance 2](../../docs/HIL_Setup_4_TDD/TDD%20Guidance%20for%20PokeysHal2.md)
- [TDD/HIL guidance thoughts](../../docs/HIL_Setup_4_TDD/TDD_HILguidance_thoughts.md)

Also inspect the current operational context:

- [Root Copilot instructions](../copilot-instructions.md)
- [Cross-tool agent rules](../../AGENTS.md)
- [Test instructions](../instructions/tests.instructions.md)
- [C/RT instructions](../instructions/c-architecture-realtime.instructions.md)
- [TDD driver agent](../agents/tdd-driver.agent.md)
- [Testing specialist agent](../agents/testing-specialist.agent.md)
- [Current TDD prompt](tdd-compile.prompt.md)
- [Repository audit prompt](repository-audit.prompt.md)

## Interpretation policy

Treat the sources according to this hierarchy.

### Normative product behavior

Requires support from:

1. official PoKeys/PoLabs documentation;
2. current protocol definitions;
3. current repository code;
4. verified upstream `pokeyslib` behavior.

### Physical fixture facts

Require:

1. physically inspected wiring;
2. a versioned fixture definition;
3. continuity or controlled loopback verification;
4. recorded device identity and configuration.

Legacy HAL/INI comments are provenance, not proof of the present fixture.

### Existing repository behavior

Current source, tests, HAL files, INI files, and exported pin names describe the
current implementation. They do not automatically define the desired behavior.

### Thought and guidance documents

Treat these as candidate proposals. They are not requirements and are not
independent evidence merely because the same statement appears more than once.

### Previous AI conclusions

Treat conversational phrasing such as "Yes", "Agreed", "should", and
"recommended" as analysis, not as an accepted project decision.

## Required analysis

### 1. Source inventory

For every source report:

- path;
- purpose;
- type: evidence, provenance, proposal, decision, or mixed;
- duplicate or near-duplicate status;
- factual claims;
- recommendations;
- unresolved assumptions.

Detect exact content duplicates and count them once.

### 2. Claim matrix

Create a table with:

| Claim ID | Claim | Sources | Classification | Evidence | Conflict | Required resolution |
|---|---|---|---|---|---|---|

Allowed classifications:

- `verified`
- `supported but not verified`
- `proposal`
- `conflicting`
- `obsolete`
- `unsafe to assume`

Do not convert a repeated proposal into a verified claim.

### 3. Explicit conflict analysis

At minimum resolve or mark unresolved:

1. Whether HIL is:
   - the source of selected unit-test oracles;
   - an outer regression/acceptance layer;
   - or both under clearly different terms.

2. Whether `wcomp` position-derived switch simulation belongs in:
   - primitive HIL;
   - machine integration;
   - or a later composite HIL layer.

3. Whether the full `DM542_XXYZ_mill` configuration may be used by basic HIL.

4. Pin 33 → Pin 52 versus the INI emergency-input Pin 54 setting.

5. Physical pin numbering versus zero-based HAL channel indexing.

6. PWM terminology:
   - PWM output;
   - analog output;
   - filtered PWM-to-ADC observation.

7. Status vocabulary:
   - HIL-observed;
   - HIL-tested;
   - HIL-verified;
   - integration-tested;
   - RT-validated;
   - timing-validated.

8. Repository placement:
   - `hil-tests.instructions.md` versus `hil-tdd.instructions.md`;
   - instruction versus skill;
   - fixture definition location;
   - primitive HIL versus machine-integration directories.

9. Whether a dedicated HIL agent is currently justified.

### 4. Decision recommendations

For each conflict, provide:

- recommended decision;
- rationale;
- evidence supporting it;
- consequences;
- remaining verification needed.

Clearly distinguish:

- decisions that can be made from repository architecture;
- decisions requiring manual hardware confirmation;
- decisions requiring official documentation;
- decisions that should remain deferred.

### 5. Proposed canonical information architecture

Recommend which final documents should exist and what each owns.

The target must avoid keeping six overlapping guidance documents active.

Include proposed dispositions:

- retain as authoritative;
- merge;
- convert to historical source note;
- archive;
- delete as exact duplicate.

### 6. Output

Return:

1. executive conclusion;
2. source inventory;
3. claim/conflict matrix;
4. recommended decisions;
5. unresolved hardware questions;
6. canonical-document proposal;
7. inputs required for the subsequent infrastructure audit.

Do not produce implementation files or an implementation PR plan yet.