Yes. The existing `Pokeys57E_SimPins.hal` is a good HIL fixture. It already documents the physical loopbacks and uses `wcomp` comparators to turn simulated axis positions into physical PoKeys output pins, which are wired back into PoKeys inputs. That means you can test the full chain:

```text
LinuxCNC HAL command
  -> pokeysHal output pin
  -> physical PoKeys57E output
  -> wire loopback
  -> physical PoKeys57E input
  -> pokeysHal PEv2/input pin
  -> LinuxCNC HAL/motion-facing signal
```

The existing config uses `DEVICE_ID=27295` and `ApplyIniSettings=1`, so the HIL profile should lock to that board unless explicitly overridden.  The machine config loads `Pokeys_DM542_XXYZ_mill.hal`, `pokeys_homing.hal`, and then `Pokeys57E_SimPins.hal`, so the current setup already treats the sim pins as part of the HAL stack.

## Key correction: HIL-TDD is not the inner TDD loop

Use this structure:

```text
1. Unit TDD
   Fast, no hardware, every commit/PR.

2. HAL integration tests
   LinuxCNC/halrun, no real PoKeys device where possible.

3. HIL acceptance tests
   PoKeys57E wired loopback fixture, manual/self-hosted CI only.

4. Full motion/homing tests
   Only when drivers/motors are safe or disconnected.
```

So the rule should be:

```text
Every production behavior change must have a fast test.
Every behavior involving real PoKeys I/O must additionally have a HIL contract test.
HIL failure proves the feature is not hardware-verified.
HIL success does not replace unit tests.
```

This matches your repo’s standing constraints: no blocking calls, no dynamic allocation, no unbounded loops in RT paths, and no timing claims without measurement.

---

## HIL fixture map to document

Use this as the canonical HIL board wiring table.

| Function            | Physical output | Physical input | HAL output used by sim file | Expected logical input            |
| ------------------- | --------------: | -------------: | --------------------------- | --------------------------------- |
| X min               |          Pin 23 |         Pin 28 | `pokeys.0.digout.22.out`    | `pokeys.0.PEv2.0.digin.LimitN.in` |
| X home              |          Pin 24 |         Pin 29 | `pokeys.0.digout.23.out`    | `pokeys.0.PEv2.0.digin.Home.in`   |
| X2 min              |          Pin 25 |         Pin 30 | `pokeys.0.digout.24.out`    | `pokeys.0.PEv2.6.digin.LimitN.in` |
| X2 home             |          Pin 26 |         Pin 31 | `pokeys.0.digout.25.out`    | `pokeys.0.PEv2.6.digin.Home.in`   |
| Y min               |          Pin 12 |         Pin 37 | `pokeys.0.digout.11.out`    | `pokeys.0.PEv2.1.digin.LimitN.in` |
| Y home              |          Pin 13 |         Pin 38 | `pokeys.0.digout.12.out`    | `pokeys.0.PEv2.1.digin.Home.in`   |
| Z min               |          Pin 14 |         Pin 39 | `pokeys.0.digout.13.out`    | `pokeys.0.PEv2.2.digin.LimitN.in` |
| Z home              |          Pin 15 |         Pin 40 | `pokeys.0.digout.14.out`    | `pokeys.0.PEv2.2.digin.Home.in`   |
| PE emergency        |          Pin 33 |         Pin 52 | PEv2 emergency output       | PEv2 emergency input              |
| PWM/analog loopback |      Pins 17–20 |     Pins 41–44 | PWM/adcout                  | analog input                      |
| PE feedback         |         PE1/PE3 | FastEncoder1/2 | step pulse output           | encoder feedback                  |

The physical wiring is explicitly documented in `Pokeys57E_SimPins.hal`.  The output HAL index is **physical pin minus one** in the existing file: for example physical Pin 23 is driven through `pokeys.0.digout.22.out`, and physical Pin 24 through `pokeys.0.digout.23.out`.

Important caveat: labels like `X max/home` in the old comment should not be treated as active positive limit tests unless the INI enables positive limits. In the current INI, X uses Pin 28 as limit-minus and Pin 29 as home, while positive limit is disabled/set to `0`.  X2 follows the same pattern with Pin 30 and Pin 31.  Y and Z also use limit-minus plus home, not positive limit switches.

---

## Proposed repository addition

Do **not** put the whole HIL procedure into always-loaded `AGENTS.md`. Add only a routing note there. Put the detailed contract in a dedicated instruction file:

