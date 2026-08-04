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
  Note: adcin.value direction mismatch (HAL_IN instead of HAL_OUT) means the
  component is an unsupported writer — see CONFLICT-009 for full impact analysis.
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
Status: code path verified by inspection (C4 trace). Evidence: canonical export: implemented; active function: PK_PWMUpdateAsync (PoKeysLibIOAsync.c:703); active build object: PoKeysLibIOAsync.o (Submakefile.rt F-009); conversion: implemented and reachable; scheduler: 100 ms; HIL hardware behaviour: not verified; supplementary PWM-object policy: unresolved. HIL execution pending.
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
  - Digital input device (hardware → HAL): component writes hardware state to HAL.
    digin.in must be HAL_OUT (component is the authoritative writer).
    digin.in-not must be HAL_OUT.
  - Digital output device (HAL → hardware): LinuxCNC writes command to component.
    digout.out must be HAL_IN (component reads the commanded value).
  - Analog input (hardware → HAL): component writes scaled value to HAL.
    adcin.value must be HAL_OUT.
  - Analog output (HAL → hardware): HAL_IN is correct.
    adcout.value HAL_IN, adcout.enable HAL_IN — both correct.
Source B: Embedded hal-canon tree at 995d7057 (hal-canon/hal_digital.c,
  hal-canon/hal_analog.c), independently verified:
  - hal_export_digin: digin.in created with HAL_IN   ← WRONG (should be HAL_OUT)
                      digin.in-not created with HAL_OUT ← CORRECT
  - hal_export_digout: digout.out created with HAL_OUT ← WRONG (should be HAL_IN)
  - hal_export_adcin: adcin.value created with HAL_IN  ← WRONG (should be HAL_OUT)
  - hal_export_adcout: adcout.value HAL_IN, adcout.enable HAL_IN ← BOTH CORRECT
Observed implementation:
  pokeysHal embeds tree at 995d7057 and inherits three direction bugs.
  Current upstream zarfld/linuxcnc-hal-canon main has SIX bugs due to regression
  commit 45adb952 — see CONFLICT-011.
LinuxCNC hal_link semantics (required for correct impact analysis):
  - A signal can have at most ONE HAL_OUT writer; hal_link rejects a second HAL_OUT.
  - Multiple HAL_IN pins can be linked to one signal without error.
  - HAL_IN + HAL_IN does not itself cause a wiring-time direction error.
Why the sources conflict:
  The hal-canon implementation declares wrong directions for three pins.
  Per-bug impact using hal_link semantics:
  a) digin.in as HAL_IN (should be HAL_OUT):
     Incorrect ownership metadata. The component code (PK_DigitalIOGetParse)
     writes the hardware state to pin memory in software, but the pin is
     declared as a reader (HAL_IN), meaning the HAL signal has no declared
     HAL_OUT writer. This is unsupported writer behaviour: the component
     writes a value to a pin it is not the declared driver of. If any external
     HAL_OUT source is connected to the same signal, it continuously overwrites
     the hardware-read value. HAL wiring succeeds without direction error
     (HAL_IN pins do not reject connections), but signal ownership is wrong.
  b) digout.out as HAL_OUT (should be HAL_IN):
     The component is declared as the driver of the signal. When LinuxCNC’s
     motion controller or any other component tries to connect an HAL_OUT
     command source to the same signal, hal_link rejects the connection
     because the signal already has a source. This directly blocks the normal
     use case of commanding a digital output from LinuxCNC.
  c) adcin.value as HAL_IN (should be HAL_OUT):
     Same unsupported writer situation as digin.in. Component code writes the
     scaled analog reading but is not the declared HAL driver. Signal has no
     HAL_OUT source. If an external HAL_OUT source is connected, it overwrites
     the hardware reading. No wiring-time direction error unless a second
     HAL_OUT is added.
Safety or compatibility impact:
  digout.out (HAL_OUT instead of HAL_IN): HIGH — blocks any HAL_OUT command
    source from being connected; hal_link rejects the second writer.
  digin.in and adcin.value (HAL_IN instead of HAL_OUT): MEDIUM — incorrect
    ownership metadata; no declared signal source; hardware state is written
    to the pin in code but may be overwritten by an external HAL_OUT source.
    HAL wiring succeeds without direction error.
Authority assessment:
  Authority A (LinuxCNC HAL API and canonical device specification) governs.
  hal-canon (Authority B) contains implementation bugs contradicting Authority A.
  Phase 0 scope prohibits modifying hal-canon.
