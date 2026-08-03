# Open Decisions — Phase 0 HAL-Interface Baseline

These decisions must be made before compatibility tests or implementation work
can begin. Phase 0 does not make these decisions. They are listed here for the
Phase 1 planning review.

---

## 1. LinuxCNC Canonical Conformity

### DEC-CANON-001: What to do about adcin.value direction mismatch and supplementary pins?

**Context:** `hal_export_adcin` IS called (PoKeysLibIOAsync.c:85). Canonical pins
`adcin.J.value`, `scale`, `offset`, `bit-weight`, `hw-offset` ARE present. However:
- `adcin.J.value` is exported as `HAL_IN` (bug in hal-canon) instead of `HAL_OUT`
  — invalid writer ownership: component writes the value but declares itself as a reader.
  The HAL signal has no declared HAL_OUT source. (CONFLICT-009).
- Supplementary pins `adcin.J.in.hw`, `adcin.J.in.raw`, `adcin.J.ReferenceVoltage`
  coexist.
- Issue #35 specified 'value-raw' but implementation uses 'in.raw' (CONFLICT-003).

**Options:**
- A: Fix hal-canon direction bug (digin.in, digout.out, adcin.value) first;
     rename 'in.raw' to 'value-raw'; keep supplementary as PoKeys extension.
- B: Keep current, document direction bug as known issue.

**Impact:** adcin.J.value has invalid writer ownership (no declared HAL_OUT source).
If an external HAL_OUT source is connected, it overwrites the hardware reading.
Structural and characterization tests remain possible. Canonical compatibility
cannot be claimed until the direction bug is fixed. This is a Phase 1 action item.

---

### DEC-CANON-002: Verify adcout functional conversion path

**Context:** `hal_export_adcout` IS called (PoKeysLibIOAsync.c:110). Canonical pins
`adcout.J.value` (HAL_IN — correct), `adcout.J.enable` (HAL_IN — correct), and all
canonical parameters are present. Supplementary PWM pins also coexist.
Issues #37 and #39 remain OPEN, likely because the functional conversion from
`adcout.J.value` to actual PWM hardware output is unverified.

**Evidence (C4 trace):** The conversion path IS implemented:
  adcout.J.value → hal_adcout_getscaledvalue → val/max_Voltage*PWMperiod → PWMduty[j]
  → PK_PWMUpdateAsync (CMD 0xCB) → hardware (PoKeysLibIOAsync.c:703–734).
  PK_PWMUpdateAsync is registered via the async scheduler at 100ms period
  (experimental/pokeys_async.c:1338). Status: IMPLEMENTED (code inspection only;
  HIL execution outside Phase 0 scope).

**Required decision:**
- Decide whether `adcout.J.PWMduty` (raw supplementary) coexists or is deprecated.
- Close issues #37 and #39 with reference to this C4 propagation evidence once HIL
  confirms end-to-end functional correctness.

---

### DEC-CANON-003: Shall encoder use hal_export_encoder or continue manual export?

**Context:** `hal_export_encoder` is a **hal-canon convention** (B-004); encoder is
NOT part of the official LinuxCNC CDI specification (A-002 covers only digin, digout,
adcin, adcout). Current implementation manually creates a subset of the hal-canon
encoder pins. The encoder struct in `sPoKeysEncoder` does not match `hal_encoder_t`
directly; it has additional PoKeys-specific options.
Legacy PoKeysCompEncoders.c uses different names: 4x_sampling (not x4_sampling),
FastEncoders.Options (not encoder.fast.Options).

**Options:**
- A: Adopt `hal_export_encoder` for the hal-canon encoder subset; keep extra params manually.
- B: Keep entirely manual export; align names with legacy E-007 for compatibility.

**Blocking:** Not blocking for compatibility — hal-canon encoder convention pins are mostly present.
Name differences vs legacy must be reconciled.

### DEC-HALCANON-001: Fix hal-canon direction bugs before canonical compatibility can be claimed

**Context:** Three direction bugs identified in the embedded hal-canon code (CONFLICT-009):
- `hal_export_digin`: `digin.in` exported as `HAL_IN` (should be `HAL_OUT`)
- `hal_export_digout`: `digout.out` exported as `HAL_OUT` (should be `HAL_IN`)
- `hal_export_adcin`: `adcin.value` exported as `HAL_IN` (should be `HAL_OUT`)

Fixing these requires modifying the embedded `hal-canon/hal_digital.c` and `hal-canon/hal_analog.c`.
NOTE: Do NOT import upstream commit 45adb952 — it is a regression introducing three new
wrong directions (see CONFLICT-011). A fresh correct fix is needed.

**Specific impact per bug:**
- `digout.out` as `HAL_OUT`: blocks normal external `HAL_OUT` command-source wiring;
  any attempt to connect a motion-controller or other `HAL_OUT` source to the same
  signal is rejected by `hal_link` because the signal already has a writer.
