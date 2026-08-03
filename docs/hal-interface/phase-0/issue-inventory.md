# Issue Inventory — Phase 0 HAL-Interface Baseline

Evidence from pokeysHal@cd1f0dc8a0f64f92dc6bdce21bddcb36d33a14cd and
LinuxCnc_PokeysLibComp@0c058e6c. Inspected: 2026-08-03 through 2026-08-04.

---

## pokeysHal Issues

| Object | Title | State | Relevance | Body status | Comment count | Comment status | Evidence summary | Conflicts | Retain |
|---|---|---|---|---|---|---|---|---|---|
| #24 | there is no hal_free which should be used | CLOSED | HIGH — documents that hal_free does not exist in LinuxCNC | not-inspected | 1 | complete | Resolved: no hal_free calls | Confirmed: no hal_free in any source file | YES — normative API fact |
| #32 | HAL Interface: PoKeys device information | OPEN | HIGH — specifies ~35 device info pins | not-inspected | 0 | none | None | Only devSerial and alive implemented | YES — unimplemented requirements |
| #33 | HAL Interface: PEv2 | CLOSED | HIGH — specifies full PEv2 global and per-axis pin set | complete | 3 | complete | Presumably complete (CLOSED) | Many specified pins absent (CONFLICT-007) | YES — but treat as incomplete; verify each pin |
| #34 | HAL Interface: PoExtBus | OPEN | MEDIUM — specifies PoExtBus HAL interface | not-inspected | 0 | none | None | No PoExtBus HAL pins found anywhere | YES — unimplemented |
| #35 | HAL-Interface: Analog Input | CLOSED | HIGH | complete | 1 | complete | Canonical adcin objects (value, scale, offset, bit-weight, hw-offset) ARE exported via hal_export_adcin (PoKeysLibIOAsync.c:85) | adcin.value has direction mismatch (HAL_IN, should be HAL_OUT; CONFLICT-009); name mismatch: issue specifies value-raw, implementation uses in.raw (CONFLICT-003) | YES |
| #36 | HAL-Interface: Digital Output | OPEN | HIGH — specifies digout.J.out, PoExtBus digout, PEv2 digout.AxisEnabled.out, digout.LimitOverride.out | not-inspected | 0 | none | Partial | digout.J.out, digout.invert exported; PEv2.AxisEnabled.out ABSENT; PoExtBus digout ABSENT; LimitOverride.out not found | YES |
| #37 | HAL-Interface: Analog Output | OPEN | HIGH | not-inspected | 0 | none | Body retrieved: documentation update for issue #39; body references #39 | Body retrieved (references #39); comments not inspected | YES |
| #38 | HAL-Interface: Digital Input | OPEN | HIGH — specifies digin.J.in and digin.J.in-not | not-inspected | 0 | none | Partial | digin.in and digin.in-not exported via hal_export_digin but with direction mismatch (CONFLICT-009); extra non-canonical invert param | YES |
| #39 | HAL-Interface: Analog Output | OPEN | HIGH | complete | 2 | complete | Body retrieved: specifies adcout.J.value (HAL_IN float), adcout.J.enable (HAL_IN bit), adcout.J.offset/scale/high_limit/low_limit (params), adcout.J.m | Body retrieved; comments not inspected | YES |
| #41 | PEv2 async implementation to be able to run it in RT | OPEN | HIGH | not-inspected | 0 | none | Partial | export_pev2_pins exists but incomplete vs. #33 | YES — implementation status unclear |
| #97 | Async Scheduler optimizations | OPEN | LOW for HAL interface | complete | 0 | none | None | Not inspected | Maybe |
| #99–#107 | REQ-F/NF-SCHED-*: Async Scheduler requirements | OPEN | LOW for HAL interface (scheduler not HAL pins) | complete | 0 | none | None | Not inspected | NO for HAL interface phase |
| #108–#113 | ADR-SCHED-*/ARC-C-ASYNC-SCHED | OPEN | LOW for HAL interface | complete | 0 | none | None | Not inspected | NO for HAL interface phase |
| #116 | REQ-F-PEV2-001: Global PEv2 HAL Pin Export | OPEN | HIGH | not-inspected | 0 | none | Partial | Global pins partially implemented | YES |
| #117 | REQ-F-PEV2-002: Per-Axis PEv2 HAL Pin Export | OPEN | HIGH | not-inspected | 0 | none | Partial | Per-axis pins always create 8 | YES |
| #118 | REQ-F-PEV2-003: Conditional Pin Creation — Only for Required and Enabled Axes | OPEN | HIGH | not-inspected | 0 | none | Not implemented | Implementation always creates 8 unconditionally | YES |
| #119 | REQ-F-PEV2-004: Input Pin Update | OPEN | HIGH | not-inspected | 0 | none | Body retrieved: specifies HAL→PoKeysLib pointer mapping table for all HAL_IN PEv2 pins | Body retrieved; comments not inspected | YES |
| #120 | REQ-F-PEV2-005: Output Pin Update | OPEN | HIGH | not-inspected | 0 | none | Body retrieved: specifies PoKeysLib pointer→HAL_OUT mapping table for all PEv2 output pins | Body retrieved; comments not inspected | YES |
| #121 | REQ-F-PEV2-006: Bitmapped Field Decomposition | OPEN | HIGH | not-inspected | 0 | none | Body retrieved: specifies AxesConfig[N] and AxesSwitchConfig[N] decomposition using ePK_PEv2_AxisConfig and ePK_PulseEngineV2_AxisSwitchOptions | Body retrieved; comments not inspected | YES |
| #122 | REQ-F-PEV2-007: HAL Parameter Export for Pin Assignments | OPEN | HIGH | not-inspected | 0 | none | Body retrieved: specifies global/per-axis params for emergency pin, probe pin, soft limits, home offset, limit pin IDs | Body retrieved; comments not inspected | YES |
| #123 | REQ-F-PEV2-008: PoKeysLib Async Function Call Sequence | OPEN | HIGH | not-inspected | 0 | none | Body retrieved: specifies per-cycle read/write sequence (StatusGet, Status2Get, etc.) | Body retrieved; comments not inspected | YES |
| #124 | REQ-NF-PEV2-001: Real-Time Safety | OPEN | HIGH | not-inspected | 0 | none | Body retrieved: <2µs worst-case per function; no blocking calls, malloc, or mutex in RT path | Body retrieved; comments not inspected | YES |
| #125 | REQ-NF-PEV2-002: Interface Completeness — 100% Parity | OPEN | HIGH | not-inspected | 0 | none | Body retrieved: 100% of specified PEv2 pins must be present | Body retrieved; comments not inspected | YES |
| #126 | REQ-NF-PEV2-003: Correct Enumeration Usage | OPEN | MEDIUM | not-inspected | 0 | none | Body retrieved | Body retrieved; comments not inspected | YES |
| #127 | ADR-PEV2-001: HAL Pin Naming Convention | OPEN | complete | not-inspected | 0 | none | HIGH (ADR) | Body retrieved: Accepted, governs %s.PEv2.N.* format | YES |
| #128 | ADR-PEV2-002: Axis-Conditional Pin Creation Based on nrOfAxes | OPEN | HIGH | not-inspected | 0 | none | n/a (ADR) | Inspected: fallback=8 but conflicts with REQ #118 | YES |
| #129 | ADR-PEV2-003: Bitmapped-to-HAL-Pin Decomposition Strategy | OPEN | complete | not-inspected | 0 | none | HIGH (ADR) | Body retrieved: Accepted, AxesConfig/AxesSwitchConfig decomposition strategy | YES |
| #130 | ADR-PEV2-004: Read/Write Phase Separation | OPEN | complete | not-inspected | 0 | none | MEDIUM (ADR) | Body retrieved: Accepted, per-cycle read/write sequence | YES |
| #131 | ADR-PEV2-005: sPoKeysHalPEv2 Struct Organisation | OPEN | complete | not-inspected | 0 | none | MEDIUM (ADR) | Body retrieved: Accepted, global vs per-axis struct layout | YES |
| #132 | ARC-C-PEV2: PulseEngine v2 HAL Interface Component | OPEN | complete | not-inspected | 0 | none | MEDIUM (ARC) | Body retrieved: Status Specified, references ADRs #127-#131 | YES |
| #133 | SDD-PEV2: Software Design Description | OPEN | complete | not-inspected | 0 | none | HIGH (SDD) | Body retrieved: full design spec for PEv2 interface | YES |

