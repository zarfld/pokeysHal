# Conflict Register — Phase 0 HAL-Interface Baseline

Evidence from pokeysHal@cd1f0dc8a0f64f92dc6bdce21bddcb36d33a14cd.

---

```
Conflict ID: CONFLICT-001
Subject: Conditional PEv2 per-axis pin creation — nrOfAxes == 0 behaviour
Source A: REQ-F-PEV2-003 (#118, pokeysHal) — Scenario 3:
  "Given device->PEv2.info.nrOfAxes equals 0
   When export_pev2_pins() completes
   Then no per-axis pins are created
   And a warning message is emitted."
Source B: ADR-PEV2-002 (#128, pokeysHal, status: Accepted):
  "If nrOfAxes is 0 at pin-export time (device not yet responding), create pins
   for all 8 axes with a warning, to avoid blocking startup."
Observed implementation:
  PoKeysLibPulseEngine_v2Async.c:380 — unconditional loop `for (int i = 0; i < 8; i++)`.
  No check for nrOfAxes. No check for nrOfAxes == 0. Always creates 8 axes.
  This matches the ADR fallback behaviour, not the primary requirement.
Why the sources conflict:
  REQ (#118) gives no fallback for nrOfAxes==0: zero pins, emit warning.
  ADR (#128) introduces a fallback: create all 8 to avoid blocking startup.
  The accepted ADR overrides the requirement for the zero case only; however,
  the non-zero case is also unchecked — the implementation does not use nrOfAxes
  as an upper bound even when nrOfAxes > 0.
Safety or compatibility impact:
  On a 2-axis device, HAL will expose pins for 6 non-existent axes (pokeys-async.0.PEv2.2-7.*).
  This may confuse integrators and LinuxCNC motion controller configuration.
  If nrOfAxes is read after pin export with a value < 8, the extra pins are
  permanently present but unused.
Authority assessment:
  ADR-PEV2-002 (#128) is Authority D (design decision).
  REQ-F-PEV2-003 (#118) is Authority C (functional requirement).
  Authority C normally takes precedence over D, but the ADR was recorded as
  "Accepted" against the requirement. The ADR partially contradicts scenario 3
  of the requirement. Neither authority is normative external (class A).
Required decision:
  1. Is the "zero axes = no pins" rule (REQ #118 scenario 3) authoritative, or
     is the "zero axes = 8 axes fallback" rule (ADR #128) authoritative?
  2. For non-zero nrOfAxes < 8: should the loop upper bound be nrOfAxes or 8?
  3. Should scenario 3 be amended in REQ #118 to match ADR #128?
Status: unresolved
```

---

```
Conflict ID: CONFLICT-002
Subject: PEv2 per-axis digout.AxisEnable.out — specified in integration files but absent from implementation
Source A: LinuxCnc_PokeysLibComp DM542_XXYZ_mill/Pokeys_DM542_XXYZ_mill.hal:
  "net x-enable => pokeys.0.PEv2.0.digout.AxisEnable.out"
  This pin is required for axis enable control in machine configurations.
Source B: Current pokeysHal implementation (PoKeysLibPulseEngine_v2Async.c):
  No hal_pin_bit_newf call for digout.AxisEnable.out found in any file.
  PoKeysLibHal.h pin pointer struct (sPoKeysPEv2HAL) does not contain
  pin_digout_AxisEnable_out[8] field.
Observed implementation:
  The pin does not exist in pokeysHal. Integration files reference it directly.
Why the sources conflict:
  Legacy integration files assume this pin is available. It is absent.
  Without it, the machine configuration cannot connect axis enable signals.
Safety or compatibility impact:
  HIGH. Without digout.AxisEnable.out, axis enable control via HAL is not
  possible. This directly affects machine safety (drives remain enabled
  even if LinuxCNC commands disable them).
Authority assessment:
  Integration file (Authority G) demonstrates expected usage.
  Issue #33 (Authority C) specifies PEv2 per-axis interface; AxisEnabledMask
  is listed in issue #33 but per-axis AxisEnable.out is not explicitly listed.
  Absence in the implementation is a gap, not a deliberate decision.
Required decision:
  1. Confirm whether digout.AxisEnable.out is required for compatibility.
  2. Identify which PEv2 register bit controls per-axis output enable.
  3. Add to PoKeysLibHal.h and export_pev2_pins.
Status: unresolved
```