- `digin.in` and `adcin.value` as `HAL_IN`: incorrect writer ownership — the component
  writes hardware state to pins declared as readers; the signal has no declared
  `HAL_OUT` source and may be overwritten if an external `HAL_OUT` is connected.

**Test implications:**
- Structural and characterization tests (pin existence, type, cardinality) remain possible.
- Canonical HAL compatibility cannot be claimed until these bugs are corrected.

**Blocking:**
- digout.out (HIGH): blocks normal external HAL_OUT command-source wiring.
- digin.in, adcin.value (MEDIUM): incorrect ownership; should fix in same pass.

---

### DEC-CONFLICT010: How to resolve the CDI terminology overloading in issue #79

**Context:** LinuxCnc_PokeysLibComp issue #79 uses the term 'Canonical Device Interface'
to cover motion/PEv2, counters/PWM and PoNET in addition to the four official CDI device
types (digin, digout, adcin, adcout). The official LinuxCNC CDI source (A-002) defines
only the four types. See CONFLICT-010.

**Required decision:**
- Separate concerns: official CDI conformity for the four defined device types;
  established LinuxCNC conventions (e.g. encoder, motion pins) where primary sources exist;
  project/legacy compatibility contracts; PoKeys-specific extensions.
- Do not call motion/PEv2 pins 'canonical' unless a primary source supports it.

---

## 2. Backward Compatibility

### DEC-COMPAT-001: What is the canonical component name?

**Context:** Current implementation registers as `pokeys-async` with pins prefixed
`pokeys-async.0.*`. Legacy integration files use `pokeys.0.*` with component name
`pokeys`. All existing machine configurations must be rewritten to use the new name.
See CONFLICT-006.

**Options:**
- A: Keep `pokeys-async` as the official name; require migration of all HAL files.
- B: Change to `pokeys` to maintain backward compatibility.
- C: Support both via an alias or compatibility wrapper.

**Blocking:** Every compatibility test and migration guide depends on this decision.
All HAL file examples in both repositories use `pokeys`.

---

### DEC-COMPAT-002: Which PEv2 pins from issue #33 are required for backward compatibility?

**Context:** Issue #33 (CLOSED) specified many pins not currently implemented:
`digout.AxisEnable.out`, `stepgen.*`, `HomingSpeed`, `LimitOverride`,
`PulseEngineEnabled`, etc. Legacy integration files reference several of these.

**Required action:** Create a list of pins referenced in integration files (Authority G, E)
that must be present for backward compatibility. Separate from pins required by LinuxCNC
motion control (joints, homing, probing).

**Blocking:** Cannot design PEv2 compatibility tests without this list.

---

## 3. Object Naming

### DEC-NAME-001: What is the pin name for raw analog input?

**Context:** Issue #35 specifies `adcin.J.value-raw`. Implementation exports
`adcin.J.in.raw`. These names differ. Canonical name is `adcin.J.value` (scaled).

**Required decision:** Align `value-raw` vs `in.raw` naming. Decide whether to follow
issue #35 spec or use a different name.

---

### DEC-NAME-002: What is the canonical PEv2 axis index format?

**Context:** Current implementation uses `PEv2.%01d.*` (single digit). Legacy files
use `PEv2.0.*` through `PEv2.7.*` (single digits). This is consistent. However, if
axis indices ever exceed 7, the format may need revision.

**Required decision:** Confirm that `PEv2.0` through `PEv2.7` (8 axes max) is the
definitive naming convention.

---

### DEC-NAME-003: HAL name length budget

**Context:** Installed LinuxCNC 2.9.10: `HAL_NAME_LEN = 47` (confirmed, `/usr/include/linuxcnc/hal.h`;
source A-001). LinuxCNC upstream/master: `HAL_NAME_LEN = 55` at commit
`71bf88009d64fa15edbebf9250b65ee4454f9a05` `src/hal/hal.h` (source A-001b). With `pokeys-async.0` prefix (14 chars), current
names are ≤46 chars. With device index ≥10, borderline for the 47 limit.

**Required decision:**
1. Determine the target LinuxCNC version (2.9.x or upstream/master) and confirm
   the applicable `HAL_NAME_LEN` with a pinned commit SHA.
2. If targeting 2.9 (HAL_NAME_LEN = 47), audit all format strings for device indices > 9.
3. If targeting upstream (HAL_NAME_LEN = 55), current names are safe for indices ≤ 9.

---

## 4. Object Type and Direction

### DEC-TYPE-001: Should AxesState/AxesCommand be HAL_OUT u32 (raw bitmap) or decomposed bit pins?

