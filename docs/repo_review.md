> **Historical snapshot.** This review describes commit `cf3902a6a2f67aa822faa429a2dcaad1e67e09f8`, not the current repository state. The P0 and P1 items identified here were addressed in PR #134 and the subsequent corrective-pass branch.

# Review scope and conclusion

I reviewed the repository at commit `cf3902a6a2f67aa822faa429a2dcaad1e67e09f8`.

The repository contains substantial engineering knowledge, but the **context architecture is currently inverted**:

* A large amount of procedural and philosophical material is placed in always-loaded or broadly matched instructions.
* Several items intended as prompts, skills, or agents are not located or named according to current Copilot discovery conventions.
* Important rules are duplicated across root instructions, phase instructions, agents, prompts, and skills.
* Many rules described as mandatory are only textual requests to the model and are not enforced by CI.
* Several examples and workflows are generic TypeScript/Python web-application material rather than PoKeys/LinuxCNC/C-specific guidance.

The highest-value changes are:

1. Repair discovery paths and filenames.
2. Reduce `.github/copilot-instructions.md` from 621 lines to approximately 80–120 lines.
3. Stop injecting the 236-line submodule manual into nearly every file-based interaction.
4. Turn detailed procedures into valid skills.
5. Turn roles into valid custom agents.
6. Back critical rules with scripts and CI instead of relying on prompt obedience.

---

# 1. What is actually loaded today

GitHub currently distinguishes:

* `.github/copilot-instructions.md`: repository-wide, always applied.
* `.github/instructions/*.instructions.md`: applied when `applyTo` matches the files being worked on.
* `.github/prompts/*.prompt.md`: explicitly invoked prompt files.
* `.github/skills/<name>/SKILL.md`: detailed procedures loaded when Copilot considers them relevant.
* `.github/agents/*.agent.md`: custom agent profiles.
* `AGENTS.md`: standing agent instructions, especially useful across tools. ([GitHub Docs][1])

## Current effective loading

| Repository artifact                                           | Current status                                              | Effective scope                                              |
| ------------------------------------------------------------- | ----------------------------------------------------------- | ------------------------------------------------------------ |
| `.github/copilot-instructions.md`                             | Valid                                                       | Always loaded                                                |
| `.github/instructions/submodules.instructions.md`             | Valid filename; `applyTo: "**"`                             | Effectively loaded for every file-based task                 |
| `.github/instructions/pokeyshal-architecture.instructions.md` | Valid filename; frontmatter uses an undocumented array form | Intended for all C/H files                                   |
| `.github/instructions/tests.instructions.md`                  | Valid filename; array-form globs                            | Intended for tests, but primarily matches JS/TS/Python tests |
| `phase-01` through `phase-09` instructions                    | Valid and path-scoped                                       | Loaded only for files in corresponding phase directories     |
| `.github/instructions/copilot-instructions.md`                | Wrong filename                                              | Not a recognized path-specific instruction                   |
| `.github/instructions/Readme_instructions.md`                 | Wrong filename                                              | Not a recognized path-specific instruction                   |
| `.github/ptompts/*.prompt.md`                                 | Directory typo                                              | Not discovered as workspace prompt files                     |
| `.github/skills/ConvertToHalRtapi/skill.md`                   | Wrong filename, missing required frontmatter                | Not a valid native skill                                     |
| `.github/skills/QtPyVCP/qtpyvcp-ui-generator.md`              | Wrong filename, missing required frontmatter                | Not a valid native skill                                     |
| `.github/agents/*.md`                                         | Non-standard filename for GitHub cloud-agent profiles       | Non-portable and not reliably discovered                     |
| Root `AGENTS.md`                                              | Referenced repeatedly but absent                            | No cross-tool root agent instructions                        |

GitHub documents `NAME.instructions.md` as the required filename and shows `applyTo` as a comma-separated glob string. Using YAML arrays may work on some clients, but it is not the documented portable form. ([GitHub Docs][1])

## Current context size

The two effectively global files contain:

* Root instructions: **621 lines**
* Submodule instructions: **236 lines**

That means approximately **857 lines of instructions** are injected into ordinary file-based work before the user request, source files, issue context, tool descriptions, or conversation history are considered.

For a C/H task, the 559-line architecture instruction is also intended to apply, producing approximately **1,416 lines of repository instruction context**. For a C test file, the test instructions can raise this to approximately **1,738 lines**.

This is the central optimization problem.

---

# 2. Comprehensive inventory of currently encoded rules

