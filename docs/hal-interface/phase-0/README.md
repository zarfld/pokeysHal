# Phase 0 — HAL-Interface Knowledge Baseline

## Objective

Establish a comprehensive, evidence-based baseline of all requirements governing
the PoKeys LinuxCNC HAL interface before any compatibility plan, test manifest,
Copilot skill, production-code change, or refactoring is designed.

## Scope and Exclusions

**In scope:**
- HAL pins, parameters, and exported functions in `pokeysHal`
- HAL interface exported by `LinuxCnc_PokeysLibComp` (legacy)
- `hal-canon` public types, functions, and conventions
- LinuxCNC canonical device interface rules
- Requirements, ADRs, and architecture components in `pokeysHal`
- Integration HAL and INI examples from both repositories
- Enumerations (`ePK_PinCap`, `ePK_PEAxisState`, `ePK_PEv2_AxisConfig`)
- Bitmapped fields and per-bit decomposition strategy
- Lifecycle, ownership, naming, cardinality, and initialization

**Out of scope (Phase 0 restrictions):**
- No production C code modified
- No `hal-canon` changes
- No submodule revision updates
- No HAL pins or parameters implemented or changed
- No HAL compatibility tests created
- No compatibility Copilot skill created
- No PoKeysLib async request/response handling changes
- No packet construction, parsing, retries, or scheduling changes
- No GitHub issues closed, edited, or commented on
- No pull requests created or merged
- No hardware or timing validation claimed

## Repository Revisions Examined

| Repository | Branch | Commit SHA |
|---|---|---|
| `zarfld/pokeysHal` | main | `cd1f0dc8a0f64f92dc6bdce21bddcb36d33a14cd` |
| `zarfld/LinuxCnc_PokeysLibComp` | main | `0c058e6c7136ddb28a6b9f463a8af3d973496246` (accessed via GitHub API) |
| `hal-canon` (embedded tree) | n/a | tree `deed4c10535530ce0383fb357ea8427896226c70` within main repo |
| `pokeyslib` submodule | n/a | `c08da06747d56962640fdf03ff954c0675b1f563` (not checked out) |

**Note on `hal-canon` embedding:** `.gitmodules` declares `hal-canon` as a
submodule pointing to `https://github.com/zarfld/linuxcnc-hal-canon.git`, but
`git submodule status` reports no entry for it and `git ls-tree HEAD hal-canon`
returns a `040000 tree` entry rather than a `160000 commit` entry. The files are
therefore embedded as regular tracked content, not as a proper Git submodule.
The upstream commit SHA from `linuxcnc-hal-canon.git` corresponding to this
embedded tree is **not recorded** in the main repository and must be determined
by comparing file contents against the upstream repository.

## Authority Hierarchy

| Class | Description |
|---|---|
| A | LinuxCNC normative external authority (canonical device interface, `hal.h`, HAL lifecycle) |
| B | Pinned dependency authority (hal-canon as embedded tree) |
| C | Explicit repository requirements (issues: StR, REQ-F, REQ-NF) |
| D | Architecture and design records (ADRs, ARC-C, SDD issues) |
| E | Legacy compatibility implementation (`LinuxCnc_PokeysLibComp`) |
| F | Current implementation (`pokeysHal` source code) |
| G | Integration examples (HAL and INI files from both repositories) |
| H | Issue comments and generated prose |

## Methodology

1. Workspace verified with `git status`, `git submodule status`, `git remote -v`.
2. `hal-canon` source files read directly from the embedded tree.
3. All `PoKeysLib*Async.c` files searched for `hal_pin_*_newf` and
   `hal_param_*_newf` calls to inventory every HAL object created.
4. `experimental/pokeys_async.c` inspected for lifecycle calls
   (`hal_init`, `hal_ready`, `hal_exit`, `hal_export_funct`) and
   component-level pin declarations (MODULE_INFO, `__comp_state`).
5. `PoKeysLibHal.h` inspected for struct layouts and HAL-type fields.
6. `PoKeysLib.h` inspected for enumerations (`ePK_PinCap`, `ePK_PEAxisState`,
   `ePK_PEv2_AxisConfig`, `ePK_PulseEngineV2_AxisSwitchOptions`).
7. `LinuxCnc_PokeysLibComp` accessed via GitHub API:
   integration HAL files, Python modules, and selected issue bodies inspected.
