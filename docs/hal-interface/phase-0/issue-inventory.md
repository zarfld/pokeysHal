# Issue Inventory — Phase 0 HAL-Interface Baseline

Evidence from pokeysHal@cd1f0dc8a0f64f92dc6bdce21bddcb36d33a14cd and
LinuxCnc_PokeysLibComp@0c058e6c. Inspected: 2026-08-03.

---

## pokeysHal Issues

| # | Title | State | Relevance | Claimed implementation | Evidence found | Conflicts | Use as source? |
|---|---|---|---|---|---|---|---|
| #24 | there is no hal_free which should be used | CLOSED | HIGH — documents that hal_free does not exist in LinuxCNC | Resolved: no hal_free calls | Confirmed: no hal_free in any source file | None | YES — normative API fact |
| #32 | HAL Interface: PoKeys device information | OPEN | HIGH — specifies 35+ device info pins | None | Only devSerial and alive implemented | None | YES — unimplemented requirements |
| #33 | HAL Interface: PEv2 | CLOSED | HIGH — specifies full PEv2 global and per-axis pin set | Presumably complete (CLOSED) | Many specified pins absent (CONFLICT-007) | CONFLICT-007 | YES — but treat as incomplete; verify each pin |
| #34 | HAL Interface: PoExtBus | OPEN | MEDIUM — specifies PoExtBus HAL interface | None | No PoExtBus HAL pins found anywhere | None | YES — unimplemented |
| #35 | HAL-Interface: Analog Input | CLOSED | HIGH | Canonical adcin objects (value, scale, offset, bit-weight, hw-offset) ARE exported via hal_export_adcin (PoKeysLibIOAsync.c:85) | adcin.value has direction mismatch (HAL_IN, should be HAL_OUT; CONFLICT-009); name mismatch: issue specifies value-raw, implementation uses in.raw (CONFLICT-003) | CONFLICT-003, CONFLICT-009 | YES |
| #36 | HAL-Interface: Digital Output | OPEN | HIGH — specifies digout.J.out, PoExtBus digout, PEv2 digout.AxisEnabled.out, digout.LimitOverride.out | Partial | digout.J.out, digout.invert exported; PEv2.AxisEnabled.out ABSENT; PoExtBus digout ABSENT; LimitOverride.out not found | None | YES |
| #37 | HAL-Interface: Analog Output | OPEN | HIGH | Body retrieved: documentation update for issue #39; body references #39 | Body retrieved (references #39); comments not inspected | CONFLICT-004 | YES |
| #38 | HAL-Interface: Digital Input | OPEN | HIGH — specifies digin.J.in and digin.J.in-not | Partial | digin.in and digin.in-not exported via hal_export_digin but with direction mismatch (CONFLICT-009); extra non-canonical invert param | CONFLICT-009 | YES |
| #39 | HAL-Interface: Analog Output | OPEN | HIGH | Body retrieved: specifies adcout.J.value (HAL_IN float), adcout.J.enable (HAL_IN bit), adcout.J.offset/scale/high_limit/low_limit (params), adcout.J.max_v, adcout.pwm.period; Count 6 channels | Body retrieved; comments not inspected | CONFLICT-004 | YES |
| #41 | PEv2 async implementation to be able to run it in RT | OPEN | HIGH | Partial | export_pev2_pins exists but incomplete vs. #33 | CONFLICT-001,007 | YES — implementation status unclear |
| #97 | Async Scheduler optimizations | OPEN | LOW for HAL interface | None | Not inspected | None | Maybe |
| #99–#107 | REQ-F/NF-SCHED-*: Async Scheduler requirements | OPEN | LOW for HAL interface (scheduler not HAL pins) | None | Not inspected | None | NO for HAL interface phase |
| #108–#113 | ADR-SCHED-*/ARC-C-ASYNC-SCHED | OPEN | LOW for HAL interface | None | Not inspected | None | NO for HAL interface phase |
| #116 | REQ-F-PEV2-001: Global PEv2 HAL Pin Export | OPEN | HIGH | Partial | Global pins partially implemented | CONFLICT-007 | YES |
| #117 | REQ-F-PEV2-002: Per-Axis PEv2 HAL Pin Export | OPEN | HIGH | Partial | Per-axis pins always create 8 | CONFLICT-001 | YES |
| #118 | REQ-F-PEV2-003: Conditional Pin Creation — Only for Required and Enabled Axes | OPEN | HIGH | Not implemented | Implementation always creates 8 unconditionally | CONFLICT-001 | YES |
| #119 | REQ-F-PEV2-004: Input Pin Update | OPEN | HIGH | Body retrieved: specifies HAL→PoKeysLib pointer mapping table for all HAL_IN PEv2 pins | Body retrieved; comments not inspected | None | YES |
| #120 | REQ-F-PEV2-005: Output Pin Update | OPEN | HIGH | Body retrieved: specifies PoKeysLib pointer→HAL_OUT mapping table for all PEv2 output pins | Body retrieved; comments not inspected | None | YES |
| #121 | REQ-F-PEV2-006: Bitmapped Field Decomposition | OPEN | HIGH | Body retrieved: specifies AxesConfig[N] and AxesSwitchConfig[N] decomposition using ePK_PEv2_AxisConfig and ePK_PulseEngineV2_AxisSwitchOptions | Body retrieved; comments not inspected | None | YES |
| #122 | REQ-F-PEV2-007: HAL Parameter Export for Pin Assignments | OPEN | HIGH | Body retrieved: specifies global/per-axis params for emergency pin, probe pin, soft limits, home offset, limit pin IDs | Body retrieved; comments not inspected | None | YES |
| #123 | REQ-F-PEV2-008: PoKeysLib Async Function Call Sequence | OPEN | HIGH | Body retrieved: specifies per-cycle read/write sequence (StatusGet, Status2Get, etc.) | Body retrieved; comments not inspected | None | YES |
| #124 | REQ-NF-PEV2-001: Real-Time Safety | OPEN | HIGH | Body retrieved: <2µs worst-case per function; no blocking calls, malloc, or mutex in RT path | Body retrieved; comments not inspected | None | YES |
| #125 | REQ-NF-PEV2-002: Interface Completeness — 100% Parity | OPEN | HIGH | Body retrieved: 100% of specified PEv2 pins must be present | Body retrieved; comments not inspected | None | YES |
| #126 | REQ-NF-PEV2-003: Correct Enumeration Usage | OPEN | MEDIUM | Body retrieved | Body retrieved; comments not inspected | None | YES |
| #127 | ADR-PEV2-001: HAL Pin Naming Convention | OPEN | HIGH | n/a (ADR) | Not fully inspected | CONFLICT-006 | YES |
| #128 | ADR-PEV2-002: Axis-Conditional Pin Creation Based on nrOfAxes | OPEN | HIGH | n/a (ADR) | Inspected: fallback=8 but conflicts with REQ #118 | CONFLICT-001 | YES |
| #129 | ADR-PEV2-003: Bitmapped-to-HAL-Pin Decomposition Strategy | OPEN | HIGH | n/a (ADR) | Not fully inspected | None directly | YES |
| #130 | ADR-PEV2-004: Read/Write Phase Separation | OPEN | MEDIUM | n/a (ADR) | Not inspected | None | YES |
| #131 | ADR-PEV2-005: sPoKeysHalPEv2 Struct Organisation | OPEN | MEDIUM | n/a (ADR) | Not inspected | None | YES |
| #132 | ARC-C-PEV2: PulseEngine v2 HAL Interface Component | OPEN | MEDIUM | n/a (ARC) | Not inspected | None | YES |
| #133 | SDD-PEV2: Software Design Description — PulseEngine v2 | OPEN | HIGH | n/a (SDD) | Not inspected | CONFLICT-007 | YES |