“Encoded” is the correct term here. Unless a rule is checked by CI, a compiler, permissions, or a validation script, it is not guaranteed enforcement.

## 2.1 Repository identity and build environment

The root instructions establish:

* Project purpose: LinuxCNC HAL driver for PoKeys USB/Ethernet devices.
* C99 and LinuxCNC `.comp` implementation.
* Linux target with RTAI or PREEMPT_RT.
* Dependencies on LinuxCNC HAL, libusb and pthreads.
* Primary Make build and secondary CMake build.
* Userspace and RT `halcompile` commands.
* `halrun` procedures for userspace and RT component tests.
* `test_compile.sh` as a compile test.
* Repository-level responsibilities for:

  * `PoKeysLibHal.h`
  * `PoKeysLibAsync.h/.c`
  * subsystem `PoKeysLib**Async.c`
  * `experimental/pokeys_async.c`
  * `experimental/async_scheduler.*`
  * `hal-canon/`

These are appropriate repository-wide context, although they can be significantly compressed.

---

## 2.2 C and real-time coding rules

Currently encoded real-time and C rules include:

* Use C99 and avoid C++ idioms.
* No dynamic allocation in RT paths.
* Pre-allocate memory before RT execution.
* Use `mlockall(MCL_CURRENT | MCL_FUTURE)`.
* No blocking calls in time-critical paths.
* UDP sockets must be non-blocking.
* Use bounded execution and avoid unbounded iteration.
* Keep ISR or RT functions below:

  * 5 µs for hard RT
  * 50 µs for soft RT
* Measure timing using instrumentation rather than inference.
* Use `hal_u32_t`, `hal_s32_t`, `hal_bit_t` and `hal_float_t` for HAL-visible fields.
* Use `PK_` prefixes for public APIs.
* Use `POKEYSLIB_XXX_H` header guards.
* Use canonical HAL export functions for standard digital, analog and encoder channels.
* Use `#ifdef RTAPI` around RT-specific code.
* Use `rtapi_print_msg` for RT logging.
* Do not use magic command numbers, masks or response offsets.
* Protocol behavior must match the PoKeys protocol specification.
* Maintain feature parity between synchronous/userspace and RT implementations.

These are among the strongest candidates to remain in instructions because they are stable and commonly relevant.

---

## 2.3 Architecture boundaries

The architecture instruction defines a strict layered model:

### `PoKeysLibHal.h`

Must contain:

* HAL-compatible structures.
* Canonical HAL interfaces.
* Expanded bitfields as individual `hal_bit_t` members.

Must not contain:

* Raw C types in HAL-exposed members.
* Function implementations.
* Async infrastructure declarations.

### `PoKeysLibAsync.h`

Must contain:

* Async transaction and mailbox types.
* Command and status enums.
* Async function declarations.
* Shared command constants and response offsets.

Must not contain:

* Implementations.
* HAL export logic.
* HAL data structures belonging to `PoKeysLibHal.h`.

### `PoKeysLibAsync.c`

Must contain only:

* Request creation.
* Sending.
* Receiving and dispatch.
* Timeout and retry handling.
* Mailbox allocation and lifecycle.

Must not contain:

* Subsystem protocol logic.
* HAL exports.
* Subsystem parsers.
* Direct LinuxCNC HAL pin creation.

### `PoKeysLib**Async.c`

Must contain:

* Subsystem HAL export.
* Async send side.
* Parse callback.
* Optional scheduler registration.

### `experimental/pokeys_async.c`

Must act as the integration shell and not become a repository for subsystem logic or direct pin creation.

### `hal-canon`

Must be used for canonical digital, analog and encoder interface exports.

The architecture is sound as a concept. The problem is that the instruction contains extensive teaching material, complete function declarations, example structures, diagrams and workflow narration. Only the invariants need automatic injection.

---

## 2.4 Standards and lifecycle rules

The root instructions state that the following always apply:

* ISO/IEC/IEEE 12207 lifecycle processes.
* ISO/IEC/IEEE 29148 requirements engineering.
* IEEE 1016 software design descriptions.
* ISO/IEC/IEEE 42010 architecture descriptions.
* IEEE 1012 verification and validation.
* XP values and practices.
* Domain-Driven Design.
* Real-time engineering practices.

It also summarizes all nine lifecycle phases in the always-loaded file:

1. Stakeholder requirements.
2. System requirements.
3. Architecture.
4. Detailed design.
5. Implementation.
6. Integration.
7. Verification and validation.
8. Transition/deployment.
9. Operation and maintenance.