Required decision:
  1. Correct hal-canon/hal_digital.c: change digin.in to HAL_OUT and
     digout.out to HAL_IN.
  2. Correct hal-canon/hal_analog.c: change adcin.value to HAL_OUT.
  3. digin.in-not (HAL_OUT) and adcout.value/enable (HAL_IN) are correct;
     do not change them.
  Priority: digout.out fix is HIGH (blocks digital output command wiring).
  digin.in and adcin.value fixes are MEDIUM (should be done in same pass).
Status: unresolved
```
---

```
Conflict ID: CONFLICT-010
Subject: Issue #79 extends "Canonical Device Interface" terminology beyond official LinuxCNC CDI scope
Source A: Official LinuxCNC CDI (Authority A, source A-002):
  docs/src/hal/canonical-devices.adoc at commit 71bf88009d64fa15edbebf9250b65ee4454f9a05
  defines exactly four canonical device interfaces: digin, digout, adcin, adcout.
  No other device types are defined as canonical in the official specification.
Source B: LinuxCnc_PokeysLibComp issue #79 (Authority E):
  "Ensure Conformity with LinuxCNC Guidelines and Canonical Device Interface"
  Body extends the term to include:
  - motion/PEv2 pins (pos-cmd, vel-cmd, amp-enable-out)
  - counters and PWM
  - PoNET
  - encoder (not an official CDI type)
Observed implementation:
  Phase 0 documentation inherited this extended usage in several places,
  classifying encoder and PEv2 as "canonical" without primary-source support.
  These have been corrected in round 4.
Why the sources conflict:
  Using "canonical" for non-CDI device types conflates the official standard with
  PoKeys-specific or established-by-convention interfaces. This makes compatibility
  claims imprecise: satisfying "canonical" could mean four things or many more
  depending on which source is referenced.
Safety or compatibility impact:
  MEDIUM. Incorrect classification leads to test-design errors: applying CDI
  compliance tests to non-CDI interfaces, or missing CDI obligations. Integrators
  reading requirements may be misled about what standard must be followed.
Authority assessment:
  Authority A (A-002) is normative. Only digin, digout, adcin, adcout are CDI.
  PEv2 and encoder follow established LinuxCNC or project conventions, not the CDI spec.
Required decision:
  1. For each non-CDI interface: identify the actual governing standard or convention.
  2. Rename "canonical" classification in requirements and tests to the accurate
     authority (e.g., "LinuxCNC motion-interface convention", "hal-canon convention",
     "PoKeys-specific extension", "project compatibility contract").
  See DEC-CONFLICT010 in open-decisions.md.
Status: unresolved
```

---

```
Conflict ID: CONFLICT-011
Subject: Upstream zarfld/linuxcnc-hal-canon commit 45adb952 introduces three new direction regressions
  making six canonical pin directions wrong in current upstream main

Embedded tree state (995d7057dd5403865d423aab64ba30d81ccd5ee0):
  THREE wrong directions:
  - digin.in   : HAL_IN  (should be HAL_OUT — component writes hardware state)
  - digout.out : HAL_OUT (should be HAL_IN  — component reads commanded value)
  - adcin.value: HAL_IN  (should be HAL_OUT — component writes scaled hardware reading)
  THREE correct directions:
  - digin.in-not : HAL_OUT (correct — component writes inverted hardware state)
  - adcout.value : HAL_IN  (correct — external command producer writes desired output)
  - adcout.enable: HAL_IN  (correct — external command producer writes enable signal)

Source A: LinuxCNC HAL data-flow semantics and hal_link rules (Authority A):
  - A signal can have at most one HAL_OUT writer; hal_link rejects a second HAL_OUT.
  - Multiple HAL_IN pins can be linked to one signal without error.
  - A component that drives hardware state must export HAL_OUT (it is the writer).
  - A component that receives a commanded value must export HAL_IN (it is the reader).
  - For adcout.value: the external command producer (motion controller, spindle control)
    is the HAL_OUT writer. adcout.value must therefore be HAL_IN so the component
    can read it. If adcout.value were HAL_OUT, any external command-source (also HAL_OUT)
    would be rejected by hal_link as a second writer.