```text
.github/instructions/hil-tdd.instructions.md
docs/testing/hil-pokeys57e-loopback.md
tests/hil/
  README.md
  hal/
    pokeys57e_hil_loopback.hal
    pokeys57e_hil_simpins.hal
  pytest/
    test_hil_identity.py
    test_hil_digital_loopback.py
    test_hil_simpins_switches.py
    test_hil_estop_loopback.py
    test_hil_analog_pwm_loopback.py
```

Add this routing line to `AGENTS.md` or root Copilot instructions:

```markdown
For HIL work against the PoKeys57E loopback fixture, apply `.github/instructions/hil-tdd.instructions.md` and `docs/testing/hil-pokeys57e-loopback.md`. HIL tests are hardware-verification tests and must not replace host unit tests.
```

---

## Draft `.github/instructions/hil-tdd.instructions.md`

```markdown
---
description: PoKeys57E hardware-in-the-loop TDD rules for loopback fixture tests.
applyTo: "tests/hil/**,docs/testing/**,*.hal,*.ini,.github/workflows/**"
---

# PoKeys57E HIL-TDD Contract

## Purpose

The PoKeys57E HIL fixture verifies real hardware behavior through a fixed loopback wiring harness. It is used to validate that PoKeysHal exports the expected HAL pins, applies the expected pin functions, drives physical outputs, observes physical inputs, and preserves PEv2 switch semantics.

HIL tests are acceptance/verification tests. They do not replace host unit tests.

## Fixture identity

Default HIL device:

- Device: PoKeys57E
- Default `DEVICE_ID`: `27295`
- Expected setup: digital outputs wired to matching digital inputs as documented in `docs/testing/hil-pokeys57e-loopback.md`
- The test runner must require `POKEYS_HIL=1`
- The test runner must fail closed if the board is missing, has a different ID, or the wiring self-test fails

## TDD rules

For behavior touching real device I/O:

1. Add or update a failing unit test first where practical.
2. Add or update a HIL contract test for the real hardware behavior.
3. Make the smallest implementation change.
4. Run fast unit tests before HIL.
5. Run HIL only on a machine explicitly marked as having the PoKeys57E fixture.
6. Report status precisely:
   - `Compiled`
   - `Unit-tested`
   - `HAL-smoke-tested`
   - `HIL-tested`
   - `RT-timing-validated`

Never claim `HIL-tested` from a mock, simulator, screenshot, or static review.

## Safety rules

HIL tests must not drive an actual CNC machine, spindle, cutter, relay, or stepper drive unless the test is explicitly classified as `full-motion-hil`.

Default HIL mode is `loopback-safe`:

- Motors disconnected or disabled
- Spindle output disconnected or dummy-loaded
- Emergency loopback verified before any motion-related test
- Tests must restore outputs to inactive state on teardown
- Tests must stop LinuxCNC/HAL cleanly on failure

## Required HIL test order

The test suite must run in this order:

1. `HIL-000 identity`
   - Confirm PoKeys device is reachable
   - Confirm expected device ID
   - Confirm expected HAL pins are exported

2. `HIL-010 output/input loopback continuity`
   - Set each documented output inactive/active/inactive
   - Verify the matching physical input changes
   - Verify logical PEv2 input semantics separately from raw electrical state

3. `HIL-020 simulated limit/home switches`
   - Load the window comparator simulation
   - Drive the simulated position source across threshold windows
   - Verify comparator output
   - Verify physical loopback input
   - Verify PEv2 logical limit/home input

4. `HIL-030 emergency loopback`
   - Assert emergency output-to-input path
   - Verify fail-safe inactive state
   - Verify active emergency state

5. `HIL-040 analog/PWM loopback`
   - Drive PWM/adcout values
   - Verify analog input response within tolerance
   - Record tolerance and settling time

6. `HIL-050 pulse/encoder loopback`
   - Drive PE step output in a bounded test
   - Verify fast encoder count changes in expected direction
   - Do not run with real motors unless explicitly enabled

## Required evidence

Each HIL run must record:

- Git commit SHA
- PoKeys device ID
- LinuxCNC version
- Kernel type
- HAL file used
- Wiring profile name
- Passed/failed test IDs
- Any skipped tests and reason
- Measured timeout/settling values where applicable

## Prohibited shortcuts

Do not:

- Use sleeps as the only synchronization method where HAL pin polling is possible
- Ignore inverted limit/home semantics
- Treat physical pin numbers and HAL channel indexes as identical
- Run HIL tests in normal GitHub-hosted CI
- Modify production HAL files only to make HIL tests pass
- Leave outputs active after test failure
```

---

## HIL tests to write first

### `HIL-000`: board identity and HAL export

Use a minimal HIL INI/HAL profile. Confirm:

