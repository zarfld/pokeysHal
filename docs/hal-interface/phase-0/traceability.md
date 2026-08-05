# Traceability — Phase 0 HAL-Interface Baseline

Evidence from pokeysHal@cd1f0dc8a0f64f92dc6bdce21bddcb36d33a14cd.

Legend:
- `→` complete, traceable link
- `?→` link exists but evidence is incomplete or indirect
- `✗` broken or missing link
- `[CONFLICT-N]` link is contradicted by a registered conflict

---

## 1. Digital Input (digin)

```
LinuxCNC CDI: digin.in (bit, HAL_OUT), digin.in-not (bit, HAL_OUT) [A-002]
  → hal-canon hal_export_digin, hal_digin_t [B-002]
  → issue #38: HAL-Interface: Digital Input [C-007] (OPEN, body=complete, 0 comments)
  ?→ no ADR/SDD found for digital input
  → hal_export_digin called: PoKeysLibIOAsync.c:35
  → update: PK_DigitalIOGetParse, PoKeysLibIOAsync.c:479-480
  ✗ future verification target: no TEST issue linked
```

**Note:** The extra non-canonical `digin.J.invert` parameter (PoKeysLibIOAsync.c:43)
has no traceability chain to any requirement or ADR.

---

## 2. Digital Output (digout)

```
LinuxCNC CDI: digout.out (bit, HAL_IN), digout.invert (bit, HAL_PARAM_RW) [A-002]
  → hal-canon hal_export_digout, hal_digout_t [B-002]
  → issue #36: HAL-Interface: Digital Output [C-005] (OPEN, body=complete, 0 comments)
  ?→ no ADR/SDD found
  → hal_export_digout called: PoKeysLibIOAsync.c:53
  → update: PK_DigitalIOSetAsync, PoKeysLibIOAsync.c:~385-465
  ✗ future verification target: no TEST issue linked
```

---

## 3. Analog Input (adcin)

```
LinuxCNC CDI: adcin.value (float, HAL_OUT), adcin.scale, adcin.offset,
              adcin.bit-weight, adcin.hw-offset [A-002]
  → hal-canon hal_export_adcin, hal_adcin_t [B-003]
  → issue #35: HAL-Interface: Analog Input [C-004] (CLOSED)
  ✗ ADR/SDD: none found
  → implementation: hal_export_adcin IS called at PoKeysLibIOAsync.c:85
      adcin.J.value PRESENT (but as HAL_IN — direction bug in hal-canon, CONFLICT-009)
      adcin.J.scale, offset, bit-weight, hw-offset: PRESENT via hal_export_adcin
  ???? non-canonical supplementary pins also exported:
      adcin.J.in.hw (u32, HAL_OUT) — PoKeysLibIOAsync.c:76
      adcin.J.in.raw (float, HAL_OUT) — PoKeysLibIOAsync.c:83
      adcin.J.ReferenceVoltage (param, HAL_RO) — PoKeysLibIOAsync.c:90
  ✗ 'value-raw' name from issue #35 not present; 'in.raw' used instead (CONFLICT-003)
  ✗ future verification target: none linked
```

---

## 4. Analog Output / PWM (adcout)

```
LinuxCNC CDI: adcout.value (float, HAL_IN), adcout.enable (bit, HAL_IN),
              adcout.scale, adcout.offset, adcout.high-limit, etc. [A-002]
  → hal-canon hal_export_adcout, hal_adcout_t [B-003]
  → issues #37, #39: HAL-Interface: Analog Output [C-006, C-008] (both OPEN)
  ✗ ADR/SDD: none found
  → implementation: hal_export_adcout IS called at PoKeysLibIOAsync.c:110
      adcout.J.value (HAL_IN — correct), adcout.J.enable (HAL_IN — correct)
      all canonical parameters PRESENT via hal_export_adcout
  ???? non-canonical supplementary pins also exported:
        adcout.J.PWMduty (u32, HAL_OUT) — PoKeysLibIOAsync.c:124
        adcout.J.max_voltage (param, HAL_RW) — PoKeysLibIOAsync.c:117
        adcout.pwm.period (param, HAL_RW) — PoKeysLibIOAsync.c:132
  ✓ adcout.J.value functional conversion path IMPLEMENTED (code inspection); HIL hardware pending
  ✓ canonical adcout objects exported; conversion IMPLEMENTED per C4 trace
  ?→ issues #37/#39 remain OPEN; HIL confirmation pending
  ✓ legacy integration files reference adcout.0.value and adcout.0.enable (E-001)
  ✓ these pin names ARE present in current implementation
  ✗ future verification target: none
```