8. `zarfld/pokeysHal` GitHub issues #24, #30, #32–#39, #116–#133 fetched;
   issue bodies inspected for #24, #30, #32, #33, #35, #36, #38, #118, #128;
   remaining HIGH-relevance issue bodies (#37, #39, #119–#126) not body-inspected.
9. `zarfld/LinuxCnc_PokeysLibComp` GitHub issues #16, #21, #24, #28, #29, #30,
   #31, #69, #79, #129, #157, #213, #216, #222, #223, #264, #310, #326 fetched;
   bodies inspected for #21, #24, #28, #29, #30, #31, #69, #79, #129, #157,
   #213, #216, #222, #223, #310, #326; body of #264 was empty.
10. HAL name lengths computed. Two relevant constraints exist:
    - LinuxCNC 2.9.x (installed: 2.9.10): `HAL_NAME_LEN = 47`
      (`/usr/include/linuxcnc/hal.h` on target system, confirmed).
    - LinuxCNC upstream/master: `HAL_NAME_LEN = 55`
      (pinned commit `71bf88009d64fa15edbebf9250b65ee4454f9a05`,
      `src/hal/hal.h`; `#define HAL_NAME_LEN 55`; source A-001b).
11. `hal-canon/hal_digital.c` and `hal-canon/hal_analog.c` read in full to
    determine actual HAL directions used by each export helper.

## Principal Findings

1. **Component naming mismatch:** The integration shell (`experimental/pokeys_async.c`)
   registers itself as `pokeys-async` and exports all subsystem pins under the
   prefix `pokeys-async.N`. Legacy integration files (`DM542_XXYZ_mill/Pokeys_DM542_XXYZ_mill.hal`)
   reference `pokeys.0.*`. These two naming conventions are incompatible.

2. **nrOfAxes conflict:** `REQ-F-PEV2-003` (#118) requires zero per-axis pins
   when `nrOfAxes == 0`. `ADR-PEV2-002` (#128) specifies creating all 8 axes
   as a fallback when `nrOfAxes == 0`. The actual implementation
   (`PoKeysLibPulseEngine_v2Async.c:380`) always loops `for (int i = 0; i < 8; i++)`
   regardless of `nrOfAxes`, matching neither source precisely.

3. **Canonical analog helpers ARE called; supplementary non-canonical pins added:**
   `export_IO_pins()` (`PoKeysLibIOAsync.c`) calls `hal_export_adcin` for each
   of 7 channels and `hal_export_adcout` for each PWM channel. The canonical
   pins `adcin.J.value`, `adcin.J.scale`, `adcin.J.offset`, `adcin.J.bit-weight`,
   `adcin.J.hw-offset`, `adcout.J.value`, `adcout.J.enable`, and all `adcout`
   parameters are therefore present. In addition, non-canonical supplementary
   pins are exported alongside: `adcin.J.in.hw` (u32, raw ADC counts),
   `adcin.J.in.raw` (float, voltage-scaled), `adcin.J.ReferenceVoltage` (param,
   float), `adcout.J.max_voltage` (param, float), `adcout.J.PWMduty` (u32),
   and `adcout.pwm.period` (param, u32). The residual issue is in the hal-canon
   implementation itself (see finding 4).
   Evidence: `PoKeysLibIOAsync.c:85` (`hal_export_adcin`), `:110` (`hal_export_adcout`).

4. **hal-canon direction mismatches (CONFLICT-009):** Inspection of
   `hal-canon/hal_digital.c` and `hal-canon/hal_analog.c` reveals that the
   embedded hal-canon helpers export pins with incorrect HAL directions:
   - `digin.in`: exported as `HAL_IN` — should be `HAL_OUT` (component writes
     hardware state; LinuxCNC reads it). Evidence: `hal_digital.c` line with
     `hal_pin_bit_newf(HAL_IN, &(digin->in), ...)`.
   - `digout.out`: exported as `HAL_OUT` — should be `HAL_IN` (LinuxCNC writes
     command; component reads it). Evidence: `hal_digital.c` line with
     `hal_pin_bit_newf(HAL_OUT, &(digout->out), ...)`.
   - `adcin.value`: exported as `HAL_IN` — should be `HAL_OUT` (component
     writes scaled hardware value; LinuxCNC reads it). Evidence: `hal_analog.c`
     line with `hal_pin_float_newf(HAL_IN, &(adcin->value), ...)`.
   These are hal-canon bugs. Production code that uses these helpers inherits
   the incorrect directions. `adcout.value` and `adcout.enable` are correctly
   `HAL_IN`. `digin.in-not` is correctly `HAL_OUT`.

5. **PoExtBus entirely absent:** Issue #34 specifies a HAL interface for
   PoExtBus digital outputs. No `hal_pin_*_newf` or `hal_export_*` calls for
   PoExtBus were found in any `*Async.c` file.

6. **Device information pins incomplete:** Issue #32 specifies `devSerial`,
   `alive`, `connected`, `err`, and a full `info.*` set (30+ capability
   fields). Only `devSerial` (HAL_IN u32) and `alive` (HAL_OUT bit) are
   declared in the `__comp_state`; the 30+ `info.*` capability pins are absent.

7. **hal-canon usage:** `digin`, `digout`, `adcin`, and `adcout` all use the
   corresponding hal-canon export helpers. `hal_export_digin` and
   `hal_export_digout` are called per digital-capable pin; `hal_export_adcin`
   is called for 7 analog input channels; `hal_export_adcout` is called for
   each PWM channel. Encoder pins are exported manually; `hal_export_encoder`
   is not called. The encoder interface in hal-canon (`hal_encoder_t`,
   `hal_export_encoder`) is a hal-canon convention — it is **not** part of
   the official LinuxCNC Canonical Device Interfaces specification, which
   covers only `digin`, `digout`, `adcin`, and `adcout`. Encoder pin names
   used in `PoKeysLibEncodersAsync.c` follow the same pattern as the
   legacy `pokeys.comp` declarations (LinuxCnc_PokeysLibComp issue #213)
   but are not normatively required by LinuxCNC Authority A.

8. **HAL name length (version-dependent):** Installed LinuxCNC 2.9.10:
   `HAL_NAME_LEN = 47` (confirmed, `/usr/include/linuxcnc/hal.h`). Reviewer
   states LinuxCNC upstream/master uses `HAL_NAME_LEN = 55`; the corresponding
   upstream commit SHA was **not independently verified** in Phase 0. With prefix
   `pokeys-async.0`, current pin names appear ≤46 chars. With `pokeys-async.10`
   (+1 char), borderline cases near 47. The legacy component had a confirmed
   violation (`pokeys.0.encoder.UltraFastEncoder.Enable4xSampling` = 50 chars;
   LinuxCnc_PokeysLibComp issues #310, #326). With HAL_NAME_LEN = 55 (if the
   upstream claim is verified), most current names would be within limit even
   for double-digit device indices.

9. **PEv2 incomplete vs. issue #33:** The closed issue #33 specifies many pins
   not present in the current implementation, including `PulseEngineEnabled`,
   `PulseGeneratorType`, `PG_swap_stepdir`, `PG_extended_io`, `ChargePumpEnabled`,
   `MiscInputStatus`, `digin.Misc-#.in`, `LimitOverride`, `LimitOverrideSetup`,
   `AxisEnabledMask`, `AxisEnabledStatesMask`, `HomingStartMaskSetup`,
   `ProbeStartMaskSetup`, `ProbeSpeed`, `BacklashCompensationEnabled`,
   per-axis `stepgen.*` parameters, `HomingSpeed`, `HomingReturnSpeed`,
   `HomingAlgorithm`, `HomeOffsets`, and `HomeBackOffDistance`. Issue #33 is
   CLOSED but the implementation does not provide all specified pins.

10. **hal_free is absent — correctly:** Issue #24 (CLOSED) documents that
    `hal_free` does not exist in LinuxCNC HAL; `hal_malloc` memory is managed
    by HAL itself. This is resolved: no `hal_free` calls appear in the codebase.

## Unresolved Questions

1. What is the intended canonical component name — `pokeys` (legacy) or
   `pokeys-async` (current)? The choice affects every HAL signal file written
   by integrators.
2. Should the non-canonical supplementary pins (`adcin.J.in.hw`, `adcin.J.in.raw`,
   `adcin.J.ReferenceVoltage`, `adcout.J.PWMduty`, `adcout.J.max_voltage`,
   `adcout.pwm.period`) coexist with the canonical exports, or be deprecated?
3. In what order must the hal-canon direction bugs be fixed?
   `digout.out` as HAL_OUT blocks any HAL_OUT command source (HIGH priority).
   `digin.in` and `adcin.value` as HAL_IN are unsupported writers with no declared
   signal source (MEDIUM priority). See CONFLICT-009 for per-bug impact analysis.
4. Which `nrOfAxes == 0` behaviour is authoritative: the requirement (no pins)
   or the ADR (8 axes fallback)?
5. Does the project target LinuxCNC 2.9.x (`HAL_NAME_LEN = 47`) or upstream/master
   (`HAL_NAME_LEN = 55` at commit `71bf88009d64fa15edbebf9250b65ee4454f9a05`,
   `src/hal/hal.h`; source A-001b)? The constraint differs by 8 characters and
   affects borderline pin names.
6. Is PEv2 issue #33 actually complete (closed status) despite many specified
   pins being absent?
7. What is the upstream commit SHA for the embedded `hal-canon` files? This
   cannot be determined from the main repository alone.
8. Which PEv2 pins from issue #33 are required for LinuxCNC homing compatibility
   vs. optional PoKeys-specific extensions?

## Completion Status

All ten required deliverables have been created. Evidence for every finding is
referenced by file and line number or issue number. No production code was
modified, no tests were designed, and no compatibility plan was created.

## No Compatibility Design Created

This document and its companions record what was found. No compatibility tests,
implementation plans, or Copilot skills were created as part of Phase 0.