The phase-specific files repeat this information in far greater detail:

* Phase 01: stakeholder identification, elicitation, business context and stakeholder issue creation.
* Phase 02: functional/non-functional requirements, acceptance criteria and test planning.
* Phase 03: ADRs, components, quality scenarios and architecture views.
* Phase 04: detailed design, DDD patterns, interfaces, contracts and test cases.
* Phase 05: TDD implementation, code traceability, PR requirements and CI.
* Phase 06: incremental integration, CI and integration test evidence.
* Phase 07: verification, validation, test issues and result reporting.
* Phase 08: release, deployment, training, handoff and rollback.
* Phase 09: incidents, corrective/adaptive/perfective/preventive maintenance and operations.

These are useful procedural assets, but almost none belongs in the repository-wide instruction file.

---

## 2.5 Issue-driven development and traceability

Current policies require:

* All implementation, design and testing work starts with a GitHub issue.
* Issue types include:

  * Stakeholder requirement
  * Functional requirement
  * Non-functional requirement
  * Architecture decision
  * Architecture component
  * Quality scenario
  * Test case
* All required issue fields must be completed.
* Parent and child links must be bidirectional.
* Requirements trace to stakeholder requirements.
* ADRs trace to requirements.
* Components trace to ADRs and requirements.
* Tests trace to requirements.
* PRs trace to implementing issues.
* Commit messages reference issue numbers.
* Source files include issue references.
* Test files include requirement and test issue references.
* PR descriptions use `Fixes #N`, `Implements #N` or corresponding references.
* Requirements cannot be considered complete without tests.
* Architecture decisions require ADR issues.
* Orphaned requirements are prohibited.

The repository contains seven corresponding issue templates, covering stakeholder, functional, non-functional, architecture decision, component, quality scenario and test-case artifacts.

The traceability model should remain. The extensive templates, examples and traversal procedures should move out of always-loaded context.

---

## 2.6 TDD, testing and CI rules

The repository currently demands:

* Tests before production code.
* A failing test before any implementation.
* Red–Green–Refactor.
* Minimal implementation during Green.
* Tests remain green during refactoring.
* Tests link to requirement issues.
* Acceptance criteria appear in tests.
* Positive, negative, boundary and error scenarios are covered.
* Test type and priority metadata are recorded.
* Requirements cannot be completed without passing tests.
* Overall coverage exceeds 80%.
* Critical paths receive higher coverage.
* All tests run before commits or integration.
* Broken CI is fixed within ten minutes.
* Integration occurs multiple times per day.
* Timing assertions require empirical measurement.

The testing instruction then spends more than half its content on generic Python, TypeScript, web login, API latency and SQL-injection examples, none of which reflects the PoKeys C codebase.

The intended rule should be refined to:

> For production behavior changes, first establish a failing automated test or a reproducible failing verification command. For refactoring, establish a green baseline first. Do not invent artificial tests for documentation-only changes.

This preserves TDD discipline without requiring meaningless tests for Markdown, workflow documentation, comments or metadata changes.

---

## 2.7 Documentation rules

Current rules require:

* Documentation changes accompany code changes.
* Documentation must remain consistent with implementation.
* Existing documentation should be updated rather than creating another file.
* New Markdown files should not duplicate existing topics.
* Temporary documents must be consolidated and removed.
* README files follow a fixed section order.
* README claims, badges and version numbers must reflect reality.
* Terms and acronyms must be defined.
* Architecture, requirements and design documents follow their cited standards.
* Markdown is the standard documentation format.
* Speculative documentation for unimplemented features is prohibited.

These are valuable rules, but the README file is currently not recognized because its filename is `Readme_instructions.md` instead of ending in `.instructions.md`.

---

## 2.8 Communication and engineering philosophy

The root and duplicate instruction files also impose:

* “Slow is fast.”
* “No excuses.”
* “No shortcuts.”
* Clarify first.
* Report bad news immediately.
* Provide options rather than excuses.
* Separate estimates from promises.
* Never blame individuals.
* Use Five Whys.
* Treat negative emotions as design signals.
* Make status visible within 15 seconds.
* Prefer boring technology.
* Make small changes.
* Strong opinions, weakly held.
* Leave the campsite cleaner.
* Reuse before reinventing.
* Maintain one source of truth.
* Curate rather than accumulate.

Most of this is useful human process guidance but weak machine instruction. Terms such as “no shortcuts,” “quality,” “simple,” and “ownership” are underspecified and can conflict with one another. A good skill should state explicit boundaries, ownership, prohibited actions and completion checks rather than relying on generic best-practice language. ([Keep It Simple, Stupid!][2])