---

## 5. Encoder (hal-canon convention, not official CDI)

```
Official LinuxCNC CDI (canonical-devices.html, Authority A): does NOT define encoder
  as a canonical device type. CDI covers digin, digout, adcin, adcout only.
  hal-canon hal_export_encoder, hal_encoder_t [B-004] is a hal-canon convention.
  → issue #42 (CLOSED): Encoder [C-022] (body=complete, 0 comments; encoder cardinality stated as 29; active interface: count, position, reset, scale; struck through: velocity, index-enable)
  ✗ no ADR/SDD found
  → manually exported (not via hal_export_encoder):
      PoKeysLibEncodersAsync.c:43-71 (count, position, velocity, reset, index-enable, scale)
  ✗ non-canonical extensions exported without traceability:
      encoderOptions, enable, x4_sampling, x2_sampling, channelApin, channelBpin
      fast.Configuration, fast.Options, fast.enable, fast.invert_E1/E2/E3, fast.disable_4x
      ultra.Configuration, ultra.Options, ultra.Filter
  ✗ future verification target: none
```

---

## 6. PEv2 Global Interface

```
PoKeys-specific requirement:
  issue #33 (pokeysHal, CLOSED): HAL Interface: PEv2 global pins [C-002]
  → issue #116: REQ-F-PEV2-001: Global PEv2 HAL Pin Export [C-009] (OPEN)
  → issue #132: ARC-C-PEV2 [D-006] (OPEN, body=complete, 0 comments)
  → issue #133: SDD-PEV2 [D-007] (OPEN, body=complete, 0 comments)
  → implementation: export_pev2_pins(), PoKeysLibPulseEngine_v2Async.c:373-715
  → PEv2.nrOfAxes, PulseEngineState, PulseEngineActivated: implemented
  ✗ PulseEngineEnabled, PulseGeneratorType, MiscInputStatus, LimitOverride,
    AxisEnabledMask, HomingStartMaskSetup, ProbeStartMaskSetup, ProbeSpeed,
    BacklashCompensationEnabled: ABSENT [CONFLICT-007]
  ✗ future verification target: none linked
```

---

## 7. PEv2 Per-Axis Interface

```
PoKeys-specific requirement:
  issue #33 (CLOSED): per-axis pins [C-002]
  → issue #117: REQ-F-PEV2-002: Per-Axis PEv2 HAL Pin Export [C-010] (OPEN)
  → issue #118: REQ-F-PEV2-003: Conditional Pin Creation [C-011] (OPEN)
  → issue #128: ADR-PEV2-002: nrOfAxes-based creation [D-001] (OPEN, Accepted)
  [CONFLICT-001]: REQ says nrOfAxes==0 → no pins; ADR says → 8 axes fallback;
    implementation always creates 8 (PoKeysLibPulseEngine_v2Async.c:380)
  → joint-pos-cmd, joint-pos-fb, joint-vel-cmd: implemented
  → AxesState, AxesCommand, CurrentPosition: implemented
  → digin.LimitN/LimitP/Home.in/.in-not: implemented
  → HomingStatus, index-enable: both endpoints exist, but the runtime propagation semantics remain untraced (PEV2A-007 / HOMECOMP-005 / IK-002)
  ✗ digout.AxisEnable.out: ABSENT [CONFLICT-002]
  ✗ stepgen.* parameters: ABSENT [CONFLICT-007]
  ✗ HomingSpeed, HomingReturnSpeed, HomingAlgorithm, HomeOffsets: ABSENT
  ✗ SoftLimitMaximum, SoftLimitMinimum: ABSENT
  ✗ per-axis AxesConfig/AxesSwitchConfig bitmap decomposition:
    struct fields defined (PoKeysLibHal.h:405-422) but not exported as HAL pins
    (ADR-PEV2-003 #129 covers decomposition strategy — not yet implemented)
  ✗ future verification target: none linked
```

---

