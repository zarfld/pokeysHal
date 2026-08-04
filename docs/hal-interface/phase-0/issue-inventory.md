# Issue Inventory — Phase 0 HAL-Interface Baseline

Evidence from pokeysHal@cd1f0dc and LinuxCnc_PokeysLibComp@0c058e6c.
Inspected: 2026-08-03 through 2026-08-04.

---

## pokeysHal Issues (45 issues)

| Object | Title | State | Source ID | Relevance | Body status | Comment count | Comment status | Evidence summary | Conflicts | Retain |
|---|---|---|---|---|---|---|---|---|---|---|
| Issue #24 | no hal_free | CLOSED | C-012 | HIGH | complete | 0 | none | Confirmed: hal_free absent | None | YES |
| Issue #30 | ePK_RETURN_CODES | OPEN | None | LOW | complete | 0 | none | ePK_RETURN_CODES enum definition | None | NO |
| Issue #32 | HAL Interface: PoKeys device info | OPEN | C-001 | HIGH | complete | 0 | none | Specifies ~35 device info pins; devSerial/alive only implemented | None | YES |
| Issue #33 | HAL Interface: PEv2 | CLOSED | C-002 | HIGH | complete | 3 | complete | CLOSED but many pins absent; lifecycle refs, PoKeysLib example | CONFLICT-007 | YES |
| Issue #34 | HAL Interface: PoExtBus | OPEN | C-003 | HIGH | complete | 0 | none | PoExtBus digin/digout absent in pokeysHal | None | YES |
| Issue #35 | HAL-Interface: Analog Input | CLOSED | C-004 | HIGH | complete | 1 | complete | Canonical adcin exported; direction bug CONFLICT-009; in.raw vs value-raw | CONFLICT-003,CONFLICT-009 | YES |
| Issue #36 | HAL-Interface: Digital Output | OPEN | C-005 | HIGH | complete | 0 | none | digout.J.out, PoExtBus digout, PEv2 AxisEnabled.out specified | None | YES |
| Issue #37 | HAL-Interface: Analog Output (ref) | OPEN | C-006 | HIGH | complete | 0 | none | References #39 | CONFLICT-004 | YES |
| Issue #38 | HAL-Interface: Digital Input | OPEN | C-007 | HIGH | complete | 0 | none | digin.J.in/in-not; direction mismatch CONFLICT-009 | CONFLICT-009 | YES |
| Issue #39 | HAL-Interface: Analog Output (spec) | OPEN | C-008 | HIGH | complete | 2 | complete | Specifies canonical adcout; comments confirm PK_PWMConfigurationSetAsync | CONFLICT-004 | YES |
| Issue #41 | PEv2 async RT | OPEN | C-021 | HIGH | empty | 0 | none | Body null; 0 comments; no HAL ABI content (C-021) | CONFLICT-001 | YES |
| Issue #97 | Async Scheduler opts | OPEN | None | LOW | not-inspected | 0 | not-inspected | scheduler, not HAL pins | None | NO |
| Issue #99 | REQ-F-SCHED-001 | OPEN | None | LOW | not-inspected | 0 | not-inspected | scheduler | None | NO |
| Issue #100 | REQ-F-SCHED-002 | OPEN | None | LOW | not-inspected | 0 | not-inspected | scheduler | None | NO |
| Issue #101 | REQ-F-SCHED-003 | OPEN | None | LOW | not-inspected | 0 | not-inspected | scheduler | None | NO |
| Issue #102 | REQ-F-SCHED-004 | OPEN | None | LOW | not-inspected | 0 | not-inspected | scheduler | None | NO |
| Issue #103 | REQ-F-SCHED-005 | OPEN | None | LOW | not-inspected | 0 | not-inspected | scheduler | None | NO |
| Issue #104 | REQ-F-SCHED-006 | OPEN | None | LOW | not-inspected | 0 | not-inspected | scheduler | None | NO |
| Issue #105 | REQ-NF-SCHED-001 | OPEN | None | LOW | not-inspected | 0 | not-inspected | scheduler | None | NO |
| Issue #106 | REQ-NF-SCHED-002 | OPEN | None | LOW | not-inspected | 0 | not-inspected | scheduler | None | NO |
| Issue #107 | REQ-NF-SCHED-003 | OPEN | None | LOW | not-inspected | 0 | not-inspected | scheduler | None | NO |
| Issue #108 | ADR-SCHED-001 | OPEN | None | LOW | not-inspected | 0 | not-inspected | scheduler | None | NO |
| Issue #109 | ADR-SCHED-002 | OPEN | None | LOW | not-inspected | 0 | not-inspected | scheduler | None | NO |
| Issue #110 | ADR-SCHED-003 | OPEN | None | LOW | not-inspected | 0 | not-inspected | scheduler | None | NO |
| Issue #111 | ADR-SCHED-004 | OPEN | None | LOW | not-inspected | 0 | not-inspected | scheduler | None | NO |
| Issue #112 | ADR-SCHED-005 | OPEN | None | LOW | not-inspected | 0 | not-inspected | scheduler | None | NO |
| Issue #113 | ARC-C-ASYNC-SCHED | OPEN | None | LOW | not-inspected | 0 | not-inspected | scheduler | None | NO |
| Issue #116 | REQ-F-PEV2-001 | OPEN | C-009 | HIGH | complete | 0 | none | Global PEv2 output/input pin spec | None | YES |
| Issue #117 | REQ-F-PEV2-002 | OPEN | C-010 | HIGH | complete | 0 | none | Per-axis pins J=0..7 | None | YES |
| Issue #118 | REQ-F-PEV2-003 | OPEN | C-011 | HIGH | complete | 0 | none | nrOfAxes conditional creation | CONFLICT-001 | YES |
| Issue #119 | REQ-F-PEV2-004 | OPEN | C-013 | HIGH | complete | 0 | none | HAL_IN→PoKeysLib pointer mapping | None | YES |
| Issue #120 | REQ-F-PEV2-005 | OPEN | C-014 | HIGH | complete | 0 | none | PoKeysLib pointer→HAL_OUT mapping | None | YES |
| Issue #121 | REQ-F-PEV2-006 | OPEN | C-015 | HIGH | complete | 0 | none | Bitmap decomposition strategy | None | YES |
| Issue #122 | REQ-F-PEV2-007 | OPEN | C-016 | HIGH | complete | 0 | none | Emergency/probe/limit/soft params | None | YES |
| Issue #123 | REQ-F-PEV2-008 | OPEN | C-017 | HIGH | complete | 0 | none | Async call sequence | None | YES |
| Issue #124 | REQ-NF-PEV2-001 | OPEN | C-018 | HIGH | complete | 0 | none | RT safety; no blocking | None | YES |
| Issue #125 | REQ-NF-PEV2-002 | OPEN | C-019 | HIGH | complete | 0 | none | 100% parity with #33 | None | YES |
| Issue #126 | REQ-NF-PEV2-003 | OPEN | C-020 | MEDIUM | complete | 0 | none | No magic numbers | None | YES |
| Issue #127 | ADR-PEV2-001: Naming | OPEN | D-002 | HIGH | complete | 0 | none | Governs PEv2.N.* naming | None | YES |
| Issue #128 | ADR-PEV2-002: nrOfAxes | OPEN | D-001 | HIGH | complete | 0 | none | nrOfAxes fallback=8 | CONFLICT-001 | YES |
| Issue #129 | ADR-PEV2-003: Bitmap | OPEN | D-003 | HIGH | complete | 0 | none | Accepted; decomposition strategy | None | YES |
| Issue #130 | ADR-PEV2-004: Read/Write | OPEN | D-004 | MEDIUM | complete | 0 | none | Accepted; read/write sequence | None | YES |
| Issue #131 | ADR-PEV2-005: Struct | OPEN | D-005 | MEDIUM | complete | 0 | none | Global vs per-axis layout | None | YES |
| Issue #132 | ARC-C-PEV2 | OPEN | D-006 | MEDIUM | complete | 0 | none | References ADRs #127-#131 | None | YES |
| Issue #133 | SDD-PEV2 | OPEN | D-007 | HIGH | complete | 0 | none | Full PEv2 design spec | CONFLICT-007 | YES |