Recommended treatment:

* Keep one concise instruction: “Report unverified assumptions, failed checks and incomplete work explicitly.”
* Move the extended philosophy to `docs/engineering-principles.md`.
* Encode concrete manifestations in individual skills and CI checks.

---

## 2.9 Submodule governance

The submodule instruction currently demands:

* Immutable pinned SHAs.
* No branch tracking.
* Explicit bump workflows.
* Adapter layers.
* Contract tests.
* License and security review.
* Changelog and ADR updates.
* Green CI before integration.
* Dedicated add, update and removal procedures.
* Quarterly review and security monitoring.

The repository has exactly two submodules:

* `pokeyslib`
* `hal-canon`

This material should not use `applyTo: "**"`. A proper scope would be:

```yaml
---
applyTo: ".gitmodules,pokeyslib/**,hal-canon/**"
---
```

The detailed add/bump/remove procedures should be a `manage-submodules` skill.

---

# 3. What is genuinely enforced by automation

## Mechanically active

### Build workflow

The build workflow:

* Runs on pushes and PRs to `main`.
* Runs every two weeks.
* Uses a self-hosted LinuxCNC runner.
* Checks out recursive submodules.
* Builds the userspace library.
* Builds the CMake version.
* Builds the HAL library.
* Compiles userspace and RT components.
* Runs userspace and RT `halrun` smoke tests.
* Performs RTC output range checks.

However:

* One build step uses `continue-on-error: true`.
* RTC validation treats missing, zero or out-of-range values as warnings and exits successfully.
* No coverage threshold is visible.
* No issue-link or traceability check is visible.
* No measured 5 µs/50 µs timing gate is visible.

### Lifecycle issue workflow

`create-lifecycle-issues.yml` creates a requirements–architecture–design issue chain, but it is hardcoded specifically for “Async Scheduler Optimizations” and stakeholder requirement `#97`. It is an issue-generation workflow, not a general traceability validator.

## Textual claims without located implementation

A repository search for `validate-traceability` returned agent and prompt references, but no corresponding validation script or workflow.

Therefore, the following are presently policies rather than reliable enforcement:

* Every PR links an issue.
* Every test links a requirement.
* Every source file has traceability metadata.
* Every requirement has tests.
* Coverage exceeds 80%.
* No architecture violation occurs.
* CI is repaired within ten minutes.
* Timing limits are met.
* Documentation is updated with every change.

This distinction should be made explicit in the instructions.

---

# 4. Contradictions and failure modes

## 4.1 Submodule policy contradicts CI

The instruction says submodules must be pinned and not track moving branches.

The build workflow executes:

```bash
git submodule update --remote --recursive
```

and automatically commits and pushes resulting changes.

This is a direct contradiction. Recommended correction:

* CI builds the committed submodule SHAs only.
* A separate scheduled `submodule-bump.yml` opens a PR for candidate updates.
* The PR runs contract/build tests.
* No build job mutates `main`.

## 4.2 Root file duplicates scoped content

The root file includes:

* Full lifecycle descriptions.
* Standards material.
* TDD procedures.
* issue templates.
* PR templates.
* test examples.
* architecture examples.
* communication philosophy.

Most of these also exist in phase instructions, test instructions, prompts or agents. Combining multiple applicable instruction files has no general conflict-resolution precedence, so contradictions should be avoided rather than expecting one file to override another. ([GitHub Docs][3])

## 4.3 Duplicate “global” instruction source

`.github/instructions/copilot-instructions.md` duplicates project architecture and engineering philosophy, but does not use the required `.instructions.md` suffix. It is simultaneously:

* Too large.
* Redundant.
* Not reliably active.
* An unclear competing source of truth.

It should be deleted after unique material is merged into appropriate destinations.

## 4.4 Generic examples contaminate a C/RT repository

A large portion of the instructions and prompts uses:

* JWT authentication.
* REST APIs.
* TypeScript.
* Jest.
* bcrypt.
* SQL injection.
* web sessions.
* Redis.
* cloud deployment.
* database services.

For example, `tdd-compile.prompt.md` is a long Jest/JWT/database authentication scenario rather than a PoKeys C test workflow.

This increases the likelihood of irrelevant design suggestions and invented project structure.

## 4.5 The repository audit prompt audits the wrong archetype