---

## LinuxCnc_PokeysLibComp Issues

| Object | Title | State | Relevance | Body status | Comment count | Comment status | Evidence summary | Conflicts | Retain |
|---|---|---|---|---|---|---|---|---|---|
| #16 | pokeys_py | OPEN | LOW | not-inspected | 0 | none | n/a | Body retrieved (partial): Software Requirements Specification for pokeys_py covering digital I/O, analog I/O, PEv2, PoNET, PWM, homing support | Limited |
| #21 | custom homing interface | CLOSED | MEDIUM | complete | 1 | complete | n/a | Body retrieved: defines interface for pokeys_homecomp.comp; standardises communication between homing procedure and PoKeys components via PEv2 AxesState/AxesCommand | YES for homing context |
| #24 | homecomp compatibility | CLOSED | MEDIUM | not-inspected | 1 | complete | Body retrieved: verify pokeys_homecomp.comp is compatible with pokeys.comp; checks devSerial, PulseEngineState, homing, homed, home_state, index_enabl | Body retrieved; comments not inspected | YES for homing context |
| #28 | userspace functional equivalence | CLOSED | HIGH | not-inspected | 0 | none | n/a | Body retrieved: verify pokeys_userspace contains all functionality of pokeys.comp; modularized to reduce comp size | YES |
| PR#29 | Add missing functionalities to pokeys_userspace | MERGED | HIGH (PR) | not-inspected | 0 | none | Body retrieved: adds PWM (pokeys_pwm.comp), PoExtBus (pokeys_poextbus.comp), and other missing modules; merged 2024-09-27 | Body retrieved; PR comments not inspected | YES — confirms userspace modularization complete |
| #30 | Create Tests for Currently Implemented Features | OPEN | LOW | not-inspected | 0 | none | Body retrieved: test scope covers pokeys.comp, pokeys_userspace, pokeys_homecomp; digital/analog I/O, PEv2, encoders; confirms expected legacy HAL pin | Body retrieved; comments not inspected | YES — useful for Phase 1 compatibility test design |
| #31 | Create Tests for `pokeys_py` Component | OPEN | LOW | not-inspected | 0 | none | Body retrieved: test scope for pokeys_py digital I/O, analog I/O, PEv2 | Body retrieved; comments not inspected | YES for Phase 1 reference |
| #69 | modular architecture | OPEN | MEDIUM | not-inspected | 0 | none | n/a | Body retrieved: refactor codebase into modular components; isolate functionality per subsystem | YES |
| #73 | LinuxCNC build alignment | OPEN | MEDIUM | not-inspected | 0 | none | n/a | Body retrieved: align pokeys_py and components with LinuxCNC 2.8.x/2.9.x build environment; refactor code for Debian compatibility; set up modular CMakeLists/Makefiles | YES |
| #79 | conformity with LinuxCNC guidelines and canonical device interface | OPEN | HIGH | not-inspected | 0 | none | n/a | Inspected: body requires CDI conformity for digital/analog I/O, motion control (PEv2), and communication protocols; references canonical naming rules in LinuxCNC HAL handbook | YES — directly relevant to Phase 1 |
| #129 | realtime clock HAL interface | CLOSED | LOW | not-inspected | 0 | none | n/a | Body retrieved: specifies rtc.sec/min/hour/dow/dom/doy/month/year pins; matches current pokeysHal implementation | Maybe |
| #157 | common pin I/O handling | CLOSED | HIGH | not-inspected | 0 | none | n/a | Body retrieved: combine common Pin I/O handling in hal components; references PoKeysLib public interface for pin functions | YES |
| #213 | encoder modularization and manual HAL export | CLOSED | HIGH | complete | 2 | complete | n/a | Body retrieved: pin declarations from pokeys.comp header section; manual creation of count/position/velocity/reset/index-enable/scale pins in C; confirms encoder naming convention | YES |
| #216 | missing PEv2 homing pin | OPEN | HIGH | complete | 21 | partial | n/a | Inspected: reports "Pin 'pokeys.0.PEv2.0.joint-in-homing' does not exist" running Pokeys57CNC_DM542_XXYZ_mill example | YES |
| #222 | physical pin-function setup | CLOSED | HIGH | complete | 5 | complete | n/a | Inspected: issue proposes PK_PinConfigurationSet(dev) to apply PinFunction settings; setting pokeys.0.digout.33.out had no effect without prior PinFunction setup. Comments confirmed: PinFunction param at 0 means device values override; non-zero causes function to be applied on device | YES |
| #223 | PEv2 limit override | OPEN | HIGH | not-inspected | 0 | none | n/a | Body retrieved (partial): LimitOverride should be linked to Override Limits checkbox in AXIS view; references PoKeysLib PEv2 LimitOverride and LimitOverrideSetup | YES |
| #264 | analog output in userspace component | CLOSED | HIGH | complete | 1 | complete | n/a | Body empty via API | YES |
| #310 | missing PEv2 AxesState and HAL name length | CLOSED | HIGH | complete | 14 | partial | Fixed in legacy component (name shortened) | Confirmed: name length violation was real | YES — evidence of real name-length problem |
| #326 | HAL parameter name longer than LinuxCNC limit | OPEN | HIGH | complete | 12 | partial | Unfixed | Confirmed: 50-char name against limit of 47 | YES |

---

## Notes

1. Body status vocabulary: complete | partial | empty | title-only | not-inspected.
2. Comment status vocabulary: complete | partial | none | not-inspected.
3. Issues #99–#113 (scheduler-related) have LOW HAL-interface relevance and bodies not inspected.
4. CLOSED status was not treated as proof of complete implementation.
5. Homecomp joint.N.* pins are owned by pokeys_homecomp; they are signal endpoints for
   integration wiring, not missing pokeysHal exports.
