**Some should move to skills, but not the core safety and architecture rules.**

The dividing line is:

> **Instructions define constraints the agent must never forget. Skills define procedures the agent should load only when performing a particular task.**

## Recommended allocation for `pokeysHal`

| Current content                                       | Keep as instruction | Move to skill                   |
| ----------------------------------------------------- | ------------------- | ------------------------------- |
| No blocking calls in RT paths                         | Yes                 | No                              |
| No allocation after RT startup                        | Yes                 | No                              |
| Preserve async/infrastructure/subsystem boundaries    | Yes                 | No                              |
| HAL-visible fields use `hal_*` types                  | Yes                 | No                              |
| Protocol changes require specification verification   | Yes                 | Detailed verification procedure |
| Do not claim RT/timing validation without measurement | Yes                 | Measurement workflow            |
| Baseline before behavior changes                      | Yes, concise        | Detailed baseline procedure     |
| No fake completion/stubs                              | Yes                 | No                              |
| Full async subsystem conversion sequence              | No                  | Yes                             |
| Detailed request/response byte verification           | No                  | Yes                             |
| Detailed test-case generation                         | No                  | Yes                             |
| HAL pin-export implementation procedure               | No                  | Yes                             |
| Mailbox/debugging workflow                            | No                  | Yes                             |
| Traceability creation and validation workflow         | No                  | Yes                             |
| Submodule bump procedure                              | No                  | Yes                             |
| README creation procedure                             | Usually no          | Yes                             |
| Architecture diagrams and extended explanations       | No                  | Documentation                   |

# Specific file recommendations

## 1. `engineering-discipline.instructions.md`

Do **not** move the entire file to a skill.

Its critical behavioral rules must remain active automatically:

* inspect requirements and current implementation first;
* establish a baseline;
* make the smallest complete change;
* do not fake completion;
* preserve architecture;
* avoid RT and protocol shortcuts;
* report exact verification evidence.

However, 183 lines is more than necessary.

Reduce it to approximately **60–90 lines** containing only enforceable rules. Move detailed workflows, examples and checklists into skills.

A skill should not be responsible for loading “No fake completion,” because Copilot might not select that skill for an apparently small bug fix.

## 2. `c-realtime.instructions.md`

Most of its **constraints** belong in instructions:

* no blocking;
* no allocation;
* bounded execution;
* allowed logging;
* HAL types;
* architecture boundaries;
* named command and mask constants.

The following belong in skills:

* full RT component build sequence;
* async subsystem conversion steps;
* protocol-document inspection method;
* HAL export implementation pattern;
* manual hardware verification procedure.

The file should become a compact C/RT invariant sheet, not a developer manual.

## 3. `pokeyshal-architecture.instructions.md`

The architectural boundaries should remain instructions because they apply to every C/H modification.

Keep only a concise responsibility table:

```text
PoKeysLibAsync.c
  Shared mailbox, dispatch, timeout and retry infrastructure only.

PoKeysLib*Async.c
  Subsystem request creation, parsing, HAL export and task registration.

experimental/pokeys_async.c
  Integration shell only.

PoKeysLibHal.h
  HAL-visible data structures.

PoKeysLibAsync.h
  Shared async contracts, enums, masks and declarations.
```

Move these out:

* architecture diagrams;
* long explanations of each layer;
* complete review procedures;
* implementation examples;
* migration walkthroughs.

Those belong in documentation and the conversion skill.

## 4. `tests.instructions.md`

Split it.

### Keep as instruction

Approximately 30–50 lines:

* tests reference applicable requirement issues;
* behavior tests must be able to fail;
* bug fixes require regression coverage where feasible;
* protocol tests cover malformed and mismatched responses;
* test changes must not hide incorrect behavior;
* compilation alone is not behavioral verification;
* unavailable hardware and RT tests must be reported explicitly.

### Move to skills

Create skills such as:

```text
.github/skills/test-pokeyshal-change/SKILL.md
.github/skills/test-async-protocol-command/SKILL.md
.github/skills/validate-realtime-behavior/SKILL.md
```

These can contain detailed procedures, fixtures, test matrices and command examples.

The existing login, database, REST and SQL-injection examples should be removed rather than transferred.

## 5. `submodules.instructions.md`

Keep a very small instruction:

* do not edit submodule contents as though they belong to the parent repository;
* pin immutable commits;
* inspect compatibility before changing the pointer;
* do not silently track branches.

Move the update procedure into:

```text
.github/skills/update-submodule/SKILL.md
```

That skill can describe fetching, selecting a commit, testing the parent repository, documenting compatibility and creating the bump PR.

## 6. `readme.instructions.md`

The current comprehensive README manual is better suited to a skill.

A path-specific instruction may contain only:

* document implemented behavior only;
* update existing authoritative sections;
* avoid duplicated documentation;
* preserve working commands and links;
* use the README as an entry point, not as the complete design document.

Move the full README structure and rewriting workflow into:

```text
.github/skills/update-readme/SKILL.md
```

## Proposed target structure

```text
.github/
├── copilot-instructions.md
│   Repository identity, commands, critical invariants, routing
│
├── instructions/
│   ├── c-realtime.instructions.md
│   │   Compact C/RT + architecture invariants
│   ├── tests.instructions.md
│   │   Compact test integrity rules
│   ├── submodules.instructions.md
│   │   Compact submodule constraints
│   ├── readme.instructions.md
│   │   Compact documentation invariants
│   └── phase-*.instructions.md
│       Artifact-specific mandatory rules only
│
├── skills/
│   ├── convert-to-hal-rtapi/SKILL.md
│   ├── verify-pokeys-protocol/SKILL.md
│   ├── test-pokeyshal-change/SKILL.md
│   ├── validate-realtime-behavior/SKILL.md
│   ├── debug-async-mailbox/SKILL.md
│   ├── update-submodule/SKILL.md
│   ├── update-readme/SKILL.md
│   └── validate-traceability/SKILL.md
│
└── agents/
    ├── tdd-driver.agent.md
    ├── testing-specialist.agent.md
    └── architecture-strategist.agent.md
```

# Practical rule

Use this test for each paragraph:

### Keep it in instructions when:

> “Would damage be possible if the agent did not see this rule during an ordinary matching-file edit?”

Examples:

* no blocking in RT;
* do not put subsystem code into `PoKeysLibAsync.c`;
* validate response lengths;
* do not claim timing verification without measurement.

### Move it to a skill when:

> “Is this a sequence of steps for accomplishing one recognizable task?”

Examples:

* convert a synchronous subsystem;
* add an async command;
* design a protocol test matrix;
* measure RT latency;
* update a submodule;
* build a complete README.

So the correct optimization is **not “instructions versus skills.”** It is:

* **short, mandatory invariants in instructions**;
* **detailed execution contracts in skills**;
* **rationale and diagrams in documentation**;
* **specialized role and tool configuration in agents**.