`repository-audit.prompt.md` assumes a Node/Python/Java-style repository with `src/`, `tests/`, `package.json`, web deployment, Terraform and database concerns.

It should either become:

* A generic reusable skill maintained outside this repository, or
* A PoKeys-specific audit skill for C, LinuxCNC, RT safety, async protocol coverage and HAL compatibility.

---

# 5. Correct allocation by customization type

GitHub’s current recommendation is:

* Custom instructions for short rules relevant to almost every task.
* Path instructions for persistent rules that apply to particular files.
* Skills for detailed, repeatable procedures loaded when relevant.
* Prompt files for explicitly invoked interactions.
* Agents for specialized roles with defined tool access and behavior. ([GitHub Docs][4])

## Recommended allocation matrix

| Current material                                                 | Recommended destination                                  |
| ---------------------------------------------------------------- | -------------------------------------------------------- |
| Project purpose, language, target, canonical build commands      | Root instructions                                        |
| Three or four critical RT safety invariants                      | Root instructions and C/H path instructions              |
| Detailed file ownership rules                                    | Concise C/H path instruction                             |
| Complete async-conversion procedure                              | `convert-to-hal-rtapi` skill                             |
| Protocol extraction and verification workflow                    | `convert-to-hal-rtapi` or `verify-pokeys-protocol` skill |
| QtPyVCP multi-file generation                                    | `qtpyvcp-ui-generator` skill                             |
| Full submodule add/bump/remove workflow                          | `manage-submodules` skill                                |
| Submodule invariants affecting `.gitmodules` and submodule paths | Scoped path instruction                                  |
| Issue/requirement linking syntax                                 | Short root/path rule                                     |
| Traceability traversal, generation and validation                | Skill plus validation script                             |
| Red–Green–Refactor implementation procedure                      | `implement-issue-tdd` skill                              |
| Requirements elicitation and refinement                          | Requirements agent plus small invocation prompts         |
| ADR and component creation                                       | Architecture agent/skill                                 |
| Test-gap analysis and test generation                            | Verification/testing skill                               |
| Reliability, SRG, SFMEA and release-decision procedures          | `reliability-engineering` skill family                   |
| Project kickoff                                                  | Prompt file                                              |
| Compile current workspace                                        | Small prompt file                                        |
| Lint current changes                                             | Small prompt file                                        |
| Repository-wide audit                                            | Audit agent or skill                                     |
| README structure invariants                                      | `readme.instructions.md`                                 |
| Full README generation/rewrite process                           | Documentation skill or agent                             |
| “Slow is fast,” “no excuses,” XP philosophy                      | Human-facing engineering principles document             |
| Tool permissions and specialist role                             | Custom agent                                             |
| Shared cross-tool standing rules                                 | Root `AGENTS.md`, kept minimal                           |

---

# 6. Existing prompt classification

The indexed `.github/ptompts/` collection includes:

* `lint`
* `compile`
* `tdd-compile`
* `project-kickoff`
* `phase-gate-check`
* `standards-validate`
* `requirements-elicit`
* `requirements-refine`
* `requirements-validate`
* `requirements-complete`
* `user-story-expansion`
* `architecture-starter`
* `repository-audit`
* `code-to-requirements`
* `traceability-builder`
* `traceability-validate`
* `test-gap-filler`
* `test-validate`
* `acceptance-test-generate`
* `sfmea-create`
* `corrective-action-loop`
* `reliability-plan-create`
* `operational-profile-create`
* `srg-model-fit`
* `reliability-test-design`
* `reliability-release-decision`

The folder must first be renamed:

```text
.github/ptompts/
    ↓
.github/prompts/
```

Prompt files are manually selected for a specific interaction. They should usually remain relatively small and may attach files or pass arguments. ([GitHub Docs][5])

## Keep as prompt files

These make sense as explicit commands:

* `project-kickoff.prompt.md`
* `compile.prompt.md`, after reducing it to build/verify rather than full implementation
* `lint.prompt.md`
* `requirements-elicit.prompt.md`
* `requirements-refine.prompt.md`
* `requirements-validate.prompt.md`
* `user-story-expansion.prompt.md`
* `architecture-starter.prompt.md`
* `phase-gate-check.prompt.md`

## Convert to skills

These contain or imply multi-step reusable procedures:

* `tdd-compile` → `implement-issue-tdd`
* `code-to-requirements` → `recover-requirements-from-code`
* `traceability-builder` and `traceability-validate` → `manage-traceability`
* `test-gap-filler`, `test-validate`, `acceptance-test-generate` → `verification-testing`
* `repository-audit`, `standards-validate` → `repository-quality-audit`
* SFMEA/reliability prompts → one `reliability-engineering` skill with references
* `corrective-action-loop` → `corrective-action`

