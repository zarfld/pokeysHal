Yes — that is the better separation.

The **HIL-TDD fixture** should test the PoKeysHal driver against the **wired hardware primitives** only:

```text
digital output -> physical wire -> digital input
PWM/DAC output -> physical wire -> analog input
pulse engine output -> stepper/encoder path -> fast encoder input
emergency output -> physical wire -> emergency input
```

The full `DM542_XXYZ_mill` LinuxCNC configuration should be treated as a **machine integration test**, not as the normal HIL-TDD loop.

## Revised test layering

| Layer                     | Purpose                                                                     | Uses full machine config? | Uses PoKeys57E hardware? |
| ------------------------- | --------------------------------------------------------------------------- | ------------------------: | -----------------------: |
| Unit TDD                  | Parser, async mailbox, HAL export, inversion logic                          |                        No |                       No |
| HAL smoke                 | `halcompile`, `halrun`, exported pins                                       |                        No |                 Optional |
| **HIL-TDD basic fixture** | Real digin/digout/PWM/ADC/PE/encoder behavior                               |                    **No** |                  **Yes** |
| Machine integration       | Full `DM542_XXYZ_mill` config, homing, sim switches, LinuxCNC motion wiring |                   **Yes** |                      Yes |
| Full motion validation    | Motors/drives/spindle connected                                             |                       Yes |                      Yes |

The existing `Pokeys57E_SimPins.hal` is useful evidence of the wiring harness: it documents the loopbacks for digital switch simulation, PWM-to-analog, and pulse-engine-to-encoder paths.  But its `wcomp` logic and position-derived simulated switches are already **machine-behavior simulation**, not the minimal primitive HIL test.

## Correct HIL-TDD principle

Use this rule:

```text
HIL-TDD tests the driver’s hardware-facing contracts with the smallest HAL setup possible.
Full LinuxCNC machine configs test integration, not primitive driver behavior.
```

So for HIL-TDD, do **not** load:

```text
motmod
trivkins / gantry kinematics
joint.N motion nets
pokeys_homing.hal
DM542_XXYZ_mill machine HAL
full AXIS UI config
```

Unless the test explicitly belongs to `integration/`.

The full config currently loads `Pokeys_DM542_XXYZ_mill.hal`, `pokeys_homing.hal`, and `Pokeys57E_SimPins.hal`; that makes it a multi-file machine configuration, therefore integration scope.

---

## Proposed split

### `tests/hil/basic/`

This is the proper HIL-TDD target.

```text
tests/hil/basic/
  README.md
  hal/
    pokeys57e_basic_load.hal
    pokeys57e_digital_loopback.hal
    pokeys57e_pwm_adc_loopback.hal
    pokeys57e_pe_encoder_loopback.hal
    pokeys57e_emergency_loopback.hal
  pytest/
    test_000_identity.py
    test_010_digital_io_loopback.py
    test_020_pwm_adc_loopback.py
    test_030_pe_encoder_loopback.py
    test_040_emergency_loopback.py
```

### `tests/integration/linuxcnc/`

This is where the full `DM542_XXYZ_mill` profile belongs.

```text
tests/integration/linuxcnc/
  DM542_XXYZ_mill/
    Pokeys57E_DM542_XXYZ_mill.ini
    Pokeys_DM542_XXYZ_mill.hal
    pokeys_homing.hal
    Pokeys57E_SimPins.hal
  pytest/
    test_machine_config_loads.py
    test_machine_homing_simulation.py
    test_machine_limit_switch_simulation.py
```

---

## HIL-TDD fixture contract

The primitive fixture should use the documented wiring as a **board capability map**.

| HIL contract         | Wiring from existing fixture                                                     | What it verifies                                           |
| -------------------- | -------------------------------------------------------------------------------- | ---------------------------------------------------------- |
| Digital output/input | Pin 23→28, 24→29, 25→30, 26→31, 12→37, 13→38, 14→39, 15→40                       | `digout` write path, physical output, input read path      |
| Emergency loopback   | Pin 33→52                                                                        | PEv2 emergency output/input path                           |
| PWM/DAC to ADC       | Pin 17→41, 18→42, 19→43, 20→44                                                   | PWM/adcout scaling, analog input parser, settling behavior |
| Motion feedback      | PE1→PoStep→Stepper→Encoder→FastEncoder1, PE3→PoStep→Stepper→Encoder→FastEncoder2 | pulse generation, direction, encoder count feedback        |

The physical mappings are already documented in the sim file.

---

## Basic HIL tests

### `HIL-000`: identity/load test

