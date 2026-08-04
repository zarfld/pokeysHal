# Issue Inventory — Phase 0 HAL-Interface Baseline

Evidence from pokeysHal@cd1f0dc8a0f64f92dc6bdce21bddcb36d33a14cd and
LinuxCnc_PokeysLibComp@0c058e6c. Inspected: 2026-08-03 through 2026-08-04.

---

## pokeysHal Issues

| Object | Title | State | Relevance | Body status | Comment count | Comment status | Evidence summary | Conflicts | Retain |
|---|---|---|---|---|---|---|---|---|---|
| Issue #24 | no hal_free | CLOSED | HIGH | complete | 0 | none | Confirmed: hal_free absent; resolved | None | YES |
| Issue #30 | ePK_RETURN_CODES | OPEN | LOW | complete | 0 | none | ePK_RETURN_CODES enum definition | None | NO |
| Issue #32 | HAL Interface: PoKeys device information | OPEN | HIGH | complete | 0 | none | Specifies ~35 device info pins; only devSerial/alive implemented | None | YES |
| Issue #33 | HAL Interface: PEv2 | CLOSED | HIGH | complete | 3 | complete | CLOSED but many pins absent; comments: lifecycle refs, PoKeysLib example | CONFLICT-007 | YES |
| Issue #34 | HAL Interface: PoExtBus | OPEN | HIGH | complete | 0 | none | Specifies PoExtBus digin/digout per pin/device; absent in pokeysHal | None | YES |
| Issue #35 | HAL-Interface: Analog Input | CLOSED | HIGH | complete | 1 | complete | Canonical adcin exported; direction bug CONFLICT-009; name mismatch in.raw vs value-raw | CONFLICT-003,CONFLICT-009 | YES |
| Issue #36 | HAL-Interface: Digital Output | OPEN | HIGH | complete | 0 | none | digout.J.out, PoExtBus digout, PEv2 AxisEnabled.out, LimitOverride.out specified | None | YES |
| Issue #37 | HAL-Interface: Analog Output | OPEN | HIGH | complete | 0 | none | References #39; canonical adcout exported; conversion implemented | CONFLICT-004 | YES |
| Issue #38 | HAL-Interface: Digital Input | OPEN | HIGH | complete | 0 | none | digin.J.in/in-not specified; CONFLICT-009 direction mismatch | CONFLICT-009 | YES |
| Issue #39 | HAL-Interface: Analog Output (spec) | OPEN | HIGH | complete | 2 | complete | Specifies adcout canonical interface; comments confirm PK_PWMConfigurationSetAsync | CONFLICT-004 | YES |
| Issue #41 | PEv2 async RT | OPEN | HIGH | complete | 0 | none | PEv2 RT implementation | CONFLICT-001 | YES |
| Issue #97 | Async Scheduler opts | OPEN | LOW | not-inspected | 0 | not-inspected | n/a | None | MAYBE |
| Issue #99-#107 | REQ-F/NF-SCHED-* | OPEN | LOW | not-inspected | 0 | not-inspected | scheduler not HAL pins | None | NO |
| Issue #108-#113 | ADR-SCHED-* | OPEN | LOW | not-inspected | 0 | not-inspected | scheduler not HAL pins | None | NO |
| Issue #116 | REQ-F-PEV2-001: Global PEv2 Export | OPEN | HIGH | complete | 0 | none | Specifies all global PEv2 output/input pins | None | YES |
| Issue #117 | REQ-F-PEV2-002: Per-Axis PEv2 Export | OPEN | HIGH | complete | 0 | none | Specifies per-axis pins J=0..7 | None | YES |
| Issue #118 | REQ-F-PEV2-003: Conditional Pin Creation | OPEN | HIGH | complete | 0 | none | nrOfAxes==0 scenario; see CONFLICT-001 | CONFLICT-001 | YES |
| Issue #119 | REQ-F-PEV2-004: Input Pin Update | OPEN | HIGH | complete | 0 | none | HAL_IN→PoKeysLib pointer mapping table | None | YES |
| Issue #120 | REQ-F-PEV2-005: Output Pin Update | OPEN | HIGH | complete | 0 | none | PoKeysLib pointer→HAL_OUT mapping table | None | YES |
| Issue #121 | REQ-F-PEV2-006: Bitmapped Decomposition | OPEN | HIGH | complete | 0 | none | AxesConfig/AxesSwitchConfig decomposition strategy | None | YES |
| Issue #122 | REQ-F-PEV2-007: HAL Parameter Export | OPEN | HIGH | complete | 0 | none | Emergency/probe/limit/soft-limit params | None | YES |
| Issue #123 | REQ-F-PEV2-008: Async Call Sequence | OPEN | HIGH | complete | 0 | none | Per-cycle read/write function sequence | None | YES |
| Issue #124 | REQ-NF-PEV2-001: RT Safety | OPEN | HIGH | complete | 0 | none | <2µs per function; no blocking in RT path | None | YES |
| Issue #125 | REQ-NF-PEV2-002: Interface Completeness | OPEN | HIGH | complete | 0 | none | 100% parity with #33 pin specification | None | YES |
| Issue #126 | REQ-NF-PEV2-003: Enumeration Usage | OPEN | MEDIUM | complete | 0 | none | No magic numbers in bitmask operations | None | YES |
| Issue #127 | ADR-PEV2-001: HAL Pin Naming | OPEN | HIGH | complete | 0 | none | Accepted; governs PEv2.N.* naming pattern | None | YES |
| Issue #128 | ADR-PEV2-002: nrOfAxes-based creation | OPEN | HIGH | complete | 0 | none | Accepted; nrOfAxes fallback = 8; conflicts with REQ #118 | CONFLICT-001 | YES |
| Issue #129 | ADR-PEV2-003: Bitmapped Decomposition | OPEN | HIGH | complete | 0 | none | Accepted; AxesConfig decomposition strategy | None | YES |
| Issue #130 | ADR-PEV2-004: Read/Write Phase Separation | OPEN | MEDIUM | complete | 0 | none | Accepted; per-cycle read/write sequence | None | YES |
| Issue #131 | ADR-PEV2-005: Struct Organisation | OPEN | MEDIUM | complete | 0 | none | Accepted; global vs per-axis struct layout | None | YES |
| Issue #132 | ARC-C-PEV2: PEv2 Component | OPEN | MEDIUM | complete | 0 | none | Specified; references ADRs #127-#131 | None | YES |
| Issue #133 | SDD-PEV2: Design Description | OPEN | HIGH | complete | 0 | none | Full PEv2 design spec | CONFLICT-007 | YES |

