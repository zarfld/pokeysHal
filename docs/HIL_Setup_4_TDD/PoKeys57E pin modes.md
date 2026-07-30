# PoKeys57E HIL wiring: available pin modes and reuse

## 1. Configuration model

For the PoKeys HAL interface, the physical pin number is **1-based**, while the HAL array index is **0-based**:

```text
Physical pin 23 -> pokeys.0.pins.22
Physical pin 28 -> pokeys.0.pins.27
```

This is already visible in `Pokeys57E_SimPins.hal`: physical pins 23–26 are driven through `digout.22`–`digout.25`, with `PinFunction 4` selecting digital-output mode.

The relevant basic `PinFunction` bit values are:

| Value | Meaning                                                                           |
| ----: | --------------------------------------------------------------------------------- |
|   `0` | Inactive/restricted, high impedance                                               |
|   `2` | Digital input                                                                     |
|   `4` | Digital output                                                                    |
|   `8` | Analog input, supported pins only                                                 |
|  `16` | Analog output, **not supported by PoKeys57E**                                     |
|  `32` | Triggered input; keyboard-related and not available on PoKeys57E Ethernet devices |
|  `64` | Digital counter, supported pins only                                              |
| `128` | Invert polarity, combined with digital input/output                               |

The library capability table confirms that PoKeys57-series devices support digital input/output on pins 1–55, analog input on pins 41–47, PWM on pins 17–22 and fixed fast-encoder pins. Analog output is only listed for the older PoKeys55 series; triggered input is limited to the USB variants.

Special functions such as PWM, fast encoders, LCD, Matrix LED and PEv2 signals are normally configured through their respective peripheral subsystem, not merely by placing the corresponding numeric value in `PinFunction`.

## 2. Pins occupied by the PoExtBusOC16-CNC and PEv2

Your PoExtBusOC16-CNC is an **external pulse generator without dedicated I/O**. On a PoKeys57E it uses:

| PoKeys57E pin | Function                            | Availability while PEv2 external generator runs |
| ------------: | ----------------------------------- | ----------------------------------------------- |
|             9 | Serialized step/direction **DATA**  | Reserved                                        |
|            11 | Serialized step/direction **CLOCK** | Reserved                                        |
|            51 | Serialized step/direction **LATCH** | Reserved                                        |
| Any free GPIO | Shared motor-enable output          | Reserved if configured                          |
|            52 | PEv2 emergency input                | Normally reserved                               |
|            53 | PEv2 5 kHz charge-pump output       | Normally reserved                               |

The OC16-CNC manual explicitly assigns pins 9, 11 and 51 and allows any free I/O pin for motor enable. It does **not** use pins 35–38 because it has no dedicated I/O capability. ([Polabs][1])

The external pulse-generator protocol is a serialized 74HCT595-style interface:

```text
Pin 9  = DATA
Pin 11 = CLOCK
Pin 51 = LATCH
```

([Polabs][2])

Your INI confirms external generation and disables extended I/O:

```ini
PEv2_PulseGeneratorType=0   # external
PEv2_PG_extended_io=0
```

Therefore, in this configuration, pins **38–40 are available as ordinary GPIO inputs**. Their fixed DIR-X/Y/Z functions apply when the **integrated** pulse generator is selected. That conclusion is based on the selected external/no-I/O generator mode and the mutually exclusive generator pin assignments. ([Polabs][2])

---

# 3. Direct digital loopback wiring

All these connections can be reversed for generic GPIO testing because both ends support digital input and output. Only one side may be an output at any moment.

## X and X2 switches

| Wiring      | Current function | Source-pin modes                                                 | Destination-pin modes                                            | Additional HIL reuse                                                                 |
| ----------- | ---------------- | ---------------------------------------------------------------- | ---------------------------------------------------------------- | ------------------------------------------------------------------------------------ |
| **23 → 28** | X minimum        | **23:** GPIO I/O, Counter 23, LCD primary D7, Matrix LED 2 DATA  | **28:** GPIO I/O, Counter 28, LCD R/W                            | GPIO propagation; polarity; counter edge testing; Matrix-LED serial-data observation |
| **24 → 29** | X maximum/home   | **24:** GPIO I/O, Counter 24, LCD primary D6, Matrix LED 2 LATCH | **29:** GPIO I/O, LCD RS                                         | GPIO propagation; Matrix-LED latch verification; LCD control-line test               |
| **25 → 30** | X2 minimum       | **25:** GPIO I/O, Counter 25, LCD primary D5, Matrix LED 2 CLOCK | **30:** GPIO I/O, LCD E                                          | GPIO propagation; Matrix-LED clock verification; LCD enable test                     |
| **26 → 31** | X2 maximum/home  | **26:** GPIO I/O, Counter 26, LCD primary D4                     | **31:** GPIO I/O, LCD secondary D7                               | GPIO and LCD-bus loopback                                                            |
| **27 → 32** | Currently unused | **27:** GPIO I/O, Counter 27, Fast Encoder 3 index               | **32:** GPIO I/O, LCD secondary D6                               | Recommended spare stimulus/response line; Fast Encoder 3 index testing               |
| **33 → 52** | PEv2 emergency   | **33:** GPIO I/O, LCD secondary D5                               | **52:** GPIO I/O, PEv2 emergency input, connection-signal output | PEv2 emergency-state test; reverse direction can test connection-signal output       |