---

## LinuxCnc_PokeysLibComp Issues (18 issues, 1 PR)

| Object | Title | State | Source ID | Relevance | Body status | Comment count | Comment status | Evidence summary | Conflicts | Retain |
|---|---|---|---|---|---|---|---|---|---|---|
| Issue #16 | pokeys_py SRS | OPEN | None | LOW | partial | 0 | none | SRS for pokeys_py; digital/analog I/O, PEv2 | None | LIMITED |
| Issue #21 | Custom homing interface | CLOSED | E-013 | MEDIUM | complete | 1 | complete | Defines homecomp interface; comment: pokeys.comp not yet updated | None | YES |
| Issue #24 | homecomp compatibility | CLOSED | E-014 | MEDIUM | complete | 1 | complete | Verify homecomp vs pokeys.comp; comment: split to userspace | None | YES |
| Issue #28 | userspace equivalence | CLOSED | None | HIGH | complete | 0 | none | Verify pokeys_userspace == pokeys.comp | None | YES |
| PR #29 | Add missing userspace functions | MERGED | None | HIGH | complete | 0 | none | Adds PWM, PoExtBus to userspace; merged 2024-09-27 | None | YES |
| Issue #30 | Tests for main branch | OPEN | None | LOW | complete | 0 | none | Test scope for pokeys.comp and userspace | None | YES |
| Issue #31 | Tests for pokeys_py | OPEN | None | LOW | complete | 0 | none | Test scope for pokeys_py | None | YES |
| Issue #69 | Modular architecture | OPEN | None | MEDIUM | complete | 0 | none | Refactor codebase modular | None | YES |
| Issue #73 | Align with LinuxCNC builds | OPEN | None | MEDIUM | complete | 0 | none | Build environment 2.8.x/2.9.x | None | YES |
| Issue #79 | CDI conformity | OPEN | None | HIGH | complete | 0 | none | CDI conformity; CONFLICT-010 on CDI scope | CONFLICT-010 | YES |
| Issue #129 | HAL-Interface: RTC | CLOSED | None | LOW | complete | 0 | none | rtc.* pins; matches pokeysHal | None | MAYBE |
| Issue #157 | Common pin I/O handling | CLOSED | None | HIGH | complete | 0 | none | Combine I/O handling | None | YES |
| Issue #213 | Encoder modularization | CLOSED | E-015 | HIGH | complete | 2 | complete | Encoder pin declarations; PoKeysLib struct | None | YES |
| Issue #216 | Missing PEv2 homing pin | OPEN | E-011 | HIGH | complete | 21 | complete | joint-in-homing absent; all 21 comments read (E-011); confirms homecomp load-order; pin naming | CONFLICT-002 | YES |
| Issue #222 | Physical pin-function setup | CLOSED | E-016 | HIGH | complete | 5 | complete | PK_PinConfigurationSet; PinFunction=0 device override | None | YES |
| Issue #223 | PEv2 LimitOverride | OPEN | None | HIGH | complete | 0 | none | LimitOverride linked to AXIS Override | None | YES |
| Issue #264 | Analog output userspace | CLOSED | E-017 | HIGH | empty | 1 | complete | Body empty; uspace supports analog output | CONFLICT-004 | YES |
| Issue #310 | PEv2 AxesState and name length | CLOSED | E-004 | HIGH | complete | 14 | complete | All 14 comments read (E-004); exact error: length 50>47 for Enable4xSampling; CONFLICT-005 | CONFLICT-002,CONFLICT-005 | YES |
| Issue #326 | HAL param name > limit | OPEN | E-005 | HIGH | complete | 12 | complete | All 12 comments read (E-005); same name-length issue; CI failures; rename fix; CONFLICT-005 | CONFLICT-005 | YES |

---

## Notes

1. Body status vocabulary: complete | partial | empty | title-only | not-inspected
2. Comment status vocabulary: complete | partial | none | not-inspected
3. CLOSED status not treated as proof of complete implementation.
4. homecomp joint.N.* pins are owned by pokeys_homecomp; they are signal endpoints.
5. PR #29 is a pull request, explicitly labeled.
6. Source ID None: no source-register entry exists for this issue. This represents
   a source-register coverage gap, not a validation defect. Retained HIGH/MEDIUM
   issues may legitimately have Source ID None.
7. Source IDs are assigned using the tuple (repository, object-type, number).
   A source ID from zarfld/pokeysHal must NEVER be attached to a row in the
   LinuxCnc_PokeysLibComp section, and vice versa, even if the issue numbers
   happen to coincide. Specifically:
   - C-012 = zarfld/pokeysHal #24; must not appear in the LC_PKComp section.
   - D-003 = zarfld/pokeysHal #129; must not appear in the LC_PKComp section.