---

## LinuxCnc_PokeysLibComp Issues

| Object | Title | State | Relevance | Body status | Comment count | Comment status | Evidence summary | Conflicts | Retain |
|---|---|---|---|---|---|---|---|---|---|
| Issue #16 | pokeys_py SRS | OPEN | LOW | partial | 0 | none | SRS for pokeys_py; covers digital/analog I/O, PEv2 | None | LIMITED |
| Issue #21 | Custom homing interface | CLOSED | MEDIUM | complete | 1 | complete | Defines homecomp interface; comment: pokeys.comp not yet updated | None | YES |
| Issue #24 | homecomp compatibility | CLOSED | MEDIUM | complete | 1 | complete | Verify homecomp vs pokeys.comp; comment: pokeys.comp split to userspace | None | YES |
| Issue #28 | userspace functional equivalence | CLOSED | HIGH | complete | 0 | none | Verify pokeys_userspace == pokeys.comp functionality | None | YES |
| PR #29 | Add missing userspace functionalities | MERGED | HIGH | complete | 0 | none | Adds PWM, PoExtBus to userspace; merged 2024-09-27 | None | YES |
| Issue #30 | Create Tests for main branch | OPEN | LOW | complete | 0 | none | Test scope for pokeys.comp and userspace components | None | YES |
| Issue #31 | Create Tests for pokeys_py | OPEN | LOW | complete | 0 | none | Test scope for pokeys_py components | None | YES |
| Issue #69 | Modular architecture | OPEN | MEDIUM | complete | 0 | none | Refactor codebase into modular components | None | YES |
| Issue #73 | Align with LinuxCNC builds | OPEN | MEDIUM | complete | 0 | none | Build environment for LinuxCNC 2.8.x/2.9.x | None | YES |
| Issue #79 | CDI conformity | OPEN | HIGH | complete | 0 | none | Requires CDI conformity for I/O, PEv2, canonical naming | CONFLICT-010 | YES |
| Issue #129 | HAL-Interface: RealTimeClock | CLOSED | LOW | complete | 0 | none | Specifies rtc.* pins; matches pokeysHal implementation | None | MAYBE |
| Issue #157 | Common pin I/O handling | CLOSED | HIGH | complete | 0 | none | Combine I/O handling; references PoKeysLib interface | None | YES |
| Issue #213 | Encoder modularization | CLOSED | HIGH | complete | 2 | complete | Encoder pin declarations in pokeys.comp; comments: PoKeysLib struct | None | YES |
| Issue #216 | Missing PEv2 homing pin | OPEN | HIGH | complete | 21 | partial | joint-in-homing pin absent; 21 comments: homing pin fixes discussed | CONFLICT-002 | YES |
| Issue #222 | Physical pin-function setup | CLOSED | HIGH | complete | 5 | complete | PK_PinConfigurationSet(dev); PinFunction=0 means device overrides | None | YES |
| Issue #223 | PEv2 LimitOverride | OPEN | HIGH | complete | 0 | none | LimitOverride linked to AXIS Override Limits checkbox | None | YES |
| Issue #264 | Analog output userspace | CLOSED | HIGH | empty | 1 | complete | Body empty; comment: uspace already supports analog output | CONFLICT-004 | YES |
| Issue #310 | PEv2 AxesState and name length | CLOSED | HIGH | complete | 14 | partial | AxesState pin missing; name length violation; 14 comments: homing workaround | CONFLICT-002,CONFLICT-005 | YES |
| Issue #326 | HAL param name > limit | OPEN | HIGH | complete | 12 | partial | Name-length fix needed; 12 comments: fix by shortening, build pipeline fail | CONFLICT-005 | YES |

---

## Notes

1. Body status vocabulary: complete | partial | empty | title-only | not-inspected
2. Comment status vocabulary: complete | partial | none | not-inspected
3. Issues #99-#113 (scheduler-related): LOW HAL-interface relevance; bodies not-inspected.
4. CLOSED status not treated as proof of complete implementation.
5. Homecomp joint.N.* pins are owned by pokeys_homecomp; they are signal endpoints
   in integration wiring, not missing pokeysHal exports.
6. PR #29 is a pull request, not an issue.
