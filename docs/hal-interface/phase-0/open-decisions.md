# Open Decisions — Phase 0 HAL-Interface Baseline

These decisions must be made before compatibility tests or implementation work
can begin. Phase 0 does not make these decisions. They are listed here for the
Phase 1 planning review.

---

## 1. LinuxCNC Canonical Conformity

### DEC-CANON-001: Shall adcin follow the canonical device interface?

**Context:** The LinuxCNC CDI requires `adcin.<J>.value` (float, HAL_OUT) and
parameters `scale`, `offset`, `bit-weight`, `hw-offset`. Issue #35 (CLOSED)
specified these. Implementation provides non-canonical `adcin.J.in.hw` (u32)
and `adcin.J.in.raw` (float) instead. A `hal_adcin_t` Canon struct is allocated
and partially populated internally but never exported.

**Options:**
- A: Call `hal_export_adcin` for each analog input channel; align naming with CDI.
- B: Keep current non-canonical pins; add canonical pins in addition.
- C: Keep current non-canonical pins; document them as the PoKeys-specific interface.

**Blocking:** Any compatibility test or integration file that references adcin must
know which pin names to use.

---

### DEC-CANON-002: Shall adcout follow the canonical device interface?

**Context:** CDI requires `adcout.<J>.value` (float, HAL_IN) and `adcout.<J>.enable`
(bit, HAL_IN). Legacy integration files already reference these. `hal_adcout_t` is
allocated but `hal_export_adcout` is never called. Current implementation provides
only PWM-based non-canonical pins.

**Options:**
- A: Call `hal_export_adcout`; add canonical value/enable pins; keep PWMduty as supplementary.
- B: Replace PWMduty-based interface entirely with canonical interface.
- C: Keep current; document as PoKeys-specific.

**Blocking:** Integration files reference `adcout.0.value` and `adcout.0.enable`.
Without decision, machine configurations for spindle speed output fail.

---

### DEC-CANON-003: Shall encoder use hal_export_encoder or continue manual export?

**Context:** `hal_export_encoder` from hal-canon creates canonical pins including
`velocity_resolution` and `max_index_vel`. Current implementation manually creates
a subset. The encoder struct in `sPoKeysEncoder` does not match `hal_encoder_t`
directly; it has additional PoKeys-specific options.

**Options:**
- A: Adopt `hal_export_encoder` for the canonical subset; keep extra params manually.
- B: Keep entirely manual export.

**Blocking:** Not blocking for compatibility — encoder canonical pins are mostly present.

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

**Context:** `HAL_NAME_LEN = 47`. Prefix `pokeys-async.N` (with N a single digit)
is 14 chars, leaving 33 chars for the subsystem path. With N = two digits (10+),
prefix is 15 chars, leaving 32. Some current names near the limit.

**Required decision:** If the component is renamed (DEC-COMPAT-001), recompute the
budget. Define a maximum prefix length and a review process for long pin names.

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