---

## LinuxCnc_PokeysLibComp Issues

| # | Title | State | Relevance | Claimed implementation | Evidence found | Conflicts | Use as source? |
|---|---|---|---|---|---|---|---|
| #16 | pokeys_py | OPEN | LOW | n/a | Body retrieved (partial): Software Requirements Specification for pokeys_py covering digital I/O, analog I/O, PEv2, PoNET, PWM, homing support | None | Limited |
| #21 | custom homing interface | CLOSED | MEDIUM | n/a | Body retrieved: defines interface for pokeys_homecomp.comp; standardises communication between homing procedure and PoKeys components via PEv2 AxesState/AxesCommand | None | YES for homing context |
| #24 | homecomp compatibility | CLOSED | MEDIUM | n/a | Body retrieved: verify pokeys_homecomp.comp is compatible with pokeys.comp; checks devSerial, PulseEngineState, homing, homed, home_state, index_enable, PEv2_AxesState/Command/Config/SwitchConfig pin mappings | Body retrieved; comments not inspected | None | YES for homing context |
| #28 | userspace functional equivalence | CLOSED | HIGH | n/a | Body retrieved: verify pokeys_userspace contains all functionality of pokeys.comp; modularized to reduce comp size | None | YES |
| PR#29 | Add missing functionalities to pokeys_userspace | MERGED | HIGH (PR) | Body retrieved: adds PWM (pokeys_pwm.comp), PoExtBus (pokeys_poextbus.comp), and other missing modules; merged 2024-09-27 | Body retrieved; PR comments not inspected | None | YES — confirms userspace modularization complete |
| #30 | Create Tests for Currently Implemented Features | OPEN | LOW | n/a | Body retrieved: test scope covers pokeys.comp, pokeys_userspace, pokeys_homecomp; digital/analog I/O, PEv2, encoders; confirms expected legacy HAL pin names | Body retrieved; comments not inspected | None | YES — useful for Phase 1 compatibility test design |
| #31 | Create Tests for `pokeys_py` Component | OPEN | LOW | n/a | Body retrieved: test scope for pokeys_py digital I/O, analog I/O, PEv2 | Body retrieved; comments not inspected | None | YES for Phase 1 reference |
| #69 | modular architecture | OPEN | MEDIUM | n/a | Body retrieved: refactor codebase into modular components; isolate functionality per subsystem | None | YES |
| #73 | LinuxCNC build alignment | OPEN | MEDIUM | n/a | Body retrieved: align pokeys_py and components with LinuxCNC 2.8.x/2.9.x build environment; refactor code for Debian compatibility; set up modular CMakeLists/Makefiles | None | YES |
| #79 | conformity with LinuxCNC guidelines and canonical device interface | OPEN | HIGH | n/a | Inspected: body requires CDI conformity for digital/analog I/O, motion control (PEv2), and communication protocols; references canonical naming rules in LinuxCNC HAL handbook | None | YES — directly relevant to Phase 1 |
| #129 | realtime clock HAL interface | CLOSED | LOW | n/a | Body retrieved: specifies rtc.sec/min/hour/dow/dom/doy/month/year pins; matches current pokeysHal implementation | None | Maybe |
| #157 | common pin I/O handling | CLOSED | HIGH | n/a | Body retrieved: combine common Pin I/O handling in hal components; references PoKeysLib public interface for pin functions | None | YES |
| #213 | encoder modularization and manual HAL export | CLOSED | HIGH | n/a | Body retrieved: pin declarations from pokeys.comp header section; manual creation of count/position/velocity/reset/index-enable/scale pins in C; confirms encoder naming convention | None | YES |
| #216 | missing PEv2 homing pin | OPEN | HIGH | n/a | Inspected: reports "Pin 'pokeys.0.PEv2.0.joint-in-homing' does not exist" running Pokeys57CNC_DM542_XXYZ_mill example | CONFLICT-002 | YES |
| #222 | physical pin-function setup | CLOSED | HIGH | n/a | Inspected: issue proposes PK_PinConfigurationSet(dev) to apply PinFunction settings; setting pokeys.0.digout.33.out had no effect without prior PinFunction setup. Comments confirmed: PinFunction param at 0 means device values override; non-zero causes function to be applied on device | None | YES |
| #223 | PEv2 limit override | OPEN | HIGH | n/a | Body retrieved (partial): LimitOverride should be linked to Override Limits checkbox in AXIS view; references PoKeysLib PEv2 LimitOverride and LimitOverrideSetup | None | YES |
| #264 | analog output in userspace component | CLOSED | HIGH | n/a | Body empty via API | CONFLICT-004 | YES |
| #310 | missing PEv2 AxesState and HAL name length | CLOSED | HIGH | Fixed in legacy component (name shortened) | Confirmed: name length violation was real | CONFLICT-005 | YES — evidence of real name-length problem |
| #326 | HAL parameter name longer than LinuxCNC limit | OPEN | HIGH | Unfixed | Confirmed: 50-char name against limit of 47 | CONFLICT-005 | YES |

---

## Notes

1. Most LinuxCnc_PokeysLibComp issue bodies were retrieved in Phase 0;
   comments on most issues not inspected.
   Phase 1 compatibility work should inspect remaining comments for HIGH-relevance issues.

2. Closed issues in both repositories have been treated with caution:
   CLOSED status was not treated as proof of complete implementation.
   Issues #33 and #35 in pokeysHal are both CLOSED but their implementation
   is incomplete — verified by source code inspection.

3. The pokeysHal issues #116–#133 form a structured set of PEv2 requirements
   and design records that supersede or elaborate on issue #33. The exact
   relationship between #33 and the REQ-F-PEV2-* series needs clarification.