---

```
Conflict ID: CONFLICT-003
Subject: Analog input HAL interface — non-canonical implementation vs. requirements
Source A: Issue #35 (pokeysHal, CLOSED) and LinuxCNC CDI (Authority A):
  Required pins: adcin.J.value-raw (float), adcin.J.value (float).
  Required parameters: adcin.J.scale (float, RW), adcin.J.offset (float, RW).
  (CDI also requires bit-weight and hw-offset per hal-canon Authority B.)
Source B: Current implementation (PoKeysLibIOAsync.c):
  Exports: adcin.J.in.hw (u32, HAL_OUT, raw ADC counts),
           adcin.J.in.raw (float, HAL_OUT, voltage-scaled),
           adcin.J.ReferenceVoltage (float, HAL_PARAM_RO).
  Does NOT export: value, value-raw (per spec), scale, offset, bit-weight, hw-offset.
  Note: sPoKeysAnalogData.Canon (hal_adcin_t) exists and Canon.value is computed
  internally, but hal_export_adcin is never called.
Observed implementation:
  Non-canonical pin names; canonical interface absent despite hal_adcin_t being
  allocated and partially populated.
Why the sources conflict:
  Issue #35 is CLOSED, implying it was resolved. But the implementation does not
  match the specification in #35 (different pin names and missing parameters).
  The issue may have been closed prematurely or the scope was narrowed.
Safety or compatibility impact:
  Integration files that expect adcin.J.value (e.g., spindle speed feedback) will
  fail to connect. scale and offset not being available prevents calibration.
Authority assessment:
  Authority C (#35, closed) and Authority A (CDI) require the canonical interface.
  Current implementation (Authority F) does not provide it.
Required decision:
  1. Reopen or supersede issue #35 to reflect what was actually implemented.
  2. Decide whether adcin.J.in.hw and adcin.J.in.raw are to be kept supplementary
     or replaced by canonical value/value-raw names.
  3. Decide whether hal_export_adcin should be called to export the Canon struct.
Status: unresolved
```

---

```
Conflict ID: CONFLICT-004
Subject: Analog output HAL interface — non-canonical implementation vs. requirements
Source A: Issues #37 and #39 (pokeysHal, both OPEN) and LinuxCNC CDI (Authority A):
  Required pins: adcout.J.value (float, HAL_IN), adcout.J.enable (bit, HAL_IN).
  Required parameters: scale, offset, high-limit, low-limit, bit-weight, hw-offset.
  Legacy integration file: "net spindle-speed-DAC scale.0.out => pokeys.0.adcout.0.value"
  and "net spindle-enable => pokeys.0.adcout.0.enable"
Source B: Current implementation (PoKeysLibIOAsync.c):
  Exports: adcout.J.PWMduty (u32, HAL_OUT), adcout.J.max_voltage (float, HAL_PARAM_RW),
           adcout.pwm.period (u32, HAL_PARAM_RW).
  hal_adcout_t is allocated (PoKeysLibCoreAsync.c:618) but hal_export_adcout is never called.
Observed implementation:
  Non-canonical PWM-based output. Legacy integration files reference canonical names
  that do not exist.
Why the sources conflict:
  Integration files (Authority G, E-001) already reference adcout.0.value and
  adcout.0.enable. Requirements #37 and #39 are OPEN. Implementation provides
  different and incompatible pins.
Safety or compatibility impact:
  HIGH. Machine configurations requiring spindle speed output via DAC/PWM will fail
  to connect. The `enable` pin absence means HAL cannot force the output to zero.
Authority assessment:
  Authority A (CDI) and Authority C (#37, #39) require canonical interface.
  Authority G (integration files) demonstrates expected usage.
  Current implementation is Authority F and does not satisfy any of the above.
Required decision:
  1. Decide whether hal_export_adcout should be called for each PWM channel.
  2. Decide whether adcout.J.PWMduty remains as a supplementary raw output or
     is removed in favour of the canonical interface.
  3. Determine the scaling formula from canonical value to PWM duty cycle.
Status: unresolved
```