A short prompt may remain as a convenient manual entry point that instructs the selected agent to use the corresponding skill.

---

# 7. Existing skill evaluation

## ConvertToHalRtapi

Conceptually, this is already the strongest skill candidate. It has:

* Clear use cases.
* Inputs and outputs.
* Prerequisites.
* A defined procedure.
* Protocol verification.
* HAL export rules.
* async send/parse conversion.
* testing stages.

But the current structure is not a valid Copilot skill.

Recommended conversion:

```text
.github/skills/
└── convert-to-hal-rtapi/
    ├── SKILL.md
    ├── references/
    │   ├── architecture-contract.md
    │   ├── hal-export-patterns.md
    │   └── protocol-verification.md
    ├── templates/
    │   └── subsystem-async.c
    └── scripts/
        ├── check-blocking-calls.sh
        └── check-hal-exports.py
```

Minimal frontmatter:

```yaml
---
name: convert-to-hal-rtapi
description: >
  Convert a synchronous PoKeysLib subsystem into a non-blocking,
  LinuxCNC RT-capable async HAL implementation. Use when creating or
  modifying PoKeysLib*Async.c subsystem implementations.
---
```

Copilot requires the exact filename `SKILL.md`, lowercase-hyphen directory naming, and `name` plus `description` frontmatter. It selects skills based primarily on that description. ([GitHub Docs][4])

## QtPyVCP

This also contains a valid skill-shaped procedure:

* Clear triggering conditions.
* Explicit exclusions.
* Inputs.
* mandatory behavioral rules.
* multi-file generation sequence.
* validation and output contract.

It should become:

```text
.github/skills/qtpyvcp-ui-generator/SKILL.md
```

Large widget/API examples should be placed in `references/`, not in the main `SKILL.md`. Skills support progressive loading of relevant procedures and bundled resources, which is the primary mechanism for avoiding giant startup prompts. ([GitHub Docs][6])

---

# 8. Existing agent evaluation

Current agent roles are:

* Requirements Analyst
* Architecture Strategist
* TDD Driver
* Testing Specialist
* Documentation Expert
* Security Analyst

Their conceptual separation is reasonable, but the implementation needs revision.

## Required changes

1. Rename files:

```text
requirements-analyst.md
    ↓
requirements-analyst.agent.md
```

GitHub creates and documents custom agent profiles as `.agent.md` files under `.github/agents`. ([GitHub Docs][7])

2. Validate tool identifiers.

Current declarations use:

```yaml
tools: ["read", "search", "edit", "githubRepo"]
```

These may not correspond to the tool identifiers exposed by the current Copilot installation. They should be selected through the agent editor/autocomplete and tested on the target surface.

3. Do not hardcode:

```yaml
model: reasoning
```

unless that exact model alias is confirmed in the deployment environment.

4. Replace generic JWT/web examples with PoKeys examples.

5. Define explicit agent boundaries:

* Requirements agent: may create or edit requirements and acceptance criteria, but not production code.
* Architecture agent: may propose architecture and ADRs, but not silently implement.
* TDD driver: may modify tests and production code for an approved issue.
* Testing specialist: focuses on tests, fixtures and verification evidence.
* Documentation agent: edits documentation, but does not invent implemented behavior.
* Security/RT-safety reviewer: read/review by default; implementation only when explicitly assigned.

6. Add handoffs rather than duplicating procedures in every agent.

## Missing `AGENTS.md`

`.github/agents/README.md` claims that a root `AGENTS.md` exists and is always loaded, but the repository does not contain it. The README also builds its selection and lifecycle workflow around this missing file.

Choose one of two approaches:

### Copilot-only

Remove all claims about root `AGENTS.md` and keep a concise `.github/copilot-instructions.md`.

### Cross-tool

Create a minimal root `AGENTS.md` containing shared rules for Copilot, Claude Code, Codex and other agents. Keep Copilot-specific details in `.github/copilot-instructions.md`. `AGENTS.md` is appropriate for standing rules intended to be shared across agent systems. ([GitHub Docs][8])

The cross-tool approach fits this repository better, but the two files must not duplicate hundreds of lines.

---

# 9. Proposed target structure