## 8. Physical Pin Configuration (PinFunction)

```
PoKeys protocol requirement:
  PoKeys protocol spec (not inspected in Phase 0) defines PinFunction byte
  → ePK_PinCap enumeration (PoKeysLib.h:72-82) [F-002]
  → pins.J.PinFunction param: PoKeysLibIOAsync.c:26
  → PK_StartPinFunctionsRequestAsync: reads PinFunction from device
  → PK_PinFunctionsSetAsync: writes PinFunction to device
  ✗ no requirements issue found for PinFunction HAL exposure
  ✗ no ADR found for PinFunction exposure strategy
  ✗ future verification target: none
```

---

## 9. Component Lifecycle

```
LinuxCNC normative rule: hal_init → pin export → hal_ready → RT function → hal_exit [A-001]
  ✗ no explicit stakeholder requirement referencing lifecycle steps
  → implemented: experimental/pokeys_async.c:156 (hal_init), :210 (hal_ready),
    :208,216 (hal_exit), :137 (hal_export_funct)
  → hal_malloc used throughout: correct per A-001
  ✗ hal_free correctly absent: resolved by issue #24 (CLOSED)
  ✗ future verification target: none linked
```

---

## 10. Device Information Pins

```
PoKeys-specific stakeholder requirement:
  issue #32 (pokeysHal, OPEN): PoKeys device information [C-001]
  ✗ no ADR/SDD found
  ✗ implementation: devSerial (HAL_IN u32), alive (HAL_OUT bit) — partial only
  ✗ connected, err, info.* (~35 pins) ABSENT
  ✗ future verification target: none linked
```

---

## 11. PoExtBus

```
PoKeys-specific requirement:
  issue #34 (pokeysHal, OPEN): PoExtBus HAL interface [C-003]
  ✗ no ADR/SDD found
  ✗ implementation: ABSENT — no HAL pin creation for PoExtBus in any file
  ✗ future verification target: none linked
```

---

## Summary of Broken/Missing Links