The pin-specific LCD, Matrix LED and counter functions are documented in the current PoKeys manual. ([Polabs][3])

### Important emergency-input limitation

Pin 33 driving pin 52 can validate:

* PEv2 input interpretation
* inversion
* state transitions
* software reaction
* motion-buffer stopping

It **does not validate a real emergency-stop system**. The PEv2 manual requires an NC emergency circuit and independent removal of motor power. ([Polabs][2])

There is also a configuration inconsistency to correct:

```text
Wiring comment:       Pin 33 -> Pin 52
Current INI setting:  PEv2_EmergencyInputPin=54
```

The current INI selects pin 54, whereas the manual and physical loopback use pin 52.

Unless the component intentionally applies an undocumented offset, this should be:

```ini
PEv2_EmergencyInputPin=52
```

Pin 54 also has the hardware recovery/reset function at startup, making it an undesirable emergency-test input. ([Polabs][3])

## Y and Z switches

| Wiring      | Current function | Source-pin modes                               | Destination-pin modes                                                               | Additional HIL reuse                                                            |
| ----------- | ---------------- | ---------------------------------------------- | ----------------------------------------------------------------------------------- | ------------------------------------------------------------------------------- |
| **12 → 37** | Y minimum        | **12:** GPIO I/O, Ultra-fast encoder B         | **37:** GPIO I/O, external-generator dedicated-I/O signal, alternate PoExtBus latch | GPIO switch; UFE-B stimulus; ordinary PoExtBus latch test when applicable       |
| **13 → 38** | Y home           | **13:** GPIO I/O, Ultra-fast encoder index     | **38:** GPIO I/O, integrated PE DIR X, external-generator dedicated-I/O signal      | GPIO switch; UFE-index stimulus; integrated-generator DIR-X observation         |
| **14 → 39** | Z minimum        | **14:** GPIO I/O                               | **39:** GPIO I/O, integrated PE DIR Y                                               | GPIO switch; integrated-generator DIR-Y observation                             |
| **15 → 40** | Z home           | **15:** GPIO I/O, Counter 15, Fast Encoder 3 A | **40:** GPIO I/O, integrated PE DIR Z                                               | GPIO switch; Counter 15; FE3-A stimulus; integrated-generator DIR-Z observation |

Pins 35–38 have special external-generator/PoExtBus functions, while pins 38–40 are the integrated generator direction outputs. ([Polabs][3])

### Valuable reuse: integrated-generator direction test

With the OC16-CNC configuration disabled and PEv2 temporarily switched to the integrated generator:

```text
PE DIR X pin 38 -> pin 13 input
PE DIR Y pin 39 -> pin 14 input
PE DIR Z pin 40 -> pin 15 input
```

The existing wiring therefore provides a direct way to verify:

* direction polarity
* direction changes
* axis inversion
* direction-before-step timing at the logical level
* PEv2 integrated-generator pin ownership

The integrated generator additionally uses pins 46, 48 and 49 for STEP X/Y/Z. Those pins are not currently looped back. ([Polabs][2])

---

# 4. PWM-to-analog/counter loopbacks

| Wiring      | Source capabilities                 | Destination capabilities              | Suitable tests                                   |
| ----------- | ----------------------------------- | ------------------------------------- | ------------------------------------------------ |
| **17 → 41** | GPIO I/O, PWM channel 6             | GPIO I/O, Analog input 41, Counter 41 | PWM duty, frequency and GPIO                     |
| **18 → 42** | GPIO I/O, PWM channel 5             | GPIO I/O, Analog input 42, Counter 42 | PWM duty, frequency and GPIO                     |
| **19 → 43** | GPIO I/O, PWM channel 4, Counter 19 | GPIO I/O, Analog input 43, Counter 43 | PWM duty/frequency; counters in either direction |
| **20 → 44** | GPIO I/O, PWM channel 3, Counter 20 | GPIO I/O, Analog input 44, Counter 44 | PWM duty/frequency; counters in either direction |

Pins 17–22 are the PWM outputs, and pins 41–47 are 12-bit analog inputs. The listed counter capabilities also apply to pins 19, 20 and 41–44. ([Polabs][3])

## Recommended test modes

### Digital PWM edge test

Configure the destination as a digital input or counter:

```text
PWM source -> digital input
PWM source -> hardware counter
```

This validates:

* PWM enable/disable
* output toggling
* approximate frequency
* edge count
* duty extremes of 0% and 100%

### Analog duty-cycle test

A directly connected PWM output is still a **0/3.3 V square wave**, not a DC analog signal. The analog input may return unstable or aliased results, especially because the ADC sampling rate is 10 kHz and your configuration uses a PWM period of 2500 counts, which commonly gives a 10 kHz PWM carrier from the 25 MHz timer.

For a reliable duty-to-voltage test, add an RC low-pass network to each connection, for example:

```text
PWM pin ---- 4.7 kΩ ----+---- analog input
                        |
                       1 µF
                        |
                       GND
```

Expected value after settling:

```text
ADC voltage ≈ PWM duty × 3.3 V
```

The analog-input voltage must remain within 0–3.3 V. Digital outputs produce approximately 0–3.3 V, so the voltage ranges are compatible. ([Polabs][3])

---

# 5. Motion/encoder HIL paths

## Encoder feedback pins

| Feedback path                                   | PoKeys pins | Available modes                           | Current use              |
| ----------------------------------------------- | ----------: | ----------------------------------------- | ------------------------ |
| PE axis 1 → OC16-CNC → PoStep → motor → encoder | **1 and 2** | GPIO I/O, Counter 1/2, Fast Encoder 1 A/B | End-to-end axis feedback |
| PE axis 3 → OC16-CNC → PoStep → motor → encoder | **5 and 6** | GPIO I/O, Counter 5/6, Fast Encoder 2 A/B | End-to-end axis feedback |

Fast Encoder 1 uses pins 1 and 2, while Fast Encoder 2 uses pins 5 and 6. ([Polabs][3])

Pins 5 and 6 have a hardware coupling restriction on PoKeys57E: when either is configured as an output, both must be configured as outputs. This is irrelevant while they are encoder inputs but must be handled by any GPIO direction-change test. ([Polabs][3])

These paths provide the strongest HIL tests because they cover:

```text
LinuxCNC command
   ↓
pokeysHal
   ↓
PEv2 motion buffer
   ↓
Pins 9/11/51 serialized output
   ↓
PoExtBusOC16-CNC
   ↓
PoStep / stepper drive
   ↓
physical motor motion
   ↓
encoder A/B
   ↓
Fast Encoder 1 or 2
   ↓
pokeysHal feedback
```

Suitable assertions include:

* commanded direction equals encoder direction
* commanded distance equals encoder count within tolerance
* stop causes encoder count to stop
* emergency stops pulse generation
* position remains monotonic during a one-direction move
* reversal changes encoder sign
* acceleration limits do not produce implausible count discontinuities
* motor-disable produces no encoder motion

## Motor-enable conflict to check

The OC16-CNC documentation uses pin 1 as the default motor-enable output but explicitly permits another free GPIO. ([Polabs][1])

Your pin 1 is used as **Fast Encoder 1 A**, and the INI repeatedly contains:

```ini
PEv2_AxisEnableOutputPins_0=1
PEv2_AxisEnableOutputPins_1=1
PEv2_AxisEnableOutputPins_2=1
PEv2_AxisEnableOutputPins_6=1
```

If that parameter contains a physical output-pin number, it conflicts with Fast Encoder 1 A. The motor-enable signal should then be moved to an otherwise unused GPIO, for example pin 27/32 only after removing that loopback, or another genuinely free pin outside the HIL harness.

---

# 6. Effective pin-ownership map

## Reserved while using the OC16-CNC external pulse generator

```text
9   PE external DATA
11  PE external CLOCK
51  PE external LATCH
52  PE emergency input
53  PE charge pump
x   Selected motor-enable GPIO
1/2 Encoder 1 feedback
5/6 Encoder 2 feedback
```

## Available for digital switch simulation

```text
12-15 -> 37-40
23-27 -> 28-32
33    -> 52, with emergency-specific restrictions
```

## Available for PWM/ADC/counter testing

```text
17 -> 41
18 -> 42
19 -> 43
20 -> 44
```

## Available only after changing pulse-generator type

```text
38, 39, 40  Integrated PE direction outputs
46, 48, 49  Integrated PE step outputs
35-38       External generator with dedicated I/O / CNCaddon functions
```

The core rule for every reusable wire should be enforced by the HIL setup:

> **Exactly one endpoint may be configured as an active output. The opposite endpoint must be input, analog input, counter or an applicable peripheral input before the output is enabled.**

A small series resistor, approximately **1–4.7 kΩ**, in each direct GPIO loopback would also limit current if a defective test accidentally configures both endpoints as opposing outputs.

[1]: https://www.poscope.com/wp-content/uploads/downloads/PoKeysPeriph/Manuals/PoExtBusOC16-CNC%20user%20manual.pdf "https://www.poscope.com/wp-content/uploads/downloads/PoKeysPeriph/Manuals/PoExtBusOC16-CNC%20user%20manual.pdf"
[2]: https://www.poscope.com/wp-content/uploads/downloads/Pokeys/Manuals/PoKeys%20Pulse%20engine%20v2%20-%20user%20manual.pdf "https://www.poscope.com/wp-content/uploads/downloads/Pokeys/Manuals/PoKeys%20Pulse%20engine%20v2%20-%20user%20manual.pdf"
[3]: https://www.poscope.com/wp-content/uploads/downloads/Pokeys/Manuals/PoKeys57%20-%20user%20manual.pdf "https://www.poscope.com/wp-content/uploads/downloads/Pokeys/Manuals/PoKeys57%20-%20user%20manual.pdf"
