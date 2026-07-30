---
name: tdd-hil-infrastructure-audit
description: >
  Audit and redesign the PoKeysHal TDD/Copilot infrastructure using the approved
  HIL/TDD synthesis. Produce a phased corrective plan; do not implement HIL tests.
agent: 'plan'
tools: ['read', 'search']
---

# Objective

Design the smallest coherent optimization of the repository's TDD and Copilot
infrastructure that must occur before HIL implementation starts.

This is a planning task only. Do not edit files.

## Authoritative input

Read:

- [Approved HIL/TDD synthesis](../../docs/HIL_Setup_4_TDD/HIL_TDD_synthesis.md)
- [Root Copilot instructions](../copilot-instructions.md)
- [AGENTS.md](../../AGENTS.md)
- [Test instructions](../instructions/tests.instructions.md)
- [Engineering discipline](../instructions/engineering-discipline.instructions.md)
- [C/RT architecture](../instructions/c-architecture-realtime.instructions.md)
- [TDD driver agent](../agents/tdd-driver.agent.md)
- [Testing specialist agent](../agents/testing-specialist.agent.md)
- [TDD compile prompt](tdd-compile.prompt.md)
- [Repository audit prompt](repository-audit.prompt.md)
- all existing `.github/skills/*/SKILL.md` files relevant to testing.

Do not re-interpret the six raw thought documents unless the synthesis explicitly
identifies an unresolved question requiring source inspection.

## Audit questions

### Current TDD agents

Determine:

- whether `TDDDriver` and `TestingSpecialist` are sufficiently PoKeys/C/HAL-specific;
- which TypeScript, JWT, REST, database, browser, Java, Jest, Cypress, and generic
  coverage examples must be removed;
- whether both agents have distinct responsibilities;
- whether one agent should be removed, merged, or rewritten;
- the minimum routing text needed for a future `hil-tdd` skill.

### Current prompts

Determine:

- which prompt files use obsolete or ineffective metadata;
- which prompts contain generic web-project assumptions;
- which workflows are duplicated between prompts, agents, instructions, and skills;
- whether `tdd-compile.prompt.md` should be rewritten, split, or replaced;
- whether `repository-audit.prompt.md` should remain available for this repository.

### Instructions versus skills

Apply this rule:

- instructions contain short mandatory invariants;
- skills contain detailed repeatable procedures;
- prompts initiate a specific workflow;
- agents define role and available tools;
- documentation contains rationale, evidence, fixture facts, and decisions.

Prevent the HIL procedure from being copied into all five layers.

### HIL readiness

Determine the minimum infrastructure needed before hardware work:

- canonical synthesis/decision document;
- versioned fixture schema design;
- short future HIL instruction;
- future HIL-TDD skill;
- status/evidence vocabulary;
- test-layer taxonomy;
- agent routing;
- validation mechanism.

Do not create the fixture, HIL skill, tests, HAL files, or workflow in this plan's
first corrective PR unless the approved synthesis explicitly requires them.

## Required output

### 1. Findings

List findings by severity:

- P0: invalid or misleading active customization;
- P1: excessive/generic context or contradictory ownership;
- P2: missing infrastructure needed before HIL;
- deferred: actual HIL implementation.

### 2. Target architecture

Provide one target tree showing:

- instructions;
- prompts;
- skills;
- agents;
- canonical documentation;
- future HIL fixture and test locations.

For each file state its single responsibility.

### 3. File-by-file disposition

Use:

| Current file | Keep | Rewrite | Split | Archive | Delete | Target size | Reason |
|---|---:|---:|---:|---:|---:|---:|---|

### 4. Context budgets

Set explicit preferred and maximum line budgets for:

- `TDDDriver`;
- `TestingSpecialist`;
- TDD prompt;
- HIL routing additions;
- future HIL instruction;
- future HIL skill.

Measure duplication, not only raw lines.

### 5. Phased PR plan

Prepare small reviewable PRs.

Recommended separation:

1. current TDD-agent and prompt cleanup;
2. canonical HIL/TDD decision documentation and source archival;
3. HIL contract/schema infrastructure;
4. first primitive HIL tests.

Each PR must include:

- exact scope;
- files changed;
- acceptance criteria;
- verification commands;
- explicit out-of-scope items.

### 6. Decision gates

State what must be manually approved before moving to the next phase.

### 7. Prohibitions

The plan must not:

- operate hardware;
- assume Pin 52/54 resolution;
- implement emergency or motion HIL;
- create a broad one-PR rewrite;
- duplicate detailed workflows across agents, prompts, instructions, and skills;
- claim HIL readiness from documentation alone.