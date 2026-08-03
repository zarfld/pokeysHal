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
Subject: Analog input — canonical pins ARE exported but name specification mismatch
  and supplementary non-canonical pins also present
Source A: Issue #35 (pokeysHal, CLOSED):
  Required: adcin.J.value-raw (float, raw hardware reading),
            adcin.J.value (float, scaled), adcin.J.scale, adcin.J.offset.
Source B: Current implementation (PoKeysLibIOAsync.c:85):
  hal_export_adcin(&device->AnalogInput[j].Canon, prefix, j, comp_id) IS called.
  This exports via hal-canon: adcin.J.value (HAL_IN per hal-canon — direction
  mismatch, see CONFLICT-009), adcin.J.scale, adcin.J.offset,
  adcin.J.bit-weight, adcin.J.hw-offset.
  ADDITIONALLY exported (non-canonical supplementary):
    adcin.J.in.hw (u32, HAL_OUT) — raw ADC counts
    adcin.J.in.raw (float, HAL_OUT) — voltage-scaled
    adcin.J.ReferenceVoltage (float, HAL_PARAM_RO)
Observed implementation:
  Canonical pins (via hal_export_adcin) and non-canonical supplementary pins
  coexist. The name 'value-raw' from issue #35 is not present; instead
  'in.raw' is used. The 'in.hw' pin is additional.
Why the sources conflict:
  Issue #35 specifies 'value-raw'; implementation exports 'in.raw' instead.
  Issue #35 is CLOSED despite the name mismatch.
Safety or compatibility impact:
  LOW for canonical pins (present). MEDIUM for the 'value-raw' name: any HAL
  file referencing adcin.J.value-raw will fail to connect (pin is 'in.raw').
  Critical: adcin.value direction mismatch (HAL_IN instead of HAL_OUT) prevents
  correct data flow until hal-canon is corrected (see CONFLICT-009).
