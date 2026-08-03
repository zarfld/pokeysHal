You are working in the repository:

    zarfld/pokeysHal

This task is Phase 0 of the HAL-interface compatibility work.

# Objective

Establish a comprehensive, evidence-based baseline of all requirements governing the
PoKeys LinuxCNC HAL interface before any compatibility plan, test manifest, Copilot
skill, production-code change, or refactoring is designed.

The baseline must cover:

- HAL pins
- HAL parameters
- exported HAL functions
- pin and parameter naming
- types, directions and access modes
- cardinality and conditional creation
- initialization defaults
- device-to-HAL and HAL-to-device update directions
- PoKeysLib enumerations
- bitmaps and per-bit decomposition
- physical PoKeys pin-function configuration
- ownership of pin creation and initialization
- LinuxCNC HAL lifecycle requirements
- `hal-canon` usage and implementation rules
- compatibility with the legacy `LinuxCnc_PokeysLibComp` implementation
- conflicts and contradictions among requirements, ADRs, issues and implementations

This is a requirements-discovery and authority-resolution task.

It is not an implementation task.

# Strict scope restrictions

Do not:

- modify production C code;
- modify `hal-canon`;
- update a submodule revision;
- implement or change HAL pins or parameters;
- create HAL compatibility tests;
- create a HAL compatibility Copilot skill;
- redesign PoKeysLib async request or response handling;
- modify packet construction, parsing, retries, transactions or scheduling;
- resolve requirement conflicts silently;
- close, edit or comment on GitHub issues;
- create or merge a pull request;
- claim hardware validation;
- claim realtime or timing validation.

Do not treat issue closure as proof that a requirement was implemented.

Do not assume that an issue, ADR or SDD is correct merely because it is detailed or marked
accepted.

Only documentation and optional read-only audit tooling may be added.

Any audit tooling must:

- inspect files without modifying them;
- be deterministic;
- have no hardware dependency;
- not become the normative requirements source.

# Repository handling

1. Verify the current workspace:

   git status --short
   git branch --show-current
   git rev-parse HEAD
   git remote -v
   git submodule status

2. Do not discard, overwrite, stash or alter existing user changes.

3. If the worktree is not clean, report this and continue read-only unless the existing
   changes prevent reliable analysis.

4. Locate the related repository:

   zarfld/LinuxCnc_PokeysLibComp

   Prefer an existing sibling checkout. If none exists, inspect it through `gh` or clone
   it into a temporary directory outside the `pokeysHal` working tree.

5. Record exact revisions for:

   - `pokeysHal`
   - `LinuxCnc_PokeysLibComp`
   - the `hal-canon` submodule
   - any other referenced submodule that supplies HAL-interface definitions

Do not use floating branch names as evidence when a commit SHA can be recorded.

# Authority hierarchy

Classify every source using this hierarchy.

## A. Normative external authority

Current official LinuxCNC documentation and API definitions, including as applicable:

- HAL component lifecycle
- `hal_init()`
- `hal_malloc()`
- `hal_pin_*_newf()`
- `hal_param_*_newf()`
- `hal_export_funct()`
- `hal_ready()`
- `hal_exit()`
- canonical device interfaces
- HAL object-name length constraints
- realtime restrictions
- joint, motion and homing interfaces

These establish LinuxCNC requirements.

## B. Pinned dependency authority

The exact pinned `hal-canon` revision.

Determine what it actually defines and which implementation conventions it imposes.

## C. Explicit repository requirements

Issues and repository documents that contain stakeholder or functional requirements.

## D. Architecture and design records

ADRs, architecture-component issues and SDDs.

These may refine requirements but must not silently contradict them.

## E. Legacy compatibility implementation

The HAL interface actually exported by `LinuxCnc_PokeysLibComp`, including its modularized
userspace implementation and earlier `pokeys.comp` where relevant.

This establishes legacy compatibility behaviour, not LinuxCNC normative behaviour.

## F. Current implementation

