---
name: hil-tdd
description: >
  Run Red-Green-Refactor development against a registered PoKeys hardware-in-the-loop
  fixture. Use for changes whose acceptance criteria depend on real PoKeys hardware,
  firmware, HAL pin propagation, PEv2 behavior, electrical loopbacks, or measured timing.
---

# HIL-TDD Skill

## When to use

Use this skill when the acceptance criterion for a behavior requires:

- real PoKeys57E digital I/O (not a mock or simulator);
- firmware-level PEv2 switch mapping (LimitN, Home, emergency);
- physical loopback electrical verification;
- PWM/ADC analog path confirmation;
- async update latency with real USB/Ethernet round-trips;
- RT-cycle timing measurement against a defined threshold.

Do not use HIL as a substitute for unit tests. Unit tests must exist first.

---

## Step A — Classify the requirement

Determine which test layers apply:

1. Unit test — parser, mailbox, HAL export, inversion logic (always required)
2. Protocol test — async send/parse/retry coverage (always required)
3. HAL smoke — `halcompile`/`halrun`, exported pins visible
4. Primitive HIL — real board I/O, minimal HAL config, no machine stack
5. Machine integration — full `DM542_XXYZ_mill` config, homing, `wcomp`
6. RT validation — timing measured in RT environment with documented threshold

HIL (layer 4) does not replace layers 1–3.

---

## Step B — Select the fixture

Specify the setup ID explicitly:

```text
pokeys57e-loopback-v1
```

Verify before proceeding:

- device model matches setup definition
- device ID (expected: `27295`) matches
- fixture revision matches
- firmware version is compatible
- declared capability covers the test's requirement
- required HAL pin names are present in the setup definition

The setup definition is at:
`tests/hil/setups/pokeys57e-loopback-v1.yaml`

---

## Step C — Acquire and preflight

Before touching any output:

1. Acquire exclusive fixture lock (prevent parallel test runs).
2. Verify the expected PoKeys device is reachable.
3. Confirm no production machine or actuators are attached.
4. Drive all outputs to their safe state (false/inactive).
5. Read back and confirm all outputs are in the safe state.
6. Verify that every HAL pin name required by the test exists.
7. Reject the run if device identity or wiring self-check fails.

A failed preflight is `SKIPPED/NOT RUN`, never `FAIL` and never `PASS`.

---

## Step D — Red phase (HIL-assisted TDD)

For hardware-dependent behavior:

1. Run a narrow HIL probe to observe real board behavior.
2. Record the observation in `docs/testing/pokeys57e-hil-observations.md` with an observation ID.
3. Write a failing unit test using that observed contract as the oracle.
4. Write a failing HIL contract test.
5. Confirm the unit test fails for the target reason, not infrastructure absence.

A missing device, failed preflight, or setup mismatch is not a red test — it is an
unavailable test environment.

### HIL-derived unit test oracle comment format

```c
/* Oracle: HIL-P57E-DIO-001, observed on PoKeys57E DEVICE_ID=27295 */
```

### Unit test oracle requirements

Unit tests for hardware-facing behavior must cite an oracle from one of:

- official PoKeys/PoLabs protocol specification or manual
- current upstream `pokeyslib` behavior
- existing verified LinuxCNC machine configuration
- a recorded HIL observation (preferred for pin mapping, polarity, PEv2 semantics)

Do not invent expected values from developer assumption.

---

## Step E — Green phase

Implement the smallest production change.

Then run in order:

1. Narrow unit/protocol test suite.
2. `halcompile` + `halrun` smoke test.
3. Relevant HIL primitive test case.

Do not run the full machine integration config until the primitive HIL test passes.

---

## Step F — Refactor

Keep all three layers green: unit tests, HAL smoke, HIL primitive.

---

## Step G — Cleanup

Whether the test passes, fails, or is interrupted:

1. Drive all source outputs to their safe state (false/inactive).
2. Disable any PEv2 operations started by the test.
3. Unload test HAL components (`halcmd unload all`).
4. Release the fixture lock.
5. Report whether cleanup succeeded.

A test that leaves outputs active is a defect in the test itself.

---

## Step H — Evidence

Every HIL run must record:

```text
commit SHA
setup ID and revision
device model and device ID
firmware version
LinuxCNC version
kernel / RT mode
HAL configuration and hash
test command
test result (PASS / FAIL / SKIP and reason)
observed input/output transitions
cleanup result
```

Use `HIL-verified` only when the named setup and test actually ran to completion.
`HIL-verified` does not imply RT-validated or timing-validated.

---

## Test layer structure

```text
tests/hil/basic/          — primitive fixture tests (no machine stack)
  hal/                    — minimal HAL files for each test category
  pytest/                 — pytest test cases
  README.md               — scope and boundary contract

tests/integration/linuxcnc/   — machine-integration tests
  DM542_XXYZ_mill/            — full INI/HAL stack, wcomp switches, homing
  pytest/
  README.md
```

---

## HIL test sequence — primitive fixture

Tests must execute in this order. Each builds on the previous.

### HIL-000 — Identity and HAL export

Goal: prove the runner is connected to the expected device and all required pins exist.

- Load a minimal HAL config (not the full machine INI).
- Confirm `pokeys.0` component is loaded.
- Confirm expected device ID is reported.
- Confirm all HAL pins required by the declared loopback pairs are present.
- Unload cleanly.

