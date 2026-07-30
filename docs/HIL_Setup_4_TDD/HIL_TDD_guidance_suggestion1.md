# Recommendation

Use **four separate layers**, each with a narrow responsibility:

| Artifact                 | Purpose                             | Recommendation              |
| ------------------------ | ----------------------------------- | --------------------------- |
| HIL setup definition     | Physical facts about the fixture    | **Add**                     |
| HIL-specific instruction | Mandatory safety and evidence rules | **Add, but keep short**     |
| HIL-TDD skill            | Detailed test procedure             | **Add as a separate skill** |
| TDD/testing agents       | Select and coordinate the procedure | **Add only routing hooks**  |
| Dedicated HIL agent      | Separate operator role              | **Not yet**                 |

This follows the same architecture established during the instruction cleanup: repository instructions should contain short rules that must always apply, while a skill should contain the detailed, specialized procedure loaded only for a relevant task. GitHub explicitly recommends instructions for broadly applicable rules and skills for detailed task-specific workflows. ([GitHub Docs][1])

## Proposed structure

```text
tests/
└── hil/
    ├── README.md
    ├── setups/
    │   ├── pokeys57e-loopback-v1.yaml
    │   └── pokeys57e-loopback-v1.md
    ├── hal/
    │   ├── hil-base.hal
    │   ├── dio-loopback.hal
    │   └── pev2-switch-loopback.hal
    ├── scripts/
    │   ├── preflight.py
    │   ├── safe_reset.py
    │   └── run_hil.py
    └── cases/
        ├── test_dio_loopback.py
        ├── test_pev2_switches.py
        └── test_safe_state.py

.github/
├── instructions/
│   └── hil-tests.instructions.md
└── skills/
    └── hil-tdd/
        ├── SKILL.md
        └── references/
            ├── result-schema.md
            └── test-levels.md
```

# 1. Add a versioned HIL setup definition

The physical fixture must be an authoritative, machine-readable artifact. Do not leave the wiring only as comments inside a HAL file.

Your old configuration already gives a useful basis:

* Pin 23 → Pin 28: X negative limit
* Pin 24 → Pin 29: X home
* Pin 25 → Pin 30: X2 negative limit
* Pin 26 → Pin 31: X2 home
* Pin 12 → Pin 37: Y negative limit
* Pin 13 → Pin 38: Y home
* Pin 14 → Pin 39: Z negative limit
* Pin 15 → Pin 40: Z home
* PWM outputs → analog inputs
* PE/stepper/encoder loopbacks were also considered.

The file also demonstrates an important numbering distinction:

* physical Pin 23 is addressed as `pokeys.0.digout.22.out`;

* physical Pin 12 is addressed as `pokeys.0.digout.11.out`.

The setup definition must therefore store **physical pin number and HAL channel index separately**.

Example:

```yaml
schema_version: 1
setup_id: pokeys57e-loopback-v1
revision: 1

device:
  model: PoKeys57E
  expected_device_id: 27295
  transport: ethernet
  exclusive_use: true

safety:
  motion_allowed: false
  pulse_engine_allowed_by_default: false
  startup_output_state: false
  cleanup_output_state: false
  require_safe_reset_on_exit: true

capabilities:
  - digital-loopback
  - pev2-limit-home
  - pwm-analog-loopback

loopbacks:
  - id: x-limit-negative
    source:
      physical_pin: 23
      hal_pin: pokeys.0.digout.22.out
    sink:
      physical_pin: 28
      observed_hal_pin: pokeys.0.PEv2.0.digin.LimitN.in
    polarity: verify-from-device-configuration

  - id: x-home
    source:
      physical_pin: 24
      hal_pin: pokeys.0.digout.23.out
    sink:
      physical_pin: 29
      observed_hal_pin: pokeys.0.PEv2.0.digin.Home.in
    polarity: verify-from-device-configuration
```

The current machine configuration maps Pin 28 and Pin 29 to X negative-limit and home inputs respectively, and enables inversion for those inputs.  The main HAL configuration exposes the corresponding PEv2 limit/home signals for X, X2, Y and Z.

## Resolve the emergency wiring before including it

There is an apparent inconsistency:

* `Pokeys57E_SimPins.hal` says **Pin 33 → Pin 52** for the pulse-engine emergency input.
* The INI file specifies `PEv2_EmergencyInputPin=54` and `PEv2_EmergencyOutputPin=53`.

