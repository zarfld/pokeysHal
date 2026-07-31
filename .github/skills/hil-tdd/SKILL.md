---
name: hil-tdd
description: >
  Run Red-Green-Refactor development against a registered PoKeys hardware-in-the-loop
  fixture. Use for changes whose acceptance criteria depend on real PoKeys hardware,
  firmware, HAL pin propagation, PEv2 behavior, electrical loopbacks, or measured timing.
---

# HIL-TDD Skill

## When to use

Use this skill when the acceptance criterion requires:
- real PoKeys57E digital I/O (not a mock or simulator);
- firmware-level PEv2 switch mapping (LimitN, Home, emergency);
- physical loopback electrical verification;
- PWM/ADC analog path confirmation;
- async update latency with real USB/Ethernet round-trips;
- RT environment validation in LinuxCNC;
- timing validation against a defined threshold and tolerance.

Do not use HIL as a substitute for unit tests.

---

## Step A — Classify the requirement

Determine which test layers apply:

1. Unit test — parser, mailbox, HAL export, inversion logic
2. Protocol test — async send/parse/retry coverage
3. HAL smoke — `halcompile`/`halrun`, exported pins visible
4. Primitive HIL — real board I/O, minimal HAL config, no machine stack
5. Machine integration — full `DM542_XXYZ_mill`, `wcomp`, homing, joint wiring
6. RT environment validation — execute the behavior in the applicable LinuxCNC RT environment
7. Timing validation — measure timing against a documented threshold and tolerance

HIL (layer 4) does not replace layers 1–3. Layers 1–2 are required where
mechanically applicable; a documentation-only change does not need an invented unit test.

---

## Step B — Select the fixture

Specify the setup ID: `pokeys57e-loopback-v1`

Before proceeding, read the fixture YAML at
`tests/hil/setups/pokeys57e-loopback-v1.yaml` and verify:

- `runnable: true` — if not, stop; the fixture is not verified for use
- `fixture_status: verified` — if draft, stop; hardware questions are unresolved
- device model and fixture `revision` match
- when the fixture is verified and `expected_device_id` is set, the device ID must match
- firmware version is compatible
- the required capability is listed (not merely declared as draft)
- required HAL pin names are present in the setup definition
- all verification fields required by the selected capability are `true`

For the current physical-pin layout and capability constraints, see
`tests/hil/setups/pokeys57e-loopback-v1.md`.

---

## Step C — Acquire and preflight

Before touching any output:

1. Acquire exclusive fixture lock (prevent parallel runs).
2. Verify expected PoKeys device is reachable and identity matches.
3. Confirm no production machine or actuators are attached.
4. Configure all source channels to `startup_mode` (high impedance if specified).
5. Configure sink endpoints as `required_mode_before_source_enable`.
6. Apply `safe_logical_value` to all source channels.
7. Read back and verify all source channels report the expected safe physical level.
8. Verify all required HAL pin names exist.

### Preflight outcomes

| Condition | Result |
|---|---|
| `POKEYS_HIL` absent | SKIPPED |
| Test suite discovered on a non-HIL runner without HIL being requested | SKIPPED |
| `POKEYS_HIL=1`, but runner lacks the fixture | ERROR |
| Fixture draft or not runnable | ERROR |
| Device missing or mismatched during requested HIL run | ERROR |
| Continuity/self-test fails | ERROR |
| Behavior differs from expectation | FAIL |
| Success | PASS |

A required HIL job that produces zero test results must not be green.

---

## Step D — Red phase

Two distinct workflows apply depending on whether the behavior is already
specified or must be discovered from hardware.

### Workflow A — Specification-known behavior

Use when the expected behavior is documented in the PoKeys protocol spec,
upstream `pokeyslib`, or a verified existing configuration:

```
authoritative specification or existing verified HIL observation
→ failing unit test (oracle cited in comment)
→ implementation
→ unit test passes
→ HIL confirmation run
```