```text
AGENTS.md                                  # 40–80 lines, cross-agent invariants

.github/
├── copilot-instructions.md               # 60–100 lines, Copilot-specific context
│
├── instructions/
│   ├── c-realtime.instructions.md        # C/H architecture + RT invariants
│   ├── tests.instructions.md             # Actual C test conventions
│   ├── readme.instructions.md            # README-only invariants
│   ├── submodules.instructions.md        # .gitmodules + two submodule paths
│   ├── requirements.instructions.md      # Phase 01/02 artifact invariants
│   ├── architecture-design.instructions.md
│   └── verification-release.instructions.md
│
├── skills/
│   ├── convert-to-hal-rtapi/
│   │   ├── SKILL.md
│   │   ├── references/
│   │   ├── templates/
│   │   └── scripts/
│   ├── verify-pokeys-protocol/
│   │   └── SKILL.md
│   ├── implement-issue-tdd/
│   │   └── SKILL.md
│   ├── manage-traceability/
│   │   ├── SKILL.md
│   │   └── scripts/
│   ├── manage-submodules/
│   │   └── SKILL.md
│   ├── repository-quality-audit/
│   │   └── SKILL.md
│   ├── reliability-engineering/
│   │   └── SKILL.md
│   └── qtpyvcp-ui-generator/
│       └── SKILL.md
│
├── agents/
│   ├── requirements-analyst.agent.md
│   ├── architecture-strategist.agent.md
│   ├── implementation-driver.agent.md
│   ├── verification-specialist.agent.md
│   ├── documentation-maintainer.agent.md
│   └── rt-safety-reviewer.agent.md
│
└── prompts/
    ├── project-kickoff.prompt.md
    ├── implement-issue.prompt.md
    ├── compile.prompt.md
    ├── lint.prompt.md
    ├── requirements-elicit.prompt.md
    ├── requirements-refine.prompt.md
    ├── architecture-starter.prompt.md
    └── phase-gate-check.prompt.md
```

---

# 10. What should remain in always-loaded instructions

The root context should contain only the following.

## Repository identity

* PoKeysLibHal is a C99 LinuxCNC HAL driver.
* Supports userspace and real-time operation.
* Communication architecture is asynchronous and non-blocking.
* `pokeyslib` is the upstream/reference submodule.
* `hal-canon` provides canonical HAL channel interfaces.

## Critical invariants

* Never block or dynamically allocate in RT execution paths.
* Preserve the async infrastructure/subsystem/integration-shell boundaries.
* Use HAL-compatible types for HAL-visible fields.
* Do not claim timing compliance without measurement.
* Do not change protocol behavior without checking the protocol specification.

## Workflow defaults

* For behavior changes, establish a failing test or reproducible failing check first.
* Keep changes small and scoped to an issue or clearly stated task.
* Run the relevant build and verification commands before claiming completion.
* Report failed, skipped and unavailable checks explicitly.
* Update existing documentation rather than creating redundant documents.

## Canonical commands

Only the smallest reliable set:

* Build library.
* Compile userspace component.
* Compile RT component.
* Run compile/smoke tests.

## Routing

* C/H rules: link to `c-realtime.instructions.md`.
* Detailed conversions: use `convert-to-hal-rtapi`.
* Submodules: use `manage-submodules`.
* Requirements/architecture/testing: use corresponding agent or skill.

Everything else should load only when relevant.

---

# 11. Prioritized optimization backlog

## P0 — Repair discovery and correctness

1. Rename `.github/ptompts` to `.github/prompts`.
2. Rename prompt agents from `.md` to `.agent.md`.
3. Convert both skill folders to lowercase-hyphen names.
4. Add valid `SKILL.md` files with required frontmatter.
5. Rename `Readme_instructions.md` to `readme.instructions.md`.
6. Delete or relocate `.github/instructions/copilot-instructions.md`.
7. Create root `AGENTS.md` or remove all references to it.
8. Replace array-form `applyTo` frontmatter with documented comma-separated strings.
9. Add C test patterns:

   * `**/test_*.c`
   * `**/*_test.c`
   * `**/tests/**/*.c`
   * `test_compile.sh`
10. Verify discovery using Copilot’s instruction, prompt, agent and skill diagnostics.

## P1 — Remove context overload

1. Reduce root instructions from 621 to approximately 80–120 lines.
2. Change submodule scope from `**` to `.gitmodules,pokeyslib/**,hal-canon/**`.
3. Reduce architecture instructions from 559 lines to approximately 80–150 lines.
4. Remove generic JWT, database, REST and TypeScript examples.
5. Replace them with one or two concise PoKeys examples.
6. Remove nine-phase summaries from root instructions.
7. Retain only phase routing and invariant traceability syntax.
8. Move extended engineering philosophy into normal documentation.