The HAL interface currently exported by `pokeysHal`.

## G. Integration examples

HAL and INI files, sample machine configurations and homing configurations.

These demonstrate expected usage but are not automatically normative component
requirements.

## H. Issue comments and generated prose

Use as evidence and historical context, but verify claims against code and authoritative
sources.

# Seed issues

Use the following only as starting points. Search comprehensively for additional related
issues and pull requests.

## LinuxCnc_PokeysLibComp

- #16 — pokeys_py
- #21 — custom homing interface
- #24 — homecomp compatibility
- #28 — userspace functional equivalence
- #30 — tests for implemented features
- #31 — pokeys_py tests
- #69 — modular architecture
- #73 — LinuxCNC build alignment
- #79 — conformity with LinuxCNC guidelines and canonical device interface
- #129 — realtime clock HAL interface
- #157 — common pin I/O handling
- #213 — encoder modularization and manual HAL export
- #216 — missing PEv2 homing pin
- #222 — physical pin-function setup
- #223 — PEv2 limit override
- #264 — analog output in userspace component
- #310 — missing PEv2 AxesState and HAL name length
- #326 — HAL parameter name longer than LinuxCNC limit

## pokeysHal

- #24 — invalid `hal_free()` expectation
- #32 — device-information HAL interface
- #33 — PEv2 HAL interface
- #34 — PoExtBus HAL interface
- #35 — analog-input HAL interface
- #36 — digital-output HAL interface
- #37 and nearby HAL-interface issues
- #38 — digital-input HAL interface
- #39 — analog-output HAL interface
- #102 — digital-output scheduler requirement
- #116 through #126 — PEv2 requirements
- #127 through #131 — PEv2 ADRs
- #132 — PEv2 architecture component
- #133 — PEv2 SDD

Search all open and closed issues, comments and relevant pull requests for terms including:

    HAL
    HAL-Interface
    HAL Interface
    canonical
    canonical device
    hal-canon
    hal_pin
    hal_param
    hal_malloc
    hal_ready
    hal_exit
    PinFunction
    ePK_PinCap
    bitmap
    bitmask
    enumeration
    digin
    digout
    adcin
    adcout
    encoder
    counter
    PWM
    PEv2
    PoExtBus
    PoNET
    homing
    probe
    limit
    enable
    initialization
    export
    pin creation
    parameter creation

Inspect issue comments as well as issue bodies.

# Code inventory

Search both repositories and `hal-canon` for at least:

    hal_init
    hal_malloc
    hal_pin_
    hal_param_
    hal_export_funct
    hal_ready
    hal_exit
    hal_free
    PinFunction
    ePK_PinCap
    PK_PinConfigurationGet
    PK_PinConfigurationSet
    ApplyIniSettings
    AxesConfig
    AxesSwitchConfig
    AxisEnabledMask
    AxisEnabledStatesMask
    LimitStatus
    HomeStatus
    ErrorInputStatus
    DedicatedLimit
    DedicatedHome
    MiscInputStatus
    HomingStartMask
    ProbeStartMask
    BacklashCompensationEnabled

Identify:

- every file that creates HAL objects;
- every structure containing HAL pin pointers or parameter storage;
- where those structures are allocated;
- where defaults are assigned;
- where HAL outputs are updated from PoKeysLib state;
- where HAL inputs are copied into PoKeysLib state;
- where physical pin configuration is read or written;
- where `hal_ready()` occurs;
- all cleanup paths;
- all uses or attempted uses of `hal_free()`;
- all conditional HAL-object creation;
- all fixed-size arrays that imply cardinality;
- all names that may violate LinuxCNC length limits.

# `hal-canon` audit

Determine and document:

1. exact upstream repository;
2. exact pinned commit;
3. submodule path and configuration;
4. whether it is compiled into the component or used only as reference code;
5. public types and functions it supplies;
6. digital-input abstraction;
7. digital-output abstraction;
8. analog-input abstraction;
9. analog-output abstraction;
10. where it expects HAL memory allocation;
11. where it creates pins and parameters;
12. how initial values are assigned;
13. how cyclic read and write updates are structured;
14. whether inversion, scaling, offsets and limits are implemented there;
15. whether current `pokeysHal` code follows, partially follows or bypasses it;
16. whether PEv2 or other noncanonical subsystems are expected to reuse its patterns.

Do not describe an inferred rule as established unless the code or documentation supports it.

# Required deliverables

Create the following directory:

    docs/hal-interface/phase-0/

Create these files.

## 1. `README.md`

Provide:

- Phase 0 objective;
- scope and exclusions;
- repository revisions examined;
- authority hierarchy;
- methodology;
- principal findings;
- unresolved questions;
- completion status;
- explicit statement that no compatibility design or tests were created.

## 2. `source-register.yaml`

One entry per important source:

```yaml
- source_id:
  repository:
  revision:
  path_or_issue:
  url:
  source_type:
  authority_class:
  scope:
  state:
  inspected:
  notes:
````

Include official LinuxCNC sources, both repositories, `hal-canon`, requirements, ADRs,
implementations and examples.

## 3. `requirement-catalogue.yaml`

Each distinct HAL object or cross-cutting requirement must use this structure:

```yaml
- interface_id:
  subsystem:
  name_pattern:
  object_kind: pin | parameter | function | lifecycle-rule | configuration-rule
  hal_type:
  direction_or_access:
  cardinality:
  creation_condition:
  units:
  scaling_formula:
  default_value:
  initialization_phase:
  update_phase:
  device_field:
  pokeyslib_function:
  enumeration:
  bitmap_definition:
  linuxcnc_canonical_status:
  legacy_compatibility_status:
  current_implementation_status:
  requirement_sources:
  implementation_sources:
  conflicts:
  evidence:
```

Do not leave a field blank without explaining that it is unknown, not applicable or
unverified.

At minimum cover:

* component/device information;
* digital input;
* digital output;
* physical `PinFunction`;
* analog input;
* analog output/PWM;
* counters;
* basic, fast and ultrafast encoders;
* PoExtBus;
* PoNET-related HAL objects;
* realtime clock;
* PEv2 global interface;
* PEv2 per-axis interface;
* homing;
* probing;
* limit handling and override;
* emergency input/output;
* bitmapped fields;
* enumeration-backed fields;
* exported read/write functions;
* initialization and readiness.

## 4. `lifecycle-ownership-matrix.md`

For every subsystem, record:

| Subsystem | HAL storage | Allocation | Object creation | Initial defaults | Device→HAL update | HAL→device update | Physical configuration | Readiness dependency | Cleanup | Evidence |
| --------- | ----------- | ---------- | --------------- | ---------------- | ----------------- | ----------------- | ---------------------- | -------------------- | ------- | -------- |

Explicitly identify which file/function owns each step.

## 5. `canonical-vs-legacy-matrix.md`

Compare:

| Interface item | Official LinuxCNC | Legacy component | Current pokeysHal | Classification | Decision required |
| -------------- | ----------------- | ---------------- | ----------------- | -------------- | ----------------- |

Use classifications such as:

* canonical and compatible;
* canonical but missing;
* legacy extension;
* PoKeys-specific extension;
* renamed;
* type mismatch;
* direction mismatch;
* semantic mismatch;
* implementation-only;
* obsolete;
* unresolved.

Examples that require explicit examination include:

* `adcin.#.value-raw`;
* canonical analog-input fields such as `bit_weight` and `hw_offset`;
* analog-output `max_v`;
* shared PWM period;
* PEv2 raw bitmap pins versus decomposed bit pins;
* generic I/O versus PEv2 ownership of the same physical pin;
* HAL object name-length limits.

## 6. `conflict-register.md`

Each conflict must contain:

```text
Conflict ID:
Subject:
Source A:
Source B:
Observed implementation:
Why the sources conflict:
Safety or compatibility impact:
Authority assessment:
Required decision:
Status: unresolved | resolved | deferred
```