```text
pokeys.0 exists
pokeys.0.PEv2.0.digin.LimitN.in exists
pokeys.0.PEv2.0.digin.Home.in exists
pokeys.0.PEv2.6.digin.LimitN.in exists
pokeys.0.PEv2.6.digin.Home.in exists
pokeys.0.PEv2.1.digin.LimitN.in exists
pokeys.0.PEv2.1.digin.Home.in exists
pokeys.0.PEv2.2.digin.LimitN.in exists
pokeys.0.PEv2.2.digin.Home.in exists
```

The existing HAL loads `pokeys` as a userspace component with `loadusr -W pokeys`.  The current main HAL reads the PEv2 limit/home inputs for X, X2, Y, and Z from the PoKeys component.

### `HIL-010`: direct digital loopback

For each loopback pair:

```text
set output inactive
wait until matching input inactive
set output active
wait until matching input active
set output inactive
wait until matching input inactive
```

Use raw input where available, then logical PEv2 input. Keep these separate because the INI enables inverted limit/home semantics.

This catches:

```text
wrong HAL pin number
wrong PinFunction
wrong output write command
wrong input parser
wrong PEv2 switch mapping
wrong inversion handling
async update starvation
```

### `HIL-020`: window-comparator switch simulation

`Pokeys57E_SimPins.hal` loads eight `wcomp` comparators and adds them to the servo thread.  It feeds the comparators from `pokeys.0.PEv2.*.CurrentPosition` through `conv_s32_float`.  It then drives the simulated switch outputs into the physical output pins wired back to the board.

Use the existing thresholds as the first contract:

```text
X min:     -10000 .. -6000
X home:    680000 .. 700000
X2 min:   -10000 .. -6000
X2 home:   680000 .. 700000
Y min:    -10000 .. -6000
Y home:    560000 .. 580000
Z min:     -8000 .. -6000
Z home:    168001 .. 180000
```

Those are counter/window values, not necessarily millimetres. They come directly from the sim file.

### `HIL-030`: PEv2 homing/status path

`pokeys_homing.hal` connects PEv2 axis state and command pins between `pokeys.0.PEv2.*` and LinuxCNC joint-side PEv2 pins.  It also sets the PoKeys PEv2 home sequence values from the joint INI sections.

So HIL tests should check two levels:

```text
driver-level:
  pokeys.0.PEv2.N.AxesState / AxesCommand / CurrentPosition

machine-level:
  joint.N.PEv2.AxesState / AxesCommand / home-sequence
```

Do not assume the normal LinuxCNC `joint.N.home-sw-in`/`neg-lim-sw-in` path is active unless the specific HIL HAL file connects it. In the main HAL, several normal limit/home net connections are commented out.

---

## CI/workflow rule

Add a manual/self-hosted workflow only:

```yaml
name: PoKeys57E HIL

on:
  workflow_dispatch:
    inputs:
      device_id:
        description: "PoKeys device ID"
        required: true
        default: "27295"

jobs:
  hil:
    runs-on: [self-hosted, linuxcnc, pokeys57e-hil]
    env:
      POKEYS_HIL: "1"
      POKEYS_DEVICE_ID: ${{ inputs.device_id }}
    steps:
      - uses: actions/checkout@v4
      - name: Build
        run: sudo make -f Makefile.noqmake install
      - name: Run HIL tests
        run: pytest -m hil tests/hil --junitxml=hil-results.xml
      - name: Stop HAL safely
        if: always()
        run: |
          halcmd stop || true
          halcmd unload all || true
```

Do **not** run this on normal GitHub-hosted CI. HIL requires the physical PoKeys57E fixture.

---

## Practical TDD workflow for a bug/fix

Use this exact loop:

```text
1. Create/identify issue:
   REQ-HIL-xxx or BUG-HIL-xxx

2. Add fast failing unit test:
   parser, async transaction, HAL pin export, timeout, inversion logic

3. Add failing HIL contract test:
   proves the behavior on the PoKeys57E loopback fixture

4. Implement smallest fix.

5. Run:
   unit tests
   halcompile / halrun smoke
   HIL test on self-hosted fixture

6. PR evidence:
   Unit-tested: yes/no
   HAL-smoke-tested: yes/no
   HIL-tested: yes/no
   RT-timing-validated: yes/no
```

For this repo, the strongest first PR would be **documentation + skeleton only**:

```text
.github/instructions/hil-tdd.instructions.md
docs/testing/hil-pokeys57e-loopback.md
tests/hil/README.md
.github/workflows/hil-pokeys57e.yml
```

Then the second PR should add `HIL-000` and `HIL-010` before touching driver behavior.
