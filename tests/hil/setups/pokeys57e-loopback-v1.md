# PoKeys57E HIL Fixture — Loopback v1

**Setup ID**: `pokeys57e-loopback-v1`  **Revision**: 1  
**Device**: PoKeys57E  **Expected device ID**: 27295 (unconfirmed — see HW-4)  
**Transport**: Ethernet

---

> **Provenance notice**: The wiring below is derived from `Pokeys57E_SimPins.hal`
> (historical machine configuration). Physical continuity has not been independently
> confirmed. All loopbacks are marked `status: unconfirmed` in the YAML until
> verified by a continuity or controlled toggle test (HW-4).

---

## Pin numbering invariant

PoKeys57E physical pin numbers are 1-based. HAL channel indices are 0-based.

```
Physical Pin N  →  HAL channel N−1
Physical Pin 23 →  pokeys.0.digout.22.out   ✓ confirmed from SimPins.hal
Physical Pin 12 →  pokeys.0.digout.11.out   ✓ confirmed from SimPins.hal
```

Do not treat physical pin numbers and HAL channel indices as interchangeable.

---

## Digital loopback wiring

| ID | Physical out | Physical in | HAL output | PEv2 logical input | INI active? |
|---|---|---|---|---|---|
| x-limit-negative | 23 | 28 | `digout.22.out` | `PEv2.0.digin.LimitN.in` | Yes — limit-minus |
| x-home | 24 | 29 | `digout.23.out` | `PEv2.0.digin.Home.in` | Yes |
| x2-limit-negative | 25 | 30 | `digout.24.out` | `PEv2.6.digin.LimitN.in` | Yes — limit-minus |
| x2-home | 26 | 31 | `digout.25.out` | `PEv2.6.digin.Home.in` | Yes |
| y-limit-negative | 12 | 37 | `digout.11.out` | `PEv2.1.digin.LimitN.in` | Yes — limit-minus |
| y-home | 13 | 38 | `digout.12.out` | `PEv2.1.digin.Home.in` | Yes |
| z-limit-negative | 14 | 39 | `digout.13.out` | `PEv2.2.digin.LimitN.in` | Yes — limit-minus |
| z-home | 15 | 40 | `digout.14.out` | `PEv2.2.digin.Home.in` | Yes |

HAL pin prefix for all: `pokeys.0.`

**Inversion**: The INI enables inversion for X and X2 LimitN and Home inputs.
Test raw electrical state (`digout.NN.out` / raw digin) and PEv2 logical state
(`PEv2.N.digin.LimitN.in`) **separately**.

**Positive limits**: The current INI disables positive limits for X, X2, Y, Z.
Do not test positive limit behavior in v1.

---

## Pin capabilities relevant to declared loopbacks

| Physical pin | GPIO | Counter | Fast Encoder | LCD/Matrix | PEv2 function |
|---|---|---|---|---|---|
| 12 | I/O | — | UFE B | — | — |
| 13 | I/O | — | UFE index | — | — |
| 14 | I/O | — | — | — | — |
| 15 | I/O | 15 | FE3 A | — | — |
| 23 | I/O | 23 | — | LCD primary D7, Matrix LED 2 DATA | — |
| 24 | I/O | 24 | — | LCD primary D6, Matrix LED 2 LATCH | — |
| 25 | I/O | 25 | — | LCD primary D5, Matrix LED 2 CLOCK | — |
| 26 | I/O | 26 | — | LCD primary D4 | — |
| 28 | I/O | 28 | — | LCD R/W | — |
| 29 | I/O | — | — | LCD RS | — |
| 30 | I/O | — | — | LCD E | — |
| 31 | I/O | — | — | LCD secondary D7 | — |
| 37 | I/O | — | — | — | ext-gen dedicated-I/O / PoExtBus latch |
| 38 | I/O | — | — | — | integrated PE DIR X / ext-gen dedicated-I/O |
| 39 | I/O | — | — | — | integrated PE DIR Y |
| 40 | I/O | — | — | — | integrated PE DIR Z |

Source: PoKeys57 user manual. Alt functions for pins 38–40 apply only when the
**integrated** pulse generator is selected; they are ordinary GPIO with the
OC16-CNC external generator.

---

## Reserved pins (must not be used by HIL tests)

| Physical pin | Function | Reason |
|---|---|---|
| 9 | PE external DATA | OC16-CNC reserved |
| 11 | PE external CLOCK | OC16-CNC reserved |
| 51 | PE external LATCH | OC16-CNC reserved |
| 52 | PE emergency input | EXCLUDED — see HW-1/HW-2 |
| 53 | PE charge-pump output | Reserved |
| 1, 2 | Fast Encoder 1 A/B | Encoder feedback path |
| 5, 6 | Fast Encoder 2 A/B | Encoder feedback path (HW coupling: both must match direction simultaneously) |

---

## Excluded capabilities (not in v1 scope)

| Capability | Reason excluded |
|---|---|
| Emergency loopback (Pin 33 → Pin 52) | HW-1: SimPins.hal says 52; INI says `PEv2_EmergencyInputPin=54`. Unresolved. |
| PWM → ADC loopback | Requires RC low-pass filter (4.7 kΩ + 1 µF) at each connection. Not yet fitted. |
| PE → fast encoder feedback | HW-3: `PEv2_AxisEnableOutputPins_N=1` may conflict with Fast Encoder 1A on pin 1. |

---

## Open hardware questions

| ID | Question | Blocking |
|---|---|---|
| HW-1 | Is the emergency loopback wired to Pin 52 or Pin 54? | Emergency tests |
| HW-2 | Is `PEv2_EmergencyInputPin` a physical pin number or API index? | Emergency tests |
| HW-3 | Does `PEv2_AxisEnableOutputPins_N=1` conflict with Fast Encoder 1A on pin 1? | PE/encoder tests |
| HW-4 | Physical continuity of all declared loopback pairs confirmed? | All loopbacks |
| HW-5 | Polarity/pull-up configuration of loopback endpoints? | Polarity fields in YAML |

Track these in issue #138.

---

## Safety rules

- All outputs must be driven to false/inactive before any test.
- Acquire exclusive fixture lock before changing any output.
- Restore all outputs to false/inactive after every test, even on failure.
- A series resistor (~1–4.7 kΩ) per GPIO loopback is recommended to limit current
  if a test defect configures both endpoints as opposing outputs.
- Exactly one endpoint must be configured as an active output at any time.