| Subsystem | Missing Link Type |
|---|---|
| adcin | Canonical exported; direction mismatch (CONFLICT-009); name mismatch value-raw/in.raw |
| adcout | Canonical exported; conversion implemented; HIL pending (CONFLICT-004) |
| PEv2 global | Requirement→Implementation (many pins from #33 absent) |
| PEv2 per-axis | Requirement→Implementation (digout.AxisEnable.out, stepgen.*, homing params) |
| PEv2 conditional creation | Requirement↔ADR conflict (#118 vs #128) |
| Device info | Requirement→Implementation (info.* pins absent) |
| PoExtBus | Requirement→Implementation (entirely absent) |
| All subsystems | Implementation→Test (no TEST issues linked to any subsystem) |
| hal-canon submodule | .gitmodules↔git-object-model (CONFLICT-008); upstream commit now known (995d705) |
| Component naming | Legacy integration↔Current implementation (CONFLICT-006) |
| CDI terminology | Issue #79 extends CDI scope beyond official 4 types (CONFLICT-010) |

---

## 12. CDI Terminology Scope (CONFLICT-010)

```
Official LinuxCNC CDI (A-002, commit 71bf88009d64fa15edbebf9250b65ee4454f9a05):
  digin, digout, adcin, adcout — exactly four types.
  ?→ LinuxCnc_PokeysLibComp issue #79 extends the term to PEv2, encoders,
     counters, PWM, PoNET — no primary source supports these as CDI types.
  ✗ Phase 0 documents corrected in round 4 to remove encoder from CDI scope.
  ✗ PEv2 pins classified as PoKeys-specific or project compatibility contracts.
  ✗ future verification: CDI compliance test scope must be limited to four types.
  → CONFLICT-010 and DEC-CONFLICT010 registered.
```

---

---


---

## Homecomp Counterpart ABI Boundary

The records in Sections 13 and 14 below describe the `pokeys_homecomp` counterpart
ABI. `pokeys_homecomp` is a separate, independently loaded LinuxCNC component
(source E-010). It is not a pokeysHal subsystem, not implemented by pokeysHal,
and does not share lifecycle management with pokeysHal.

- `joint.N.*` pins are created and owned by `pokeys_homecomp` exclusively.
- They are not pokeysHal exports.
- Connection to the PoKeys-side endpoints is done by the machine integrator
  through HAL configuration (`net` statements).
- Integration wiring is NOT automatically generated by pokeysHal.
- The legacy PoKeys component (`pokeys.comp`, E-009) is a PoKeys-side component;
  it is not an AxesCommand producer.
- See integration-links.yaml (IK-001..IK-004) for the authoritative integration
  contracts.


## 13. AxesState Traceability

```
ESTABLISHED DECLARATIONS (authoritative sources):

(A) Legacy component — PokeysCompPulsEngine_base.c (E-009, LA-012):
    hal_pin_u32_newf(HAL_OUT, ..., "%s.PEv2.%01d.AxesState")
    Type:      hal_u32_t
    Direction: HAL_OUT  — legacy component is the PRODUCER
    Source:    legacy-pev2-parity.yaml LA-012, function_name=hal_pin_u32_newf,
               direction_or_access=HAL_OUT

(B) Current pokeysHal — PoKeysLibPulseEngine_v2Async.c:414 (F-005):
    hal_pin_u32_newf(HAL_OUT, &pev2->pin_AxesState[i], comp_id,
                     "%s.PEv2.%01d.AxesState", prefix, i)
    Type:      hal_u32_t
    Direction: HAL_OUT  — current pokeysHal is the PRODUCER
    Runtime update: *pev2->pin_AxesState[i] = pev2->AxesState[i]
                    at PoKeysLibPulseEngine_v2Async.c:833
    Reachability: PK_PEv2_StatusGetAsync sends request; response handler
                  invokes the update. Reachable via async_dispatcher from
                  FUNCTION(_). COMPILED/REFERENCED — full RT call path traced
                  to parse handler but HAL write at line 833 inside
                  PK_PEv2_StatusGetParse not independently confirmed as HIL-
                  verified.

(C) homecomp — pokeys_rt/pokeys_homecomp.comp (E-010, HOMECOMP-006):
    hal_pin_u32_newf(HAL_IN, &(addr->PEv2_AxesState), id,
                     "joint.%d.PEv2.AxesState", jno)
    Type:      hal_u32_t
    Direction: HAL_IN   — homecomp is the CONSUMER
    Source:    requirement-catalogue.yaml HOMECOMP-006,
               evidence: pokeys_rt/pokeys_homecomp.comp

HAL integration wiring (expected, not inspected in pokeysHal):
    net pev2-axes-state-J  pokeys-async.0.PEv2.J.AxesState => joint.J.PEv2.AxesState
    Producer (HAL_OUT): pokeysHal
    Consumer (HAL_IN):  homecomp
    Configuration file: integration HAL file (not present in pokeysHal repo).
    This connection is defined by the integrator; pokeysHal does not generate it.

Signal flow:
    device hardware
    → PK_PEv2_StatusGetAsync / PK_PEv2_StatusGetParse (PoKeysLib)
    → device->PEv2.AxesState[i] (in-memory cache)
    → pev2->pin_AxesState[i] (pokeysHal HAL_OUT, from parse handler)
    → HAL signal (configured by integration file)
    → addr->PEv2_AxesState (homecomp HAL_IN, read each servo cycle)

Enum compatibility:
    Legacy enum (ePK_PEAxisState, device values):
      {0,1,2,8,9,10,11,12,13,14,15,16,20,30}
    Legacy userspace extended (PEAxisStateEx):
      {3=ReadyToFinalizeHoming, 4=ReadyToArmEncoder, 17-19 extended}
    homecomp enum (pokeys_home_state_t):
      {0=STOPPED,1=READY,2=RUNNING,8-16=homing states,20=ERROR,30=LIMIT}
    Numeric overlap between device values and extended values: NONE FOUND
    in inspected sources.
    UNVERIFIED: Whether the device firmware ever emits extended PEAxisStateEx
    values, and whether homecomp handles values not in pokeys_home_state_t.
    No open decision or conflict currently covers this gap; register one if
    Phase 1 evidence shows discrepancy.
    Name mismatch (legacy prefix vs current prefix): CONFLICT-001, CONFLICT-006.
```

---

## 14. AxesCommand Traceability

```
ESTABLISHED DECLARATIONS (authoritative sources):

(A) Legacy component — PokeysCompPulsEngine_base.c (E-009, LA-013):
    hal_pin_u32_newf(HAL_IN, ..., "%s.PEv2.%01d.AxesCommand")
    Type:      hal_u32_t
    Direction: HAL_IN   — legacy component is the CONSUMER
    Source:    legacy-pev2-parity.yaml LA-013, function_name=hal_pin_u32_newf,
               direction_or_access=HAL_IN
    Enum (PK_PEAxisCommand, PoKeysComp.h L931-937):
      IDLE=0, HOMINGSTART=1, ARMENCODER=2, HOMINGWaitFinalMove=3,
      HOMINGFinalMove=4, HOMINGCancel=5, HOMINGFinalize=6  (7 values)
    Export reads PEv2_data->PEv2_AxesCommand[j]; call paths from the
    pin read to PK_PEv2_PulseEngineMove2 or SetParameter:
    DEFINED/REFERENCED BUT REACHABILITY UNVERIFIED — inspected source
    contains PEv2_data field reference at LA-013 creation; full update-cycle
    call path to a PoKeysLib send function was not traced to a primary source.

(B) homecomp — pokeys_rt/pokeys_homecomp.comp (E-010, HOMECOMP-007):
    hal_pin_u32_newf(HAL_OUT, &(addr->PEv2_AxesCommand), id,
                     "joint.%d.PEv2.AxesCommand", jno)
    Type:      hal_u32_t
    Direction: HAL_OUT  — homecomp is the PRODUCER
    Source:    requirement-catalogue.yaml HOMECOMP-007
    Enum (pokeys_home_command_t):
      IDLE=0, HOMINGSTART=1, HOMINGCANCEL=2, FINALIZE=3  (4 values)
    Homecomp FUNCTION(_) writes values 0, 1, 3 in documented paths.
    Value 2 (HOMINGCANCEL) is defined but its emit conditions were not
    fully traced.
    → CONFLICT-013: value 2 is ARMENCODER in the legacy 7-value enum;
      value 3 maps to HOMINGWaitFinalMove (legacy) ≠ FINALIZE (homecomp).

(C) Current pokeysHal — PoKeysLibPulseEngine_v2Async.c:421 (F-005):
    hal_pin_u32_newf(HAL_IN, &pev2->pin_AxesCommand[i], comp_id,
                     "%s.PEv2.%01d.AxesCommand", prefix, i)
    Type:      hal_u32_t
    Direction: HAL_IN   — current pokeysHal is the CONSUMER
    FUNCTION(_) calls: PK_ReceiveAndDispatch, PK_TimeoutAndRetryCheck,
                       async_dispatcher, update_ponet_hal_pins
                       (experimental/pokeys_async.c:1014–1070, inspected)
    rt_read_command_pins and rt_handle_homing_commands are declared
    (experimental/pokeys_async.c:356,359) but are NOT called from FUNCTION(_).
    → AxesCommand HAL_IN pin has no reachable consumer in the normal RT cycle.
    → CONFLICT-014: no reachable forwarding path demonstrated.
    Test mode (experimental/pokeys_async.c:891-892): writes |= 0x01 to
    pin_AxesCommand[0]; this violates HAL_IN direction.
    Open decisions: DEC-AXESCMD-001 (enum reconciliation), DEC-AXESCMD-002
                    (implement or remove forwarding path).

HAL integration wiring (expected, not inspected in pokeysHal):
    net pev2-axes-cmd-J  joint.J.PEv2.AxesCommand => pokeys-async.0.PEv2.J.AxesCommand
    Producer (HAL_OUT): homecomp
    Consumer (HAL_IN):  pokeysHal
    Configuration file: integration HAL file (not present in pokeysHal repo).
    This connection is defined by the integrator; pokeysHal does not generate it.

Signal flow:
    homecomp FUNCTION(_)
    → addr->PEv2_AxesCommand (homecomp HAL_OUT)
    → HAL signal (configured by integration file)
    → pev2->pin_AxesCommand[i] (pokeysHal HAL_IN)
    → [NO REACHABLE CONSUMER in current FUNCTION(_)] → CONFLICT-014

Integration link: IK-003 (incompatible, CONFLICT-013, CONFLICT-014)
Requirements: HOMECOMP-007 (conflicts: CONFLICT-013, CONFLICT-014)
```