Goal: prove the HIL runner is connected to the expected PoKeys57E.

Checks:

```text
load PoKeysHal component
confirm device present
confirm expected DEVICE_ID
confirm expected HAL pin namespace exists
confirm all configured basic test pins exist
stop HAL cleanly
```

This should use a minimal HAL file, not the full machine INI.

---

### `HIL-010`: digital loopback

For each wired pair:

```text
set output inactive
wait for input inactive
set output active
wait for input active
set output inactive
wait for input inactive
```

Example contract:

```text
physical Pin 23 -> physical Pin 28
HAL output: pokeys.0.digout.22.out
expected input: raw pin 28 input and/or PEv2 X LimitN input
```

The existing sim file already drives physical Pin 23 through `pokeys.0.digout.22.out`, confirming the HAL index convention used in that config.

For primitive HIL, first test raw digital input if exported. Then separately test PEv2 semantic mapping such as `LimitN`, `Home`, or emergency.

---

### `HIL-020`: PWM/DAC to ADC loopback

For each PWM/adcout-to-analog-input pair:

```text
set analog output enable true
set output value low
wait until analog input near low value
set output value mid
wait until analog input near mid value
set output value high
wait until analog input near high value
restore output to safe value
```

Test assertions should be tolerant, not exact:

```text
ADC observed value must be monotonic
ADC value must settle within configured tolerance
ADC value must not exceed configured limits
```

Do not use the full spindle scaling here. The full machine HAL maps spindle speed through `scale.0` into `pokeys.0.adcout.0.value`; that belongs in integration testing.

---

### `HIL-030`: pulse-engine to fast-encoder loopback

Primitive motion HIL should not test LinuxCNC homing yet. It should test:

```text
enable bounded pulse output
command small positive move / pulse count
confirm encoder count increases
command small negative move / reversed direction if supported
confirm encoder count decreases or changes according to expected direction
disable pulse output
```

This verifies the PoKeys motion I/O path without requiring the full `XXYZ` machine config.

Full LinuxCNC joint wiring — for example `joint.0.motor-pos-cmd` into `pokeys.0.PEv2.0.joint-pos-cmd` and PoKeys feedback back into `joint.0.motor-pos-fb` — belongs in integration tests.

---

### `HIL-040`: emergency loopback

Primitive test:

```text
set emergency output inactive
verify emergency input safe/inactive
set emergency output active
verify emergency input active
restore inactive state
```

Machine integration can then verify the higher-level LinuxCNC E-stop chain, where `iocontrol.0.user-enable-out` is connected to `pokeys.0.PEv2.digout.Emergency.out` and the emergency input feeds `iocontrol.0.emc-enable-in`.

---

## Integration test scope for the existing config

The current `DM542_XXYZ_mill` setup should test things like:

```text
full HAL file load order
INI parameter propagation
PEv2 axis switch config
homing algorithm config
wcomp-based simulated limits
X/X2/Y/Z joint mappings
machine-on / estop / probe routing
spindle DAC scaling
```

That is valuable, but it is too broad for TDD. A failure there could come from:

```text
driver bug
HAL naming change
INI mistake
LinuxCNC version difference
machine config bug
homing component bug
wrong physical wiring
wrong simulation threshold
```

In HIL-TDD basic tests, a failure should point to one subsystem with minimal ambiguity.

---

## Update to the instruction file

I would change the earlier HIL-TDD instruction like this:

```markdown
## Scope distinction

HIL-TDD basic tests use the PoKeys57E loopback wiring as a primitive hardware fixture.

Basic HIL tests must verify:
- digital output to digital input loopback
- PWM/adcout to analog input loopback
- PE output to fast encoder feedback path
- emergency output to emergency input loopback
- HAL pin export and pin-function application

Basic HIL tests must not require:
- full LinuxCNC machine configuration
- motmod
- kinematics
- AXIS UI
- homing state machine
- full `DM542_XXYZ_mill` HAL/INI stack

The full `DM542_XXYZ_mill` configuration is an integration test fixture. It may reuse the same physical wiring, but it must live under `tests/integration/`, not under the primitive HIL-TDD test suite.
```

## Concrete next step

Create the first PR as **documentation and structure only**:

```text
.github/instructions/hil-tdd.instructions.md
docs/testing/pokeys57e-hil-fixture.md
tests/hil/basic/README.md
tests/integration/linuxcnc/README.md
```

Then add `HIL-000` and `HIL-010`. Do not start with the full `DM542_XXYZ_mill` config; start with direct digital loopback because it gives the cleanest signal on driver correctness.