## P2 — Consolidate procedures

1. Combine the requirements prompt family into a requirements agent plus small entry prompts.
2. Combine TDD/test prompts into `implement-issue-tdd` and `verification-testing`.
3. Combine traceability prompts into `manage-traceability`.
4. Combine reliability/SFMEA/SRG prompts into `reliability-engineering`.
5. Move submodule workflows into `manage-submodules`.
6. Add scripts and templates as skill resources rather than embedding hundreds of example lines.

## P3 — Convert claimed rules into actual gates

1. Implement a traceability validator.
2. Validate PR issue references.
3. Validate test-to-requirement references.
4. Validate forbidden blocking calls in RT paths.
5. Validate direct HAL export calls against architecture rules.
6. Add real C coverage collection before claiming an 80% gate.
7. Add timing benchmark evidence before claiming ISR/runtime limits.
8. Separate submodule-update automation from build CI.
9. Remove `continue-on-error` where a build is genuinely mandatory.
10. Distinguish warnings from release-blocking failures.

## P4 — Add customization regression tests

Create a small evaluation suite with tasks such as:

* Explain the project without loading a lifecycle manual.
* Modify a C async subsystem.
* Update only the README.
* Bump `pokeyslib`.
* Add a failing C test for a protocol parser.
* Create an ADR.
* Audit traceability.
* Generate a QtPyVCP panel.
* Review an RT path for blocking operations.
* Perform a documentation-only correction.

For each task, record:

* Which instructions loaded.
* Which skill activated.
* Which agent was used.
* Whether irrelevant web-auth examples appeared.
* Whether required verification occurred.
* Whether prohibited actions were attempted.

---

# Final assessment

The repository does not lack instructions. It has **too many instructions in the wrong loading layers**.

The intended engineering system is coherent:

```text
Requirement
→ Architecture
→ Design
→ Failing test
→ Implementation
→ Verification
→ Integration
→ Release
```

That sequence should remain. The optimization is to represent it as:

* **Short standing invariants** in root/path instructions.
* **Explicit specialist roles** in agents.
* **Detailed, contract-like procedures** in skills.
* **Small manual entry points** in prompt files.
* **Deterministic checks** in scripts and CI.

The immediate context reduction target should be from approximately **857 generally applicable lines** to no more than roughly **150–250 combined lines for an ordinary C task**, with detailed procedures loaded only when the task actually requires them.

[1]: https://docs.github.com/en/copilot/how-tos/copilot-on-github/customize-copilot/add-custom-instructions/add-repository-instructions?ref=hackernoon.com&tool=vscode "https://docs.github.com/en/copilot/how-tos/copilot-on-github/customize-copilot/add-custom-instructions/add-repository-instructions?ref=hackernoon.com&tool=vscode"
[2]: https://blog.skopow.ski/a-good-agent-skill-is-a-contract-not-a-prompt "https://blog.skopow.ski/a-good-agent-skill-is-a-contract-not-a-prompt"
[3]: https://docs.github.com/en/copilot/how-tos/copilot-cli/customize-copilot/add-custom-instructions "https://docs.github.com/en/copilot/how-tos/copilot-cli/customize-copilot/add-custom-instructions"
[4]: https://docs.github.com/en/copilot/how-tos/copilot-on-github/customize-copilot/customize-cloud-agent/add-skills "https://docs.github.com/en/copilot/how-tos/copilot-on-github/customize-copilot/customize-cloud-agent/add-skills"
[5]: https://docs.github.com/en/copilot/how-tos/configure-custom-instructions-in-your-ide/add-repository-instructions-in-your-ide?tool=visualstudio "https://docs.github.com/en/copilot/how-tos/configure-custom-instructions-in-your-ide/add-repository-instructions-in-your-ide?tool=visualstudio"
[6]: https://docs.github.com/en/copilot/concepts/agents/about-agent-skills "https://docs.github.com/en/copilot/concepts/agents/about-agent-skills"
[7]: https://docs.github.com/en/copilot/how-tos/copilot-on-github/customize-copilot/customize-cloud-agent/create-custom-agents "https://docs.github.com/en/copilot/how-tos/copilot-on-github/customize-copilot/customize-cloud-agent/create-custom-agents"
[8]: https://docs.github.com/en/copilot/concepts/agents/code-review "https://docs.github.com/en/copilot/concepts/agents/code-review"