**Context:** ADR-PEV2-003 (#129) covers the decomposition strategy. The struct in
`PoKeysLibHal.h` already has decomposed `AxesConfig_enabled[8]` etc. (hal_bit_t),
but these are not exported as HAL pins. Issue #33 specifies raw u32 pins for
`AxesState` and `AxesConfig`. Integration files use the raw u32.

**Options:**
- A: Expose raw u32 only (current implementation).
- B: Expose both raw u32 and decomposed bit pins.
- C: Expose only decomposed bit pins (breaking change vs. integration files).

**Blocking:** ADR-PEV2-003 must be implemented before per-axis homing logic can read
individual enable/config bits.

---

### DEC-TYPE-002: Should digin.J.invert be kept?

**Context:** `digin.J.invert` is a non-canonical extra parameter. The canonical
`digout.J.invert` is correct. For digital inputs, inversion is handled by writing
`PK_PinCap_invertPin` to the device's PinFunction register. Exposing it as a HAL
parameter allows runtime reconfiguration.

**Options:**
- A: Keep as PoKeys-specific extension (current).
- B: Remove; handle inversion only at device configuration time.

---

## 5. Lifecycle

### DEC-LIFE-001: How are defaults initialized for HAL parameters?

**Context:** All HAL pin memory is zeroed by `memset`. This means `encoder.I.scale`
defaults to 0.0 (not useful). `adcin.J.scale` (if added) needs a sensible default.
No explicit default assignment was found.

**Required decision:** Define sensible defaults for all scale/offset parameters
(e.g., scale=1.0, offset=0.0) and ensure they are written after `hal_malloc`.

---

## 6. Pin Cardinality

### DEC-CARD-001: How many per-axis PEv2 pins should be created?

**Context:** REQ #118 says use `nrOfAxes` as upper bound; nrOfAxes==0 → no pins.
ADR #128 says nrOfAxes==0 → create all 8 as fallback. Implementation always creates 8.
See CONFLICT-001.

**Required decision (before any test can be written for conditional creation):**
- Which source governs: REQ #118 or ADR #128?
- If ADR governs: is nrOfAxes > 0 → create nrOfAxes pins still required?
- If REQ governs: is the nrOfAxes==0 fallback to be removed?

---

### DEC-CARD-002: How many analog input channels?

**Context:** Implementation hardcodes 7 analog inputs (iAnalogInputs). `PoKeysLib.h`
and device info report actual count. Need to confirm whether 7 is correct for all
supported devices or whether it should be `device->info.iAnalogInputs`.

---

## 7. Physical Pin-Function Ownership

### DEC-PHYS-001: Who owns physical pin configuration at startup?

**Context:** PinFunction is read from device on connect, parsed by
`PK_ParsePinFunctionsResponse`, and written to HAL params. If an integrator changes
the PinFunction param, `PK_PinFunctionsSetAsync` writes it back. However, the order
of operations (read-then-write vs write-then-read) and the default priority are not
documented in any requirement or ADR.

**Required decision:** Define the authoritative source of PinFunction at startup:
device EEPROM or HAL parameter value.

---

## 8. PEv2 Decomposition

### DEC-PEV2-001: Define the full set of HAL pins required from issue #33 and REQ-F-PEV2-* series

**Context:** Issue #33 (CLOSED) specifies a large set of pins. Issues #116-#126 refine
specific subsets. Many pins from #33 are absent. Before Phase 1 testing, a definitive
list must exist.

**Required action:** Cross-reference issue #33 specification with REQ-F-PEV2-001/002
and produce a single authoritative pin list.

---

## 9. Homing Integration

### DEC-HOMING-001: Is pokeys_homecomp required for LinuxCNC homing, or can standard homing be used?

**Context:** Legacy integration files load `HOMEMOD=pokeys_homecomp` and connect
`joint.N.PEv2.AxesState`. Issue #310 (LinuxCnc_PokeysLibComp) shows this fails
if the pin doesn't exist. Standard LinuxCNC homing uses `joint.N.home-sw-in`.

**Required decision:** Clarify whether pokeysHal should support standard LinuxCNC
homing (via `joint.N.home-sw-in`), custom homing (via pokeys_homecomp), or both.
This determines which homing-related pins must be exported.

---

## 10. Initialization Fallback Behaviour

### DEC-INIT-001: What happens if device is not connected at hal_ready?

**Context:** If the PoKeys device is not connected when `rtapi_app_main` calls
`export_pev2_pins`, `nrOfAxes` will be 0. ADR #128 says create all 8 axes as
fallback. If device connects later, `nrOfAxes` may update but pins cannot be
retroactively created.

**Required decision:** Confirm the fallback policy. Document in the SDD (#133).

---

## 11. Deprecation and Aliases

### DEC-DEPR-001: What is the fate of legacy non-canonical pin names?

**Context:** Several implemented pin names differ from canonical or from issue
specifications (e.g., `adcin.J.in.hw` vs `adcin.J.value-raw`). If canonical
names are added, the non-canonical names may coexist or be deprecated.

**Required decision:** Define a deprecation policy and timeline for non-canonical pins.