Do not include emergency-loopback tests until this has been resolved by:

1. checking the actual physical wiring;
2. confirming whether the numbers are physical pins or API indices;
3. reading the active device configuration;
4. performing a continuity or manually controlled input test.

The old machine files should be treated as **provenance**, not the new HIL fixture’s source of truth.

# 2. Add a separate `hil-tdd` skill

This is the main procedural artifact.

Suggested frontmatter:

```markdown
---
name: hil-tdd
description: >
  Run Red-Green-Refactor development against a registered PoKeys hardware-in-the-loop
  fixture. Use for changes whose acceptance criteria depend on real PoKeys hardware,
  firmware, HAL pin propagation, PEv2 behavior, electrical loopbacks, or measured timing.
---
```

The skill should contain the complete workflow.

## HIL-TDD workflow

### A. Classify the requirement

Determine whether the behavior requires:

1. unit testing;
2. protocol/parser testing;
3. userspace HAL integration;
4. HIL verification;
5. RT validation;
6. timing measurement.

HIL must not replace unit or protocol tests. It is an additional outer verification loop.

### B. Select the fixture

Require an explicit setup ID:

```text
pokeys57e-loopback-v1
```

Verify:

* device model;
* device ID or serial;
* fixture revision;
* firmware compatibility;
* expected wiring;
* declared capability needed by the test.

### C. Acquire and preflight

Before touching outputs:

* acquire an exclusive fixture lock;
* verify that the expected PoKeys device is connected;
* confirm no production machine or actuators are attached;
* confirm all outputs are in the documented safe state;
* verify the HAL pin names required by the setup;
* reject the test if the setup does not match.

### D. Red

For a hardware-dependent defect:

* add the HIL test first;
* run it against the unmodified implementation;
* record the expected failure;
* confirm it fails because of the target behavior, not because the fixture is unavailable or miswired.

A missing device, failed preflight or setup mismatch is **not a red test**. It is an unavailable test environment.

### E. Green

Implement the smallest production change.

Then run:

1. narrow unit/protocol test;
2. userspace integration test;
3. relevant HIL case.

### F. Refactor

Keep both software-only and HIL tests green.

### G. Cleanup

Whether the test passes, fails or is interrupted:

* drive all source outputs to their safe state;
* disable PEv2 operations started by the test;
* unload test HAL components;
* release the fixture lock;
* report whether cleanup succeeded.

### H. Evidence

Every HIL result should record:

```text
commit SHA
setup ID and revision
device model and device ID
firmware version
LinuxCNC version
kernel / RT mode
HAL configuration hash
test command
test result
observed input/output transitions
cleanup result
```

Use the status term **HIL-verified** only when the named setup and test actually ran successfully. HIL verification does not by itself imply RT validation or timing validation.

# 3. Add a short path-specific HIL instruction

Create:

```text
.github/instructions/hil-tests.instructions.md
```

Suggested scope:

```yaml
---
description: "Mandatory safety and integrity rules for PoKeys hardware-in-the-loop tests."
applyTo: "tests/hil/**"
---
```

Keep it around 30–50 lines. It should contain only non-negotiable rules:

```markdown
# PoKeys HIL Test Invariants

- Every HIL test must reference a registered setup ID and revision.
- Never run against a device whose model or identity does not match the selected setup.
- Acquire exclusive access to the fixture before changing outputs.
- Establish and verify the safe output state before and after every test.
- Tests must manipulate only outputs declared by the setup capability.
- Motion, pulse-engine and emergency tests are disabled unless explicitly declared safe by the setup.
- A missing device, failed preflight or unavailable runner is SKIPPED/NOT RUN, never PASS.
- HIL tests must remain independently repeatable and must reset device/HAL state between cases.
- Record exact hardware, firmware, HAL configuration and command evidence.
- HIL-verified does not mean RT-validated or timing-validated.
- Do not add timing thresholds without a measured baseline and documented tolerance.
```

The existing general test instruction already distinguishes hardware-required tests from userspace-only tests and forbids claiming RT validation from userspace tests.  The HIL-specific instruction should extend that only for `tests/hil/**`; it should not inflate the context for ordinary tests.

# 4. Do not append the procedure to the current TDD agents

The current `TDDDriver` is still largely a generic web-application agent. Its examples use TypeScript, npm, JWT authentication and database-backed services.

The `TestingSpecialist` has the same problem: it contains Jest, Cypress, Playwright, Java and database examples, and currently says tests must not require manual setup or external services.

