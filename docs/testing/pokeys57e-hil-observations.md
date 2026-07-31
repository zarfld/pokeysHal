# PoKeys57E HIL Observations

This file records real hardware observations used as unit-test oracles.

## Format

Each observation entry:

```markdown
## HIL-P57E-XXX-NNN — <short description>

- **Date**: YYYY-MM-DD
- **Device**: PoKeys57E
- **Device ID**: <id from halcmd show param>
- **Firmware**: <version>
- **Setup ID**: pokeys57e-loopback-v1 rev <N>
- **Commit**: <SHA>

**Wiring**: Physical Pin X → Physical Pin Y
**HAL source**: `pokeys.0.digout.NN.out`
**Observed raw digital input**: `pokeys.0.digin.MM.in`
**Observed PEv2 logical input**: `pokeys.0.PEv2.N.digin.XXX.in`

**Observation**:
- source HAL command: (e.g., `halcmd setp pokeys.0.digout.22.out false`)
- source physical level: <measured voltage/high/low | not measured>
- measurement instrument: <instrument/model | none>
- sink raw digital reading: (e.g., `halcmd gets pokeys.0.digin.27.in`)
- PEv2 logical reading: (e.g., `halcmd gets pokeys.0.PEv2.0.digin.LimitN.in`)
- configured inversion: (yes/no; INI/HAL source)
- settling/update time: <measured value and method | not measured>

**Verified by**: tests/hil/basic/pytest/test_XXX.py
**Used as oracle in**: tests/unit/test_XXX.c
```

Unit tests citing an oracle must include:
```c
/* Oracle: HIL-P57E-DIO-001, setup=pokeys57e-loopback-v1 rev=1, device=<verified-id> */
```

---

## Observations

*No observations recorded yet. Add entries after physical fixture runs.*

### Pending — to be recorded when fixture is confirmed

- HIL-P57E-DIO-001: Physical Pin 23 → Pin 28 loopback (x-limit-negative)
- HIL-P57E-DIO-002: Physical Pin 24 → Pin 29 loopback (x-home)
- HIL-P57E-DIO-003: Physical Pin 25 → Pin 30 loopback (x2-limit-negative)
- HIL-P57E-DIO-004: Physical Pin 26 → Pin 31 loopback (x2-home)
- HIL-P57E-DIO-005: Physical Pin 12 → Pin 37 loopback (y-limit-negative)
- HIL-P57E-DIO-006: Physical Pin 13 → Pin 38 loopback (y-home)
- HIL-P57E-DIO-007: Physical Pin 14 → Pin 39 loopback (z-limit-negative)
- HIL-P57E-DIO-008: Physical Pin 15 → Pin 40 loopback (z-home)

All pending until HW-4 (physical continuity) and HW-5 (polarity) are confirmed.
See issue #138.