Source B: Observed changes in upstream commit 45adb952627ab07cac9e5467e49c25e35ea6cd4a
  (zarfld/linuxcnc-hal-canon, 2025-06-09, 'Fix pin direction for adcout and digin
  parameters in hal_analog.c and hal_digital.c'). Changes verified by diff:
  - adcout.value : HAL_IN  → HAL_OUT   REGRESSION (was correct; now wrong)
  - adcout.enable: HAL_IN  → HAL_OUT   REGRESSION (was correct; now wrong)
  - digin.in-not : HAL_OUT → HAL_IN    REGRESSION (was correct; now wrong)
  Three original bugs (digin.in, digout.out, adcin.value) remain UNCHANGED.

Current upstream state (zarfld/linuxcnc-hal-canon HEAD after 45adb952):
  SIX wrong directions — three original bugs plus three new regressions:
  - digin.in   : HAL_IN  WRONG (original bug)
  - digin.in-not: HAL_IN  WRONG (regression — was HAL_OUT at 995d705)
  - digout.out : HAL_OUT WRONG (original bug)
  - adcin.value: HAL_IN  WRONG (original bug)
  - adcout.value: HAL_OUT WRONG (regression — was HAL_IN at 995d705; hal_link will reject
    any external HAL_OUT command source as second writer)
  - adcout.enable: HAL_OUT WRONG (regression — was HAL_IN at 995d705; same second-writer
    rejection applies)

Observed implementation (pokeysHal):
  The embedded tree (995d705) has only three wrong directions.
  Importing 45adb952 would increase the bug count from three to six.

Why the sources conflict:
  Commit 45adb952 is titled as a direction fix but introduces regressions on all three
  changed pins. It does not repair any of the three original bugs. The result is worse
  than the baseline it claims to improve.

Safety or compatibility impact:
  adcout.value/enable regression (HIGH): if 45adb952 is ever imported, any attempt to
    connect an external HAL_OUT command producer to adcout.J.value or adcout.J.enable
    would fail with "signal already has a source" from hal_link. Analog output would
    become non-functional from a wiring perspective.
  digin.in-not regression (MEDIUM): pin changes from a declared writer (HAL_OUT) to a
    reader (HAL_IN), removing the valid signal-source declaration for the inverted state.

Required action:
  Do NOT cherry-pick or import commit 45adb952.
  A correct fix must change ONLY:
  - digin.in from HAL_IN to HAL_OUT
  - digout.out from HAL_OUT to HAL_IN
  - adcin.value from HAL_IN to HAL_OUT
  adcout.value, adcout.enable, and digin.in-not must remain at their current (correct)
  values in the embedded tree.
Status: unresolved
```

---

```
Conflict ID: CONFLICT-013
Subject: PEv2 AxesCommand semantic mismatch between legacy userspace component and pokeys_homecomp

Source A: pokeys_uspace/PoKeysComp.h @ 0c058e6c (pokeys_uspace PK_PEAxisCommand enum, L931-937):
  PK_PEAxisCommand_axIDLE              = 0  // Axis in IDLE
  PK_PEAxisCommand_axHOMINGSTART       = 1  // Start Homing procedure
  PK_PEAxisCommand_axARMENCODER        = 2  // reset position to zeros
  PK_PEAxisCommand_axHOMINGWaitFinalMove = 3  // move to homeposition
  PK_PEAxisCommand_axHOMINGFinalMove   = 4  // move to homeposition
  PK_PEAxisCommand_axHOMINGCancel      = 5  // Cancel Homing procedure
  PK_PEAxisCommand_axHOMINGFinalize    = 6  // Finish Homing procedure
  This enum has 7 values (0-6). Values 2-6 are ARMENCODER, Wait, Final, Cancel, Finalize.

Source B: pokeys_rt/pokeys_homecomp.comp @ 0c058e6c (pokeys_home_command_t enum, inspected E-010):
  PK_PEAxisCommand_axIDLE       = 0  // IDLE
  PK_PEAxisCommand_axHOMINGSTART = 1  // Start Homing
  PK_PEAxisCommand_axHOMINGCANCEL = 2  // Cancel Homing procedure
  PK_PEAxisCommand_axHOMINGFINALIZE = 3  // Finish Homing procedure
  This enum has 4 values (0-3). Values 2 and 3 are CANCEL and FINALIZE.

Observed incompatibility:
  Value 2: legacy = ARMENCODER; homecomp = HOMINGCANCEL  — INCOMPATIBLE SEMANTICS
  Value 3: legacy = HOMINGWaitFinalMove; homecomp = HOMINGFINALIZE — INCOMPATIBLE SEMANTICS
  The AxesCommand HAL pin carries the value as a raw u32. If the legacy userspace component
  reads a command written by pokeys_homecomp, or vice versa, the action taken will be wrong.

Why the sources conflict:
  The two components were developed with different homing state machines. The homecomp
  is a skeleton/minimal implementation; the userspace component has a more elaborate
  multi-step sequence requiring ARMENCODER and separate wait/finalize states.
  No shared header enforces a single canonical enum.

Safety or compatibility impact:
  HIGH. A value-2 command from pokeys_homecomp (HOMINGCANCEL) would be interpreted
  as ARMENCODER by the legacy userspace PEv2 component, potentially causing unintended
  encoder reset and position loss during a homing procedure.

Authority assessment:
  Neither source has normative authority over the other. The PoKeysLib hardware enum
  (ePK_PEAxisState in PoKeysLib.h) defines axis states but not HAL-level commands.
  The HAL command contract must be agreed between the two software components through
  a shared header or documented convention.

Required decision:
  1. Define a single authoritative PEv2 AxesCommand enum shared by all components.
  2. Determine which component's homing state machine is the reference implementation.
  3. Align homecomp and userspace component on a common command vocabulary.
  4. Integration link IK-003 (AxesCommand) must not be classified compatible until resolved.
Status: unresolved
```

---

```
Conflict ID: CONFLICT-014
Subject: Current pokeysHal AxesCommand pin has no traced normal-cycle consumer;
  test mode writes a HAL_IN pin with a bitmask

Source A: experimental/pokeys_async.c FUNCTION(_) active RT function (F-008):
  The active FUNCTION(_) calls:
    PK_ReceiveAndDispatch() and PK_TimeoutAndRetryCheck()  — Phase 1
    async_dispatcher()  — Phase 2 (scheduler tasks)
    PK_ReceiveAndDispatch() again  — Phase 3
    update_ponet_hal_pins()
  None of the following helper functions are called from FUNCTION(_):
    rt_read_command_pins()        — DEFINED-BUT-UNREACHED from active path
    rt_update_motion_commands()   — DEFINED-BUT-UNREACHED from active path
    rt_read_device_cache()        — DEFINED-BUT-UNREACHED from active path
    rt_handle_homing_commands()   — DEFINED-BUT-UNREACHED from active path
    rt_update_external_outputs()  — DEFINED-BUT-UNREACHED from active path
  rt_handle_homing_commands() reads HomingStatus (not AxesCommand) to detect
  homing start requests.

Source B: experimental/pokeys_async.c test-mode code (~L891-892):
  if (inst->dev->PEv2.pin_AxesCommand && inst->dev->PEv2.pin_AxesCommand[0]) {
      *(inst->dev->PEv2.pin_AxesCommand[0]) |= 0x01;  // Set enable bit
  }
  This writes to a HAL_IN pin (incorrect: component should not drive HAL_IN)
  and treats the value as a bitmask, not the standard command enum.

Observed state:
  The current pokeysHal example component exports AxesCommand as HAL_IN,
  meaning it is declared as a consumer (reader) of external AxesCommand values.
  No code path from FUNCTION(_) was found that reads AxesCommand and sends
  the value to the PoKeys hardware. The test-mode code writes to the pin
  incorrectly and uses bitmask semantics.

Why this matters:
  Any integration expecting the current component to forward AxesCommand
  from a homecomp producer to the hardware will not work as expected —
  the RT path does not implement this forwarding.

Safety or compatibility impact:
  HIGH. Homing and motion commands relying on AxesCommand propagation
  to hardware may silently fail. Integration link IK-003 cannot be classified
  as implemented until a reachable call path is identified.

Required action:
  1. Determine whether AxesCommand forwarding is intended or out of scope.
  2. If intended: implement a reachable call path from FUNCTION(_).
  3. If out of scope: remove the AxesCommand pin or document explicitly.
  4. Remove the test-mode write to a HAL_IN pin.
Status: unresolved
```

---

## External Observations Excluded from the pokeysHal Baseline

The following observations concern a separate component and are not pokeysHal conflicts.

**pokeys_homecomp implementation observations** belong to `zarfld/LinuxCnc_PokeysLibComp`
and are not part of the pokeysHal library baseline. Specifically:

- The unreachable initialization block in `homing_init()` (lines 366–397) and its
  effect on `volatile_home` is an internal defect of `pokeys_homecomp` (E-010).
- `set_unhomed()` behavior and `VOLATILE_HOME` semantics are LinuxCNC homemod policy
  questions, not pokeysHal library interface requirements.
- These observations may be relevant to the `zarfld/LinuxCnc_PokeysLibComp` project
  but must not block the pokeysHal Phase 0 baseline or Phase 1 planning.
