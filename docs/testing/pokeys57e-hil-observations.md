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
**HAL output**: `pokeys.0.digout.NN.out`  
**Expected input**: `pokeys.0.PEv2.N.digin.XXX.in`

**Observation**:
- output false → input false/inactive (verified)
- output true → input true/active (verified)
- output false → input false/inactive (verified)

**Verified by**: tests/hil/basic/pytest/test_XXX.py  
**Used as oracle in**: tests/unit/test_XXX.c
```

Unit tests citing an oracle must include:
```c
/* Oracle: HIL-P57E-DIO-001, observed on PoKeys57E DEVICE_ID=27295 */
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