---

```
Conflict ID: CONFLICT-005
Subject: HAL object name length limit
Source A: LinuxCNC HAL API (Authority A): HAL_NAME_LEN = 47 (confirmed on target system).
  All pin, parameter, and function names must be ≤ 47 characters.
Source B: Legacy LinuxCnc_PokeysLibComp (Authority E):
  'pokeys.0.encoder.UltraFastEncoder.Enable4xSampling' = 50 characters — VIOLATED.
  Reported in issues #310 and #326 (LinuxCnc_PokeysLibComp).
Observed implementation in current pokeysHal:
  Current names appear ≤46 chars for device index 0 with prefix 'pokeys-async.0'.
  Example borderline names (rendered with index 0):
  - 'pokeys-async.0.PEv2.digout.ExternalRelay-0.out' = 46 chars ✓
  - 'pokeys-async.0.PEv2.0.digin.LimitN.in-not' = 42 chars ✓
  With device index ≥10 (prefix 'pokeys-async.10'), names are 1 char longer.
Why the sources conflict:
  Legacy component violated the limit. Current implementation is borderline for
  multi-digit device indices. No explicit length check is performed before
  hal_pin_*_newf calls.
Safety or compatibility impact:
  MEDIUM. A name-length violation causes a hard error at component load time,
  preventing the component from starting. This is a deployment blocker, not a
  runtime safety issue.
Authority assessment:
  Authority A is normative. Names MUST be ≤47 chars.
Required decision:
  1. Audit all format strings with device indices > 9.
  2. Consider whether 'pokeys-async' (12 chars) or a shorter prefix is warranted.
  3. Add a compile-time or startup assertion that names will fit.
Status: unresolved
```

---

```
Conflict ID: CONFLICT-006
Subject: Component name — pokeys vs. pokeys-async
Source A: LinuxCnc_PokeysLibComp legacy integration files (Authority E, G):
  'loadusr -W pokeys' — component name is "pokeys".
  All pins referenced as pokeys.0.*, pokeys.0.PEv2.*, etc.
Source B: Current pokeysHal integration shell (Authority F):
  hal_init("pokeys_async") at experimental/pokeys_async.c:156.
  Prefix: "pokeys-async.N" per export() at line 165.
  All pins exported as pokeys-async.0.*, pokeys-async.0.PEv2.*, etc.
Observed implementation:
  Every machine configuration file using the legacy component must have all
  "pokeys.0." references changed to "pokeys-async.0." before it works with
  the current implementation.
Why the sources conflict:
  The legacy component and current implementation use incompatible component
  and pin name prefixes. A machine configuration cannot use both without editing.
Safety or compatibility impact:
  HIGH for migration. All existing machine configurations using LinuxCnc_PokeysLibComp
  must be manually updated, or aliases/compatibility wrappers must be provided.
Authority assessment:
  There is no normative requirement fixing the component name to "pokeys".
  "pokeys-async" reflects the asynchronous nature of the new implementation.
  However, changing the name is a breaking change for all existing users.
Required decision:
  1. Decide whether pokeysHal will maintain the "pokeys" component name for
     backward compatibility, or document "pokeys-async" as the canonical name.
  2. If renaming, provide a migration guide for integrators.
  3. Consider whether "pokeys-async" is the final name or a development placeholder.
Status: unresolved
```

---