Authority assessment:
  Canonical pins are exported. The name conflict between 'value-raw' (issue #35)
  and 'in.raw' (implementation) is unresolved. Direction mismatch is a hal-canon
  defect (Authority B), not a pokeysHal implementation defect.
Required decision:
  1. Align 'in.raw' name with issue #35 'value-raw', or update issue #35.
  2. Decide whether 'in.hw' and 'ReferenceVoltage' are kept as supplementary.
  3. The hal-canon direction mismatch for adcin.value must be resolved (see
     CONFLICT-009 and DEC-HALCANON-001 in open-decisions.md).
Status: unresolved
```

---

```
Conflict ID: CONFLICT-004
Subject: Analog output — canonical pins ARE exported; supplementary non-canonical
  pins also present; requirements still reference missing PWM mapping
Source A: Issues #37 and #39 (pokeysHal, both OPEN) and legacy integration file:
  Required: adcout.J.value (float, HAL_IN), adcout.J.enable (bit, HAL_IN), and
  full parameter set. Legacy: "net spindle-speed-DAC => pokeys.0.adcout.0.value"
  and "net spindle-enable => pokeys.0.adcout.0.enable".
Source B: Current implementation (PoKeysLibIOAsync.c:110):
  hal_export_adcout(&device->PWM.PWManalogOutputs[j], prefix, j, comp_id) IS called.
  This exports via hal-canon: adcout.J.value (HAL_IN — correct), adcout.J.enable
  (HAL_IN — correct), adcout.J.offset, adcout.J.scale, adcout.J.high-limit,
  adcout.J.low-limit, adcout.J.bit-weight, adcout.J.hw-offset.
  ADDITIONALLY exported (non-canonical supplementary):
    adcout.J.max_voltage (float, HAL_PARAM_RW)
    adcout.J.PWMduty (u32, HAL_OUT)
    adcout.pwm.period (u32, HAL_PARAM_RW)
Observed implementation:
  Canonical adcout pins (via hal_export_adcout) are present. Supplementary PWM
  raw pins also exported. Issues #37 and #39 remain OPEN, suggesting the
  scaling from canonical value to PWM duty cycle is not yet complete or tested.
Why the sources conflict:
  Issues #37 and #39 are OPEN despite canonical pins being exported. The
  conflict may be that the functional link — converting adcout.J.value to a
  PWM duty cycle via the analog output hardware — is unimplemented or unverified.
Safety or compatibility impact:
  MEDIUM. Canonical pin names present for HAL wiring. PWM conversion logic
  may not be implemented, so writing adcout.J.value may not affect hardware.
Authority assessment:
  Canonical object export is implemented. The open issues likely refer to the
  unresolved question of whether the conversion path (value → PWM duty) works.
Required decision:
  1. Determine whether the PWManalogOutputs[j] conversion path from adcout.value
     to PWM duty cycle is functionally complete.
  2. Decide whether adcout.J.PWMduty and adcout.J.max_voltage remain supplementary.
  3. Close or update issues #37 and #39 with evidence of actual conversion behavior.
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

---

```
Conflict ID: CONFLICT-009
Subject: hal-canon export helpers use incorrect HAL directions
Source A: LinuxCNC HAL data-flow semantics (Authority A):
  - Digital input device (hardware → HAL): component writes to HAL.
    digin.in must be HAL_OUT (component is the driver/writer).
    digin.in-not must be HAL_OUT.
  - Digital output device (HAL → hardware): LinuxCNC writes to component.
    digout.out must be HAL_IN (component is the reader).
  - Analog input (hardware → HAL): component writes to HAL.
    adcin.value must be HAL_OUT.
  - Analog output (HAL → hardware): LinuxCNC writes to component.
    adcout.value must be HAL_IN (correct).
    adcout.enable must be HAL_IN (correct).
Source B: Observed hal-canon implementation (hal-canon/hal_digital.c,
  hal-canon/hal_analog.c):
  - hal_export_digin: digin.in created with HAL_IN   ← WRONG (should be HAL_OUT)
                      digin.in-not created with HAL_OUT ← correct
  - hal_export_digout: digout.out created with HAL_OUT ← WRONG (should be HAL_IN)
  - hal_export_adcin: adcin.value created with HAL_IN  ← WRONG (should be HAL_OUT)
  - hal_export_adcout: adcout.value HAL_IN, adcout.enable HAL_IN ← both correct
Observed implementation:
  pokeysHal calls hal_export_digin, hal_export_digout, hal_export_adcin, and
  hal_export_adcout. It therefore inherits all three direction bugs. The pins
  digin.J.in, digout.J.out, and adcin.J.value are created with the wrong
  direction in every running instance of the component.
Why the sources conflict:
  The LinuxCNC HAL direction convention is unambiguous: HAL_OUT means the
  component writes the value; HAL_IN means the component reads it. The
  hal-canon implementation reverses the convention for digin.in, digout.out,
  and adcin.value.
Safety or compatibility impact:
  HIGH. A pin with the wrong direction will fail to connect with compatible
  signals in HAL. For example, attempting to net digin.J.in to a motion-controller
  input will fail at HAL wiring time because both end-points would be HAL_IN.
  LinuxCNC halcmd reports "pin direction mismatch" and refuses the connection.
  This renders the digital input and analog input interfaces non-functional.
Authority assessment:
  Authority A (LinuxCNC HAL API and canonical device specification) governs.
  hal-canon (Authority B) contains implementation bugs contradicting Authority A.
  Phase 0 scope prohibits modifying hal-canon. The conflict must be recorded
  and a decision made before Phase 1.
Required decision:
  1. Correct hal-canon/hal_digital.c: change digin.in to HAL_OUT and
     digout.out to HAL_IN.
  2. Correct hal-canon/hal_analog.c: change adcin.value to HAL_OUT.
  3. Verify that digin.in-not (HAL_OUT) and adcout.value/enable (HAL_IN)
     remain unchanged — these are correct.
  Note: This requires modifying hal-canon, which is out of scope for Phase 0.
  It must be the first technical action in Phase 1 before any compatibility
  testing can produce meaningful results.
Status: unresolved
```