At minimum include the known PEv2 contradiction:

* requirement #118: `nrOfAxes == 0` creates no per-axis pins;
* ADR #128: `nrOfAxes == 0` creates all eight groups as fallback.

Search for further contradictions, including:

* enabled axes versus hardware-supported axes;
* issue descriptions versus ADRs;
* closed issues versus comments saying work remained incomplete;
* legacy `pokeys.comp` names versus modularized userspace names;
* `analogin`/`analogout` versus `adcin`/`adcout`;
* pin versus parameter classification;
* HAL_IN/HAL_OUT direction;
* raw bitmap exposure versus decomposed bit pins;
* dynamic discovery versus the rule that HAL objects must exist before `hal_ready()`.

Do not resolve a conflict merely by selecting the newest source.

## 7. `traceability.md`

Build traceability chains in this form:

```text
LinuxCNC normative rule
  → repository stakeholder requirement
  → subsystem functional requirement
  → ADR/design record
  → legacy implementation
  → current implementation
  → future verification target
```

Mark broken, contradictory and missing links.

## 8. `issue-inventory.md`

Provide all relevant issues and pull requests from both repositories with:

* repository and number;
* title;
* open/closed state;
* relevance;
* claimed implementation;
* actual evidence found;
* conflicts;
* whether it should remain a source for future compatibility work.

## 9. `open-decisions.md`

List only decisions that must be made before compatibility tests or implementation work
can begin.

Group decisions by:

* LinuxCNC canonical conformity;
* backward compatibility;
* object naming;
* object type and direction;
* lifecycle;
* pin cardinality;
* physical pin-function ownership;
* PEv2 decomposition;
* homing integration;
* initialization fallback behaviour;
* deprecation and aliases.

Do not make those decisions during Phase 0.

## 10. `phase-0-completion-report.md`

Provide an acceptance table:

| Criterion | PASS/FAIL/PARTIAL | Evidence | Residual gap |
| --------- | ----------------- | -------- | ------------ |

Acceptance criteria:

1. both repositories inspected;
2. open and closed issues searched;
3. issue comments inspected where relevant;
4. official LinuxCNC rules recorded;
5. exact `hal-canon` provenance recorded;
6. legacy HAL interface extracted from source;
7. current HAL interface extracted from source;
8. lifecycle and ownership documented;
9. enumerations and bitmaps documented;
10. canonical and project-specific extensions distinguished;
11. contradictions recorded;
12. no production code changed;
13. no compatibility tests designed;
14. no unresolved claim presented as fact.

Finish with exactly one status:

```
PHASE 0 BASELINE COMPLETE
```

or:

```
PHASE 0 BASELINE INCOMPLETE
```

If incomplete, enumerate the missing evidence precisely.

# Evidence rules

Every technical conclusion must include evidence using one or more of:

* repository-relative `file:line`;
* repository, issue number and quoted/paraphrased requirement;
* repository, pull request and commit SHA;
* official documentation section and retrieval date;
* submodule commit SHA.

Do not cite only a generated summary when primary code or requirement text exists.

Clearly distinguish:

* requirement;
* architecture decision;
* legacy behaviour;
* current behaviour;
* inference;
* unresolved interpretation.

# Validation

Before completion:

1. confirm only permitted files changed:

   git status --short
   git diff --stat
   git diff --check

2. inspect the documentation for unsupported claims;

3. verify YAML parses successfully;

4. verify all referenced repository paths exist at the recorded revisions;

5. verify no source, build, test, fixture or submodule files changed;

6. report commands run and commands that could not be run.

# Final response

Report:

1. workspace and revisions inspected;
2. files created;
3. major source categories found;
4. number of catalogue entries;
5. number of conflicts;
6. most consequential unresolved decisions;
7. validation performed;
8. final Phase 0 status;
9. suggested next step, limited to reviewing and resolving the Phase 0 findings.

Do not begin Phase 1.

```