```
Conflict ID: CONFLICT-007
Subject: Issue #33 closed but implementation incomplete
Source A: Issue #33 (pokeysHal, CLOSED):
  "ensure pokeys_py and pokeys_rt follow the definition below for halinterface of PEv2"
  Specifies many pins including PulseEngineEnabled, PulseGeneratorType, MiscInputStatus,
  LimitOverride, AxisEnabledMask, digout.AxisEnable.out, HomingStartMaskSetup,
  ProbeStartMaskSetup, ProbeSpeed, BacklashCompensationEnabled, per-axis stepgen.*,
  HomingSpeed, HomingReturnSpeed, HomingAlgorithm, HomeOffsets, HomeBackOffDistance,
  SoftLimitMaximum, SoftLimitMinimum.
Source B: Current implementation (PoKeysLibPulseEngine_v2Async.c, PoKeysLibHal.h):
  None of the above pins found in export_pev2_pins or in __comp_state.
  Many are defined as struct fields in PoKeysLibHal.h (sPoKeysPEv2HAL) but not
  exported as HAL pins.
Observed implementation:
  Issue #33 is marked CLOSED but the implementation does not match the specification.
  This may mean: the issue was closed as "implemented in the legacy component" not
  in pokeysHal; or it was closed prematurely; or the scope was reduced without
  documentation.
Why the sources conflict:
  Closed issue implies resolved requirement. Absent implementation contradicts that.
Safety or compatibility impact:
  HIGH. Machine configurations referencing stepgen.*, HomingSpeed, SoftLimit pins
  will fail to connect. Backlash compensation, homing configuration, and probing
  setup pins are all missing.
Authority assessment:
  Issue #33 (Authority C) was the original HAL interface requirement for PEv2.
  The open REQ-F-PEV2-* issues (#116-#123) appear to re-specify subsets of #33.
  It is unclear whether #33 was superseded by the REQ-F-PEV2-* issues or if its
  scope was narrowed by them.
Required decision:
  1. Re-examine the closure rationale for issue #33.
  2. Determine which items from #33 are required for pokeysHal compatibility.
  3. Create or link REQ issues for the missing PEv2 pins.
  4. Do not treat CLOSED status as proof of implementation.
Status: unresolved
```

---

```
Conflict ID: CONFLICT-008
Subject: hal-canon submodule registration vs. embedded tree
Source A: .gitmodules declares hal-canon as a submodule:
  [submodule "hal-canon"]
    path = hal-canon
    url = https://github.com/zarfld/linuxcnc-hal-canon.git
Source B: git ls-tree HEAD hal-canon returns:
  040000 tree deed4c10535530ce0383fb357ea8427896226c70  hal-canon
  (040000 = regular tree, not 160000 = commit/submodule pointer)
  git submodule status: no entry for hal-canon.
Observed implementation:
  Files exist in hal-canon/ directory and are used directly in compilation.
  The upstream commit SHA (from linuxcnc-hal-canon.git) corresponding to these
  files is not recorded anywhere in the repository.
Why the sources conflict:
  .gitmodules promises that hal-canon is tracked at a specific remote commit.
  The git object model shows it is a regular tree, not a submodule pointer.
  This means: (a) the submodule was never properly initialized after the files
  were committed; or (b) the files were manually copied without running
  git submodule add; or (c) the submodule configuration is inconsistent.
Safety or compatibility impact:
  LOW for runtime. Compilation works because files are present.
  MEDIUM for maintenance: git submodule update will not work as expected.
  The upstream version of hal-canon cannot be tracked without manual comparison.
Authority assessment:
  submodules.instructions.md (repo instructions) requires pinned immutable
  commits for submodules. This constraint cannot be met with the current setup.
Required decision:
  1. Decide: properly initialize hal-canon as a submodule (removing the tree),
     or declare it an embedded subtree and update .gitmodules accordingly.
  2. Record the upstream commit SHA if hal-canon is to remain as a subtree.
Status: unresolved
```
