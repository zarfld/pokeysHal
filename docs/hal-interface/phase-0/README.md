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
8. `zarfld/pokeysHal` GitHub issues #24, #32–#39, #116–#133 fetched.
9. `zarfld/LinuxCnc_PokeysLibComp` GitHub issues #310 and #326 fetched.
10. HAL name lengths computed against `HAL_NAME_LEN = 47`
    (`/usr/include/linuxcnc/hal.h` on target system).

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

3. **Analog input interface is non-canonical:** Issue #35 specifies `value` and
   `value-raw` following the canonical `adcin` interface. The implementation
   exports `adcin.J.in.hw` (u32, raw ADC counts) and `adcin.J.in.raw` (float,
   voltage-scaled), omitting the canonical `value` pin and all canonical
   parameters (`scale`, `offset`, `bit-weight`, `hw-offset`). A partial
   canonical `adcin` struct (`Canon.value`, `Canon.scale`, `Canon.offset`) is
   updated internally but never exported to HAL.

4. **Analog output interface is non-canonical:** Issues #37 and #39 reference
   the canonical `adcout` interface. The current implementation provides only
   `adcout.J.PWMduty` (u32) and `adcout.J.max_voltage` (param, float), using
   the PWM peripheral. The canonical `value`, `enable`, `scale`, `offset`,
   `high-limit`, `low-limit`, `bit-weight`, `hw-offset` objects are absent.

5. **PoExtBus entirely absent:** Issue #34 specifies a HAL interface for
   PoExtBus digital outputs. No `hal_pin_*_newf` or `hal_export_*` calls for
   PoExtBus were found in any `*Async.c` file.

6. **Device information pins incomplete:** Issue #32 specifies `devSerial`,
   `alive`, `connected`, `err`, and a full `info.*` set (30+ capability
   fields). Only `devSerial` (HAL_IN u32) and `alive` (HAL_OUT bit) are
   declared in the `__comp_state`; the 30+ `info.*` capability pins are absent.

7. **hal-canon partially followed:** `digin` and `digout` use `hal_export_digin`
   and `hal_export_digout` from hal-canon. `adcin` and `adcout` define a `Canon`
   sub-struct in `sPoKeysAnalogData` (using `hal_adcin_t`/`hal_adcout_t`) but
   never call `hal_export_adcin` or `hal_export_adcout`; they export a different
   set of non-canonical pins instead. The encoder canonical struct
   (`hal_encoder_t`) is not used; encoder pins are exported manually.

8. **HAL name length:** `HAL_NAME_LEN = 47` on the target system. Several pin
   name format strings, when rendered with a two-character prefix index, produce
   names of 43–46 characters — within the limit. However, multi-digit device IDs
   (e.g., `pokeys-async.10`) push some names over the limit. The legacy
   component (`LinuxCnc_PokeysLibComp`) had a confirmed name-length violation
   (`pokeys.0.encoder.UltraFastEncoder.Enable4xSampling` = 50 chars,
   reported in issues #310 and #326).

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
2. Should the analog input interface follow the `hal-canon` `adcin` contract
   exactly, or expose raw hardware values in addition?
3. Is the partially-canonical `Canon` sub-struct (`sPoKeysAnalogData.Canon`)
   intended to eventually replace the current non-canonical export, or to coexist?
4. Which `nrOfAxes == 0` behaviour is authoritative: the requirement (no pins)
   or the ADR (8 axes fallback)?
5. Is PEv2 issue #33 actually complete (closed status) despite many specified
   pins being absent?
6. What is the upstream commit SHA for the embedded `hal-canon` files? This
   cannot be determined from the main repository alone.
7. Which PEv2 pins from issue #33 are required for LinuxCNC homing compatibility
   vs. optional PoKeys-specific extensions?

## Completion Status

All ten required deliverables have been created. Evidence for every finding is
referenced by file and line number or issue number. No production code was
modified, no tests were designed, and no compatibility plan was created.

## No Compatibility Design Created

This document and its companions record what was found. No compatibility tests,
implementation plans, or Copilot skills were created as part of Phase 0.