Adding a large HIL chapter to either agent would recreate the context-accumulation problem just corrected elsewhere.

Instead, add only a small routing block to each:

```markdown
## Hardware-dependent testing

When acceptance criteria depend on actual PoKeys hardware, firmware, physical I/O,
PEv2 signal propagation, analog conversion, encoder feedback, or measured timing:

1. Use the `hil-tdd` skill.
2. Select a registered HIL setup by setup ID.
3. Keep software-only unit and protocol tests; HIL does not replace them.
4. Never report an unavailable or skipped HIL test as passing.
5. Do not operate hardware outside the capabilities declared by the selected setup.
```

The larger PoKeys-specific cleanup of those two agents should be a separate task.

# 5. No dedicated HIL agent yet

A custom agent is appropriate when it has a distinct role, tool set or access boundary. GitHub custom agents primarily define a specialized role and tools, whereas skills package repeatable procedural knowledge. ([GitHub Docs][2])

At present:

* `TDDDriver` can execute commands;
* `TestingSpecialist` can execute tests;
* the HIL distinction is procedural and fixture-specific.

Therefore a skill is sufficient.

Create a dedicated `HILTestOperator` agent later only when the rig gains capabilities such as:

* SSH access to a dedicated LinuxCNC test host;
* controlled PoKeys power cycling;
* bench reservation or locking service;
* oscilloscope or logic-analyzer access;
* emergency-stop supervision;
* restricted permission to operate a dedicated fixture.

# Recommended test layers

## Layer 1 — Direct digital loopback

Do not initially load the complete mill configuration.

Use a minimal HAL configuration that:

1. loads `pokeys_async`;
2. configures one declared output;
3. reads the linked input;
4. toggles low/high/low;
5. checks propagation and cleanup.

Initial tests:

```text
test_each_output_reaches_only_its_declared_input
test_low_high_low_round_trip
test_inverted_input_reports_expected_logical_state
test_no_other_input_changes
test_outputs_return_to_safe_state_after_failure
```

## Layer 2 — PEv2 limit/home mapping

Verify that an output transition travels through:

```text
HAL digout
→ physical PoKeys output
→ loopback wire
→ physical PoKeys input
→ PEv2 limit/home state
→ exported HAL PEv2 pin
```

The existing machine HAL already maps PEv2 limit/home states for X, X2, Y and Z, so this is a credible first end-to-end HIL target.

Test:

* limit inactive → active → inactive;
* home inactive → active → inactive;
* configured inversion;
* filter/debounce behavior;
* wrong channel does not toggle;
* repeated cycles do not leave stale state.

## Layer 3 — Window-comparator machine simulation

Only after Layers 1 and 2 pass should the `wcomp` approach be reused.

The old simulation drives physical digital outputs from PEv2 current positions using window comparators.  This is useful for testing homing and limit behavior, but it combines several systems:

* PEv2 position reporting;
* conversion components;
* window comparators;
* output export;
* physical loopback;
* input interpretation;
* LinuxCNC motion behavior.

Keeping this as Layer 3 makes failures diagnosable.

## Later layers

Add separately:

* PWM → analog input calibration and tolerance;
* pulse output → encoder/fast-encoder feedback;
* disconnect/reconnect and stale-state handling;
* retry/timeout behavior with real hardware;
* RT-cycle and latency measurements;
* emergency-loop behavior after the current pin discrepancy is resolved.

# First implementation PR

The initial HIL PR should remain narrow:

1. add `pokeys57e-loopback-v1.yaml`;
2. add a human-readable fixture diagram;
3. add preflight, exclusive lock and safe-reset scripts;
4. add one minimal direct DIO loopback test;
5. add `hil-tests.instructions.md`;
6. add the `hil-tdd` skill;
7. add only routing paragraphs to the two test agents;
8. document that emergency, analog and encoder tests remain out of scope.

That gives you a usable HIL-TDD foundation without coupling the first iteration to the entire DM542 mill configuration.

[1]: https://docs.github.com/en/copilot/concepts/agents/about-agent-skills?utm_source=chatgpt.com "About agent skills - GitHub Docs"
[2]: https://docs.github.com/en/copilot/how-tos/copilot-on-github/customize-copilot/customize-cloud-agent/create-custom-agents?utm_source=chatgpt.com "Creating custom agents for Copilot cloud agent - GitHub Docs"