### Workflow B — Hardware behavior not yet known

Use when the expected behavior must be observed before a test can be written:

```
narrow exploratory HIL probe (not a Red test — record observation only)
→ observation recorded in docs/testing/pokeys57e-hil-observations.md
→ failing deterministic unit test (observation as oracle)
→ implementation
→ unit test passes
→ automated HIL regression test
```

The exploratory observation is not itself the Red test. Running a probe without
a prior expectation is valid for discovery; it must not be reported as a test result.

### Oracle comment format

```c
/* Oracle: HIL-P57E-DIO-001, setup=pokeys57e-loopback-v1 rev=1, device=<verified-id> */
```

---

## Step E — Green phase

Implement the smallest production change. Then run in order:

1. Narrow unit/protocol test suite.
2. `halcompile` + `halrun` smoke test.
3. Relevant primitive HIL test case.

Do not run the full machine integration config until the primitive HIL test passes.

---

## Step F — Refactor

Keep all three layers green: unit tests, HAL smoke, HIL primitive.

---

## Step G — Cleanup

Whether the test passes, fails, or is interrupted:

1. Apply `safe_logical_value` to all source channels.
2. Configure all sources back to `startup_mode` (high impedance).
3. Disable any PEv2 operations started by the test.
4. Unload test HAL components (`halcmd unload all`).
5. Release the fixture lock.
6. Report whether cleanup succeeded.

A test that leaves any output in a non-safe state is a defect in the test.

---

## Step H — Evidence

Every HIL run must record:

```
commit SHA
setup ID and revision
device model and expected_device_id
firmware version
LinuxCNC version
kernel / RT mode
HAL configuration used
test command
test result (PASS / FAIL / SKIPPED / ERROR and reason)
observed input/output transitions
cleanup result
```

See `references/result-schema.md` for the full status vocabulary.

---

## HIL test sequence — primitive fixture

Tests run in this order; each depends on the previous passing.

### HIL-000 — Identity and HAL export

- Load a minimal HAL config (not the full machine INI).
- Confirm device is reachable and `expected_device_id` matches.
- Confirm all HAL pins required by the declared loopbacks are present.
- Unload cleanly.

A HAL pin name mismatch here invalidates all subsequent tests.

### HIL-010 — Direct digital loopback

For each loopback pair listed in the fixture YAML:

```
set source to safe_logical_value
verify sink reports expected safe state
set source to active (not safe_logical_value)
verify sink transitions
set source back to safe_logical_value
verify sink returns to safe state
```

Test raw digital input and PEv2 logical input (LimitN/Home) separately, because
the INI enables inversion. Catches: wrong HAL index, wrong PinFunction, wrong
parser, wrong PEv2 mapping, inversion error, async starvation.

### HIL-020 — PWM/ADC loopback

Requires RC low-pass filter (4.7 kΩ + 1 µF) at each PWM→ADC connection.
See `tests/hil/setups/pokeys57e-loopback-v1.md` for wiring constraints.
**Status**: excluded from v1; RC filter not fitted.

### HIL-030 — Pulse engine to fast encoder

**Status**: excluded from v1; HW-3 (motor-enable conflict) unresolved.

### HIL-040 — Emergency loopback

**Status**: excluded from v1; HW-1/HW-2 (Pin 33/52 vs INI Pin 54) unresolved.

---

## PR evidence checklist

```
Unit-tested:         yes / no
HAL-smoke-tested:    yes / no
HIL-test-executed:   yes / no  (requires physical fixture run against a verified fixture)
HIL-verified:        yes / no  (requires the named HIL test to pass on the named fixture revision)
RT-validated:        yes / no  (behavior executed in the applicable RT environment)
Timing-validated:    yes / no  (timing measured against a threshold)
```

See `references/result-schema.md` for precise status term definitions.
Do not mark `HIL-verified: yes` based on a userspace simulation or code review.
