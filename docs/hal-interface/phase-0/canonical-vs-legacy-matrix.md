# Canonical vs. Legacy Interface Matrix — Phase 0 HAL-Interface Baseline

Evidence from pokeysHal@cd1f0dc8a0f64f92dc6bdce21bddcb36d33a14cd and
LinuxCnc_PokeysLibComp@0c058e6c.

## Classification Key

- **canonical and compatible** — matches LinuxCNC CDI spec and is implemented
- **canonical but missing** — LinuxCNC CDI spec requires it; not implemented
- **legacy extension** — exists in LinuxCnc_PokeysLibComp; not in LinuxCNC CDI
- **PoKeys-specific extension** — neither canonical nor legacy; PoKeys-only
- **renamed** — similar purpose, different name vs. CDI or legacy
- **type mismatch** — object exists but wrong HAL type
- **direction mismatch** — object exists but wrong HAL direction
- **semantic mismatch** — name matches but meaning or units differ
- **implementation-only** — present in code but not in any requirement
- **obsolete** — superseded by another object
- **unresolved** — insufficient evidence to classify

---

| Interface item | Official LinuxCNC (A) | Legacy LinuxCnc_PokeysLibComp (E) | Current pokeysHal (F) | Classification | Decision required |
|---|---|---|---|---|---|
| `digin.<J>.in` (bit, HAL_OUT spec) | canonical digin.in | `pokeys.0.digin.N.in` | `pokeys-async.0.digin.N.in` via `hal_export_digin` | **direction mismatch** — hal-canon exports HAL_IN; spec requires HAL_OUT; unsupported writer (CONFLICT-009) | Fix hal-canon digin.in direction bug (MEDIUM) |
| `digin.<J>.in-not` (bit, HAL_OUT) | canonical digin.in-not | `pokeys.0.digin.N.in-not` (implied) | `pokeys-async.0.digin.N.in-not` via `hal_export_digin` | **canonical and compatible** | Same prefix decision |
| `digin.<J>.invert` (bit, HAL_PARAM_RW) | not canonical (canonical digin has no invert) | `pokeys.0.digin.N.invert` (via Python layer) | `pokeys-async.0.digin.N.invert` via `hal_param_bit_newf` | **PoKeys-specific extension** | Decide whether to keep or remove |
| `digout.<J>.out` (bit, HAL_IN spec) | canonical digout.out | `pokeys.0.digout.N.out` | `pokeys-async.0.digout.N.out` via `hal_export_digout` | **direction mismatch** — hal-canon exports HAL_OUT; spec requires HAL_IN; blocks command source wiring (CONFLICT-009) | Fix hal-canon digout.out direction bug (HIGH) |
| `digout.<J>.invert` (bit, HAL_PARAM_RW) | canonical digout.invert parameter | `pokeys.0.digout.N.invert` | `pokeys-async.0.digout.N.invert` via `hal_export_digout` | **canonical and compatible** | Prefix decision |
| `adcin.<J>.value` (float, HAL_OUT spec) | canonical adcin.value | `pokeys.0.adcin.J.value` (issue #35) | PRESENT via `hal_export_adcin` (PoKeysLibIOAsync.c:85) BUT exported as HAL_IN (bug) | **direction mismatch** — hal-canon exports HAL_IN; spec requires HAL_OUT; unsupported writer (CONFLICT-009) | Fix hal-canon adcin.value direction bug (MEDIUM) |
| `adcin.<J>.value-raw` (float, HAL_OUT) | not canonical | issue #35 specifies `adcin.J.value-raw` | `adcin.J.in.raw` (float, HAL_OUT) — **different name** | **renamed** (name mismatch with issue #35 spec) | Align name with issue #35 or use canonical `value` only |
| `adcin.<J>.in.hw` (u32, HAL_OUT) | not canonical | not in legacy spec | implemented in pokeysHal as raw ADC count | **PoKeys-specific extension** (non-canonical name and type) | Decide: keep as supplementary, rename, or remove |
| `adcin.<J>.scale` (float, HAL_PARAM_RW) | canonical adcin.scale | specified in issue #35 | PRESENT via `hal_export_adcin` | **canonical and compatible** | Default 0.0 from memset is not useful |
| `adcin.<J>.offset` (float, HAL_PARAM_RW) | canonical adcin.offset | specified in issue #35 | PRESENT via `hal_export_adcin` | **canonical and compatible** | |
| `adcin.<J>.bit-weight` (float, HAL_PARAM_RW) | canonical adcin.bit-weight | not specified in issue #35 | PRESENT via `hal_export_adcin` | **canonical and compatible** | |
| `adcin.<J>.hw-offset` (float, HAL_PARAM_RW) | canonical adcin.hw-offset | not specified in issue #35 | PRESENT via `hal_export_adcin` | **canonical and compatible** | |
| `adcin.<J>.ReferenceVoltage` (float, HAL_PARAM_RO) | not canonical | not in legacy | implemented | **PoKeys-specific extension** | Decide: keep or integrate into hw-offset semantics |
| `adcout.<J>.value` (float, HAL_IN) | canonical adcout.value | `pokeys.0.adcout.0.value` (legacy HAL file) | PRESENT via `hal_export_adcout` (PoKeysLibIOAsync.c:110). Correct direction (HAL_IN). Conversion IMPLEMENTED (PK_PWMUpdateAsync at 100 ms). | **canonical and compatible** — conversion implemented; HIL pending | Decide policy for supplementary PWM objects |
| `adcout.<J>.enable` (bit, HAL_IN) | canonical adcout.enable | `pokeys.0.adcout.0.enable` (legacy HAL file) | PRESENT via `hal_export_adcout`. Correct direction (HAL_IN). Enables/disables PWM output. | **canonical and compatible** | |
| `adcout.<J>.scale` (float, HAL_PARAM_RW) | canonical adcout.scale | not confirmed | PRESENT via `hal_export_adcout` | **canonical and compatible** | |
| `adcout.<J>.offset` (float, HAL_PARAM_RW) | canonical adcout.offset | not confirmed | PRESENT via `hal_export_adcout` | **canonical and compatible** | |
| `adcout.<J>.high-limit` (float, HAL_PARAM_RW) | canonical adcout.high-limit | not confirmed | PRESENT via `hal_export_adcout` | **canonical and compatible** | |
| `adcout.<J>.low-limit` (float, HAL_PARAM_RW) | canonical adcout.low-limit | not confirmed | PRESENT via `hal_export_adcout` | **canonical and compatible** | |
| `adcout.<J>.bit-weight` (float, HAL_PARAM_RW) | canonical adcout.bit-weight | not confirmed | PRESENT via `hal_export_adcout` | **canonical and compatible** | |
| `adcout.<J>.hw-offset` (float, HAL_PARAM_RW) | canonical adcout.hw-offset | not confirmed | PRESENT via `hal_export_adcout` | **canonical and compatible** | |
| `adcout.<J>.PWMduty` (u32, HAL_OUT) | not canonical | not in legacy CDI | implemented | **PoKeys-specific extension** (non-canonical PWM raw output) | Decide: keep supplementary or replace with canonical |
| `adcout.<J>.max_voltage` (float, HAL_PARAM_RW) | not canonical | not in legacy CDI | implemented | **PoKeys-specific extension** | Decide: keep or map to canonical hw-offset/scale |
| `adcout.pwm.period` (u32, HAL_PARAM_RW) | not canonical | not in legacy CDI | implemented | **PoKeys-specific extension** | Decide: keep |
| `encoder.<I>.count` (s32, HAL_OUT) | NOT in official LinuxCNC CDI (canonical-devices.html covers digin/digout/adcin/adcout only) | `pokeys.0.encoder.0.count` (legacy) | `pokeys-async.0.encoder.I.count` | **PoKeys-specific extension** matching hal-canon convention (B-004) — NOT Authority A canonical | Note: not normatively required by A |
| `encoder.<I>.position` (float, HAL_OUT) | not in official CDI | not confirmed | `pokeys-async.0.encoder.I.position` | **PoKeys-specific / hal-canon convention** | |
| `encoder.<I>.velocity` (float, HAL_OUT) | not in official CDI | not confirmed | `pokeys-async.0.encoder.I.velocity` | **PoKeys-specific / hal-canon convention** | |
| `encoder.<I>.reset` (bit, HAL_IN) | not in official CDI | not confirmed | `pokeys-async.0.encoder.I.reset` | **PoKeys-specific / hal-canon convention** | |
| `encoder.<I>.index-enable` (bit, HAL_IN) | not in official CDI | not confirmed | `pokeys-async.0.encoder.I.index-enable` | **PoKeys-specific / hal-canon convention** | |
| `encoder.<I>.scale` (float, HAL_PARAM_RW) | not in official CDI | not confirmed | `pokeys-async.0.encoder.I.scale` | **PoKeys-specific / hal-canon convention** | |
| `encoder.<I>.encoderOptions` (u32, HAL_PARAM_RW) | not canonical | not in legacy CDI | implemented | **PoKeys-specific extension** | Decide: keep |
| `encoder.<I>.enable` (bit, HAL_PARAM_RW) | not canonical (canonical uses index-enable) | not in legacy CDI | implemented | **PoKeys-specific extension** | Decide: keep or map to index-enable semantics |
| `encoder.<I>.x4_sampling` (bit, HAL_PARAM_RW) | not canonical | not in legacy CDI | implemented | **PoKeys-specific extension** | Decide: keep |
| `encoder.<I>.channelApin` (u32, HAL_PARAM_RW) | not canonical | not in legacy CDI | implemented | **PoKeys-specific extension** | Decide: keep |
| `encoder.fast.Configuration` (u32, HAL_PARAM_RW) | not canonical | `pokeys[0].encoder.UltraFastEncoder.Enable4xSampling` (too long — legacy) | `pokeys-async.0.encoder.fast.Configuration` | **renamed** (shorter than legacy, non-canonical) | Verify name length ≤47 chars |
| `encoder.fast.enable` etc. | not canonical | not in CDI | implemented | **PoKeys-specific extension** | Decide: keep |
| `encoder.ultra.Configuration` (u32, HAL_PARAM_RW) | not canonical | name violated HAL_NAME_LEN in legacy | `pokeys-async.0.encoder.ultra.Configuration` | **renamed** (improved, shorter) | Verify name length |
| `PEv2.nrOfAxes` (u32, HAL_OUT) | not canonical | not in CDI | implemented | **PoKeys-specific extension** | Keep |
| `PEv2.PulseEngineState` (u32, HAL_OUT) | not canonical | referenced in homing HAL | implemented | **PoKeys-specific extension** | Keep |
| `PEv2.<I>.joint-pos-cmd` (float, HAL_IN) | not canonical (motion controller pins are canonical) | `pokeys.0.PEv2.0.joint-pos-cmd` | implemented | **legacy extension** (compatible with legacy naming) | Prefix decision |
| `PEv2.<I>.joint-pos-fb` (float, HAL_OUT) | not canonical | `pokeys.0.PEv2.0.joint-pos-fb` | implemented | **legacy extension** | Prefix decision |
| `PEv2.<I>.AxesState` (u32, HAL_OUT) | not canonical | `pokeys.0.PEv2.0.AxesState` | implemented | **legacy extension** | Prefix decision |
| `PEv2.<I>.AxesCommand` (u32, HAL_IN) | not canonical | `pokeys.0.PEv2.0.AxesCommand` | implemented | **legacy extension** | Prefix decision |
| `PEv2.<I>.digin.LimitN.in` (bit, HAL_OUT) | not canonical | `pokeys.0.PEv2.0.digin.LimitN.in` | implemented | **legacy extension** | Prefix decision |
| `PEv2.<I>.digin.Home.in` (bit, HAL_OUT) | not canonical | `pokeys.0.PEv2.0.digin.Home.in` | implemented | **legacy extension** | Prefix decision |
| `PEv2.<I>.digout.AxisEnable.out` (bit, HAL_IN) | not canonical | `pokeys.0.PEv2.0.digout.AxisEnable.out` (legacy HAL) | **ABSENT** | **legacy extension — missing** | Critical for axis enable; must be added |
| `PEv2.<I>.HomingStatus` (u32, HAL_IO) | not canonical | referenced in homing HAL | implemented | **PoKeys-specific extension** | Keep |
| `PEv2.digin.Emergency.in` (bit, HAL_OUT) | not canonical | used in machine config | implemented | **PoKeys-specific extension** | Keep |
| `PEv2.digout.Emergency.out` (bit, HAL_IN) | not canonical | `pokeys.0.PEv2.digout.Emergency.out` | implemented | **legacy extension** | Prefix decision |
| `PEv2.digin.Probed.in` (bit, HAL_OUT) | not canonical | `pokeys.0.PEv2.digin.Probed.in` | implemented | **legacy extension** | Prefix decision |
| `PEv2.PulseEngineEnabled` (bit, HAL_OUT) | not canonical | issue #33 specifies | **ABSENT** | **legacy extension — missing** | Issue #33 closed but pin absent |
| `PEv2.MiscInputStatus` (u32, HAL_OUT) | not canonical | issue #33 specifies | **ABSENT** | **legacy extension — missing** | |
| `PEv2.LimitOverride` | not canonical | issue #33 specifies | **ABSENT** | **legacy extension — missing** | |
| `PEv2.AxisEnabledMask` (u32, HAL_OUT) | not canonical | issue #33 specifies | **ABSENT** | **legacy extension — missing** | |
| `PEv2.<I>.stepgen.*` (params) | not canonical | `pokeys.0.PEv2.0.stepgen.STEP-SCALE` etc. (legacy HAL) | **ABSENT** | **legacy extension — missing** | Referenced in integration files |
| `PEv2.<I>.HomingSpeed` etc. | not canonical | issue #33 specifies | **ABSENT** | **legacy extension — missing** | |
| `PoExtBus.*` | not canonical | issue #34 specifies | **ABSENT** | **legacy extension — missing** | |
| `rtc.sec/min/hour/...` (u32, HAL_OUT) | not canonical | issue #129 (LinuxCnc_PokeysLibComp) | implemented | **PoKeys-specific extension** | Keep |
| `PoNET.*` (u32 pins) | not canonical | not confirmed in legacy | implemented | **PoKeys-specific extension** | Keep |
| `pins.<J>.PinFunction` (u32, HAL_PARAM_RW) | not canonical | not in CDI | implemented | **PoKeys-specific configuration** | Keep; decide exposure strategy |
| `machine-is-on` (bit) | not canonical — this is a HAL signal not a component pin | `net machine-is-on => pokeys.0.machine-is-on` (legacy HAL) | **ABSENT** | **legacy extension — missing** | Determine if this is a pin on the legacy component or an external signal |
| `devSerial` (u32, HAL_IN) | not canonical | implemented in legacy .comp | implemented | **PoKeys-specific extension** | Keep |
| `alive` (bit, HAL_OUT) | not canonical | implemented in legacy .comp | implemented | **PoKeys-specific extension** | Keep |
| `connected` (bit, HAL_OUT) | not canonical | issue #32 | **ABSENT** | **PoKeys-specific — missing** | Add |
| `info.*` (u32, HAL_OUT × 35+) | not canonical | issue #32 | **ABSENT** | **PoKeys-specific — missing** | Decision on exposure scope |

---

## Summary Counts

| Classification | Count (approximate) |
|---|---|
| canonical and compatible | ~14 |
| direction mismatch (present, wrong direction) | 3 |
| canonical but missing | 3 |
| PoKeys-specific extension (present) | 20 |
| PoKeys-specific / hal-canon convention (encoder) | 6 |
| legacy extension (present) | 10 |
| renamed | 2 |
| legacy extension — missing | 10 |
| PoKeys-specific — missing | 4 |
| **Total HAL objects catalogued** | **~72** |
