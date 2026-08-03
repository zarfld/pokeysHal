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
| `digin.<J>.in` (bit, HAL_OUT) | canonical digin.in | `pokeys.0.digin.N.in` | `pokeys-async.0.digin.N.in` via `hal_export_digin` | **canonical and compatible** — but component prefix differs | Decide canonical component name: `pokeys` or `pokeys-async` |
| `digin.<J>.in-not` (bit, HAL_OUT) | canonical digin.in-not | `pokeys.0.digin.N.in-not` (implied) | `pokeys-async.0.digin.N.in-not` via `hal_export_digin` | **canonical and compatible** | Same prefix decision |
| `digin.<J>.invert` (bit, HAL_PARAM_RW) | not canonical (canonical digin has no invert) | `pokeys.0.digin.N.invert` (via Python layer) | `pokeys-async.0.digin.N.invert` via `hal_param_bit_newf` | **PoKeys-specific extension** | Decide whether to keep or remove |
| `digout.<J>.out` (bit, HAL_IN) | canonical digout.out | `pokeys.0.digout.N.out` | `pokeys-async.0.digout.N.out` via `hal_export_digout` | **canonical and compatible** | Prefix decision |
| `digout.<J>.invert` (bit, HAL_PARAM_RW) | canonical digout.invert parameter | `pokeys.0.digout.N.invert` | `pokeys-async.0.digout.N.invert` via `hal_export_digout` | **canonical and compatible** | Prefix decision |
| `adcin.<J>.value` (float, HAL_OUT) | canonical adcin.value | `pokeys.0.adcin.J.value` (issue #35) | **ABSENT** (Canon.value computed internally but never exported) | **canonical but missing** | Must decide: export canonical value via `hal_export_adcin` or continue non-canonical export |
| `adcin.<J>.value-raw` (float, HAL_OUT) | not canonical | issue #35 specifies `adcin.J.value-raw` | `adcin.J.in.raw` (float, HAL_OUT) — **different name** | **renamed** (name mismatch with issue #35 spec) | Align name with issue #35 or use canonical `value` only |
| `adcin.<J>.in.hw` (u32, HAL_OUT) | not canonical | not in legacy spec | implemented in pokeysHal as raw ADC count | **PoKeys-specific extension** (non-canonical name and type) | Decide: keep as supplementary, rename, or remove |
| `adcin.<J>.scale` (float, HAL_PARAM_RW) | canonical adcin.scale | specified in issue #35 | **ABSENT** | **canonical but missing** | Must be added with `hal_export_adcin` |
| `adcin.<J>.offset` (float, HAL_PARAM_RW) | canonical adcin.offset | specified in issue #35 | **ABSENT** | **canonical but missing** | Must be added with `hal_export_adcin` |
| `adcin.<J>.bit-weight` (float, HAL_PARAM_RW) | canonical adcin.bit-weight | not specified in issue #35 | **ABSENT** | **canonical but missing** | Decide: required by CDI spec, include with `hal_export_adcin` |
| `adcin.<J>.hw-offset` (float, HAL_PARAM_RW) | canonical adcin.hw-offset | not specified in issue #35 | **ABSENT** | **canonical but missing** | Same as bit-weight |
| `adcin.<J>.ReferenceVoltage` (float, HAL_PARAM_RO) | not canonical | not in legacy | implemented | **PoKeys-specific extension** | Decide: keep or integrate into hw-offset semantics |
| `adcout.<J>.value` (float, HAL_IN) | canonical adcout.value | `pokeys.0.adcout.0.value` (legacy HAL file) | **ABSENT** (hal_adcout_t allocated, hal_export_adcout never called) | **canonical but missing** | Critical: referenced in integration files |
| `adcout.<J>.enable` (bit, HAL_IN) | canonical adcout.enable | `pokeys.0.adcout.0.enable` (legacy HAL file) | **ABSENT** | **canonical but missing** | Critical: referenced in integration files |
| `adcout.<J>.scale` (float, HAL_PARAM_RW) | canonical adcout.scale | not confirmed | **ABSENT** | **canonical but missing** | Must add with `hal_export_adcout` |
| `adcout.<J>.offset` (float, HAL_PARAM_RW) | canonical adcout.offset | not confirmed | **ABSENT** | **canonical but missing** | Must add |
| `adcout.<J>.high-limit` (float, HAL_PARAM_RW) | canonical adcout.high-limit | not confirmed | **ABSENT** | **canonical but missing** | Must add |
| `adcout.<J>.low-limit` (float, HAL_PARAM_RW) | canonical adcout.low-limit | not confirmed | **ABSENT** | **canonical but missing** | Must add |
| `adcout.<J>.bit-weight` (float, HAL_PARAM_RW) | canonical adcout.bit-weight | not confirmed | **ABSENT** | **canonical but missing** | Must add |
| `adcout.<J>.hw-offset` (float, HAL_PARAM_RW) | canonical adcout.hw-offset | not confirmed | **ABSENT** | **canonical but missing** | Must add |
| `adcout.<J>.PWMduty` (u32, HAL_OUT) | not canonical | not in legacy CDI | implemented | **PoKeys-specific extension** (non-canonical PWM raw output) | Decide: keep supplementary or replace with canonical |
| `adcout.<J>.max_voltage` (float, HAL_PARAM_RW) | not canonical | not in legacy CDI | implemented | **PoKeys-specific extension** | Decide: keep or map to canonical hw-offset/scale |
| `adcout.pwm.period` (u32, HAL_PARAM_RW) | not canonical | not in legacy CDI | implemented | **PoKeys-specific extension** | Decide: keep |
| `encoder.<I>.count` (s32, HAL_OUT) | canonical encoder.count | `pokeys.0.encoder.0.count` (legacy HAL) | `pokeys-async.0.encoder.I.count` | **canonical and compatible** | Prefix decision |
| `encoder.<I>.position` (float, HAL_OUT) | canonical encoder.position | not confirmed | `pokeys-async.0.encoder.I.position` | **canonical and compatible** | Prefix decision |
| `encoder.<I>.velocity` (float, HAL_OUT) | canonical encoder.velocity | not confirmed | `pokeys-async.0.encoder.I.velocity` | **canonical and compatible** | Prefix decision |
| `encoder.<I>.reset` (bit, HAL_IN) | canonical encoder.reset | not confirmed | `pokeys-async.0.encoder.I.reset` | **canonical and compatible** | Prefix decision |
| `encoder.<I>.index-enable` (bit, HAL_IN) | canonical encoder.index-enable | not confirmed | `pokeys-async.0.encoder.I.index-enable` | **canonical and compatible** | Prefix decision |
| `encoder.<I>.scale` (float, HAL_PARAM_RW) | canonical encoder.scale | not confirmed | `pokeys-async.0.encoder.I.scale` | **canonical and compatible** | Prefix decision |
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
| canonical and compatible | 9 |
| canonical but missing | 12 |
| PoKeys-specific extension (present) | 18 |
| legacy extension (present) | 10 |
| renamed | 2 |
| legacy extension — missing | 10 |
| PoKeys-specific — missing | 4 |
| **Total HAL objects catalogued** | **~65** |
