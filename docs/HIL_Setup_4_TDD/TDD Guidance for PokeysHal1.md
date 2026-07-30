Agreed — with one important distinction:

**Use HIL during unit-test design as the truth source, but do not turn every unit-test run into a hardware test.**

The better model is:

```text
HIL observation → codified unit-test expectation → implementation → HIL confirmation
```

So the unit test is still fast and deterministic, but its expected behavior is not invented from assumptions.

## Revised principle

```text
No unit test for hardware-facing behavior may be based only on developer assumption.

The expected behavior must come from at least one of:
1. PoKeys protocol/specification,
2. upstream pokeyslib behavior,
3. existing verified LinuxCNC config,
4. direct HIL observation on the PoKeys57E fixture.
```

For anything involving pin numbering, inversion, PEv2 switch semantics, PWM/ADC scaling, encoder direction, emergency logic, or async timing, **HIL should be used before writing the final unit expectation**.

That fits the repository discipline already present: establish a baseline before changing behavior, avoid fake completion, report exact evidence, and distinguish compiled/tested/hardware-verified states.  The C/RT rules also explicitly warn that “Async” is not enough; the full call path must be verified.

---

## Correct workflow: HIL-assisted Unit TDD

For hardware-facing code, use this loop:

```text
1. Define the behavior question.
2. Run a narrow HIL probe to observe real behavior.
3. Record the observation as a fixture/golden contract.
4. Write a failing unit test using that observed contract.
5. Implement the smallest code change.
6. Run unit tests.
7. Re-run the narrow HIL probe to confirm the implementation still matches hardware.
```

Example:

```text
Question:
Does physical Pin 23 map to HAL digout index 22 and cause physical Pin 28 input to change?

HIL probe:
Set pokeys.0.digout.22.out false/true/false.
Observe matching input/PEv2 LimitN state.

Unit test:
Assert HAL export/indexing/mapping function maps physical pin 23 to digout channel 22 and PEv2 LimitN pin 28.

Implementation:
Fix mapping/export/parser.

HIL confirmation:
Run the same physical loopback test again.
```

This avoids assumption-driven development without making all tests dependent on the physical PoKeys57E.

---

## Add this as a rule to HIL-TDD instructions

```markdown
## HIL-derived unit-test rule

For hardware-facing behavior, unit-test expectations must not be invented from assumptions.

Before writing or changing a unit test for any of the following, run a narrow HIL probe or cite an existing verified HIL observation:

- physical pin number to HAL channel index mapping
- digin/digout polarity
- PEv2 limit/home/emergency inversion semantics
- PWM/adcout scaling
- ADC raw/scaled value behavior
- pulse-engine direction/count behavior
- fast-encoder count direction
- async update latency/timeout behavior
- behavior after disconnect/reconnect or timeout

The HIL observation becomes the test oracle. The unit test then freezes that behavior in a deterministic host-side test.

A HIL-derived unit test must include a comment such as:

`/* Oracle: HIL-P57E-DIO-001, observed on PoKeys57E DEVICE_ID=27295 */`

Do not mark a behavior as `hardware-verified` unless the matching HIL test was actually run.
```

---

## Documentation structure I would use

```text
docs/testing/
  pokeys57e-hil-fixture.md
  pokeys57e-hil-observations.md

tests/unit/
  test_pin_mapping.c
  test_pev2_switch_mapping.c
  test_pwm_adc_scaling.c
  test_encoder_direction.c

tests/hil/basic/
  test_010_digital_loopback.py
  test_020_pwm_adc_loopback.py
  test_030_pe_encoder_loopback.py
  test_040_emergency_loopback.py
```

The key file is:

```text
docs/testing/pokeys57e-hil-observations.md
```

It should contain stable, versioned observations, for example:

```markdown
# PoKeys57E HIL Observations

## HIL-P57E-DIO-001 — physical Pin 23 to Pin 28 loopback

- Device: PoKeys57E
- DEVICE_ID: 27295
- Wiring: Pin 23 -> Pin 28
- HAL output: pokeys.0.digout.22.out
- Expected input: physical Pin 28 / PEv2 X LimitN path
- Observation:
  - output false -> input false/inactive
  - output true -> input true/active
  - output false -> input false/inactive
- Verified by: tests/hil/basic/test_010_digital_loopback.py
```

The existing fixture already documents the relevant physical wiring: digital outputs to inputs, PWM to analog inputs, and pulse-engine outputs through PoStep/stepper/encoder back to fast encoders.

---

## Unit-test oracle examples

### 1. Pin-index mapping

The existing HAL file confirms that physical Pin 23 is driven through `pokeys.0.digout.22.out`, physical Pin 24 through `digout.23.out`, and so on.  That should become a unit test for the mapping helper, not a scattered assumption.

```c
void test_physical_pin_to_hal_digout_index_uses_zero_based_channel(void)
{
    /* Oracle: HIL-P57E-DIO-001 */
    TEST_ASSERT_EQUAL_INT(22, pk_hal_digout_index_from_physical_pin(23));
    TEST_ASSERT_EQUAL_INT(23, pk_hal_digout_index_from_physical_pin(24));
    TEST_ASSERT_EQUAL_INT(24, pk_hal_digout_index_from_physical_pin(25));
    TEST_ASSERT_EQUAL_INT(25, pk_hal_digout_index_from_physical_pin(26));
}
```

### 2. PEv2 switch mapping

The INI maps X limit-minus to Pin 28 and X home to Pin 29. Positive limit is disabled.  So the unit test should encode exactly that:

```c
void test_x_axis_switch_mapping_matches_verified_p57e_fixture(void)
{
    /* Oracle: HIL-P57E-SWITCH-001 */
    TEST_ASSERT_EQUAL_INT(28, axis[0].limit_n_pin);
    TEST_ASSERT_EQUAL_INT(29, axis[0].home_pin);
    TEST_ASSERT_EQUAL_INT(0,  axis[0].limit_p_pin);
}
```

Same for X2, where Pin 30 is limit-minus and Pin 31 is home.

### 3. PWM/ADC scaling

The existing full config uses `pokeys.0.adcout.0.value` and `pokeys.0.adcout.0.enable` through a spindle scaling path.  For unit testing, do not test spindle behavior. First observe the primitive PWM/ADC relation on HIL, then freeze the scaling assumptions.

```c
void test_adcout_value_to_pwm_command_uses_verified_scale(void)
{
    /* Oracle: HIL-P57E-ADCOUT-001 */
    TEST_ASSERT_EQUAL_UINT32(expected_raw_mid, pk_adcout_scaled_to_raw(2.5));
}
```

The actual expected value should come from the HIL observation or protocol docs, not guessed.

---

## HIL as “assumption breaker”

Add this status category:

```text
HIL-observed
```

Use it before `unit-tested` and before `HIL-tested`.

| Status                | Meaning                                                           |
| --------------------- | ----------------------------------------------------------------- |
| `Assumed`             | Not acceptable for implementation decisions                       |
| `HIL-observed`        | Real board behavior was observed and recorded                     |
| `Unit-tested`         | The observed/spec behavior is encoded in deterministic host tests |
| `HIL-tested`          | The implementation was run against the physical fixture           |
| `Integration-tested`  | Full LinuxCNC machine config was run                              |
| `RT-timing-validated` | Timing was measured with defined threshold                        |

This keeps the language precise. HIL observation is not the same as a passing HIL regression test.

---

## Updated HIL-TDD loop

Use this as the final discipline:

```text
For each hardware-facing issue:

1. HIL-observe the primitive behavior.
2. Document the observation ID.
3. Write/adjust the unit test from the observation.
4. Watch the unit test fail.
5. Implement.
6. Run unit tests.
7. Run the matching basic HIL test.
8. Only then update integration tests or full machine config.
```

That is the right balance: HIL prevents false assumptions, unit tests keep the development loop fast, and the full `DM542_XXYZ_mill` configuration remains an integration test rather than the foundation of every implementation decision.