A HAL pin name mismatch here invalidates all subsequent tests.

### HIL-010 — Direct digital loopback

For each declared loopback pair (e.g., Pin 23 → Pin 28):

```text
set output inactive
wait until matching input is inactive
set output active
wait until matching input is active
set output inactive
wait until matching input is inactive
```

Test raw digital input state first. Test PEv2 logical input (LimitN/Home) separately,
because the INI enables inversion for several inputs.

Catches: wrong HAL pin number, wrong PinFunction, wrong output write command,
wrong input parser, wrong PEv2 switch mapping, wrong inversion, async starvation.

### HIL-020 — PWM/ADC loopback

Requires RC low-pass filter at each connection (4.7 kΩ + 1 µF recommended).
Without the filter, the ADC sees an unstable square wave.

For each PWM/adcout → analog input pair:

```text
set adcout enable true
set value to low
wait for ADC to settle near low
set value to mid
wait for ADC to settle near mid
set value to high
wait for ADC to settle near high
restore to safe value
```

Assertions must be tolerant (monotonic, within ±10% tolerance, not exact).
Do not load spindle scaling here — that belongs in integration.

### HIL-030 — Pulse engine to fast encoder

Test the PE → encoder path without LinuxCNC `motmod`:

```text
enable bounded pulse output
command small positive pulse count
confirm fast encoder count increases
command reverse pulse count
confirm fast encoder count decreases (or changes as expected per direction)
disable pulse output
```

Do not wire `joint.N.motor-pos-cmd` here. Joint wiring belongs in integration.

### HIL-040 — Emergency loopback

**Status: EXCLUDED from v1 scope** pending resolution of hardware questions HW-1 and
HW-2 (Pin 33 → Pin 52 wiring vs `PEv2_EmergencyInputPin=54` in INI). Do not
implement until the discrepancy is physically confirmed.

When unblocked:
```text
set emergency output inactive
verify emergency input safe/inactive
set emergency output active
verify emergency input active
restore inactive state
```

---

## PIN NUMBERING INVARIANT

Physical pin N on PoKeys57E = HAL channel index N−1.

Examples confirmed from `Pokeys57E_SimPins.hal`:
- Physical Pin 23 → `pokeys.0.digout.22.out`
- Physical Pin 12 → `pokeys.0.digout.11.out`

The fixture YAML stores physical pin numbers and HAL channel indices separately.
Never treat them as identical.

---

## Declared loopback pairs (v1 fixture)

| Function | Physical output | Physical input | HAL output | Expected logical input |
|---|---|---|---|---|
| X min | 23 | 28 | `pokeys.0.digout.22.out` | `pokeys.0.PEv2.0.digin.LimitN.in` |
| X home | 24 | 29 | `pokeys.0.digout.23.out` | `pokeys.0.PEv2.0.digin.Home.in` |
| X2 min | 25 | 30 | `pokeys.0.digout.24.out` | `pokeys.0.PEv2.6.digin.LimitN.in` |
| X2 home | 26 | 31 | `pokeys.0.digout.25.out` | `pokeys.0.PEv2.6.digin.Home.in` |
| Y min | 12 | 37 | `pokeys.0.digout.11.out` | `pokeys.0.PEv2.1.digin.LimitN.in` |
| Y home | 13 | 38 | `pokeys.0.digout.12.out` | `pokeys.0.PEv2.1.digin.Home.in` |
| Z min | 14 | 39 | `pokeys.0.digout.13.out` | `pokeys.0.PEv2.2.digin.LimitN.in` |
| Z home | 15 | 40 | `pokeys.0.digout.14.out` | `pokeys.0.PEv2.2.digin.Home.in` |
| PWM/ADC | 17–20 | 41–44 | `pokeys.0.adcout.N.value` | `pokeys.0.adcin.N.val` |
| Emergency | 33 | 52 | PEv2 emergency out | PEv2 emergency in | ← EXCLUDED (HW-1/HW-2 open) |

Source: `Pokeys57E_SimPins.hal` (provenance); physical continuity not independently
confirmed. Do not treat switch-label comments (X min/home) as proof of INI enable
state — the current INI disables positive limits; only limit-minus and home are active.

---

## Reserved pins — do not use in HIL tests

```text
9   PE external DATA (OC16-CNC)
11  PE external CLOCK (OC16-CNC)
51  PE external LATCH (OC16-CNC)
52  PE emergency input (EXCLUDED — see HW-1/HW-2)
53  PE charge pump output
```

Pins 38–40 (integrated PE direction outputs) are available only when PEv2 is
switched from external generator (OC16-CNC) to integrated generator. Do not
reassign generator type within a primitive HIL test.

---

## CI workflow

HIL tests must run only on self-hosted runners with the physical fixture.

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
        run: pytest -m hil tests/hil/basic --junitxml=hil-results.xml
      - name: Stop HAL safely
        if: always()
        run: |
          halcmd stop || true
          halcmd unload all || true
```

Do not run this workflow on normal GitHub-hosted CI.

---

## Status vocabulary for PR evidence

```text
Unit-tested:        yes / no
HAL-smoke-tested:   yes / no
HIL-tested:         yes / no  (requires physical fixture run)
RT-validated:       yes / no  (requires measured timing)
```

Do not mark `HIL-tested: yes` based on a userspace simulation or code review.
