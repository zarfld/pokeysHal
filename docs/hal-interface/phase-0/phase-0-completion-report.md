# Phase 0 Completion Report — HAL-Interface Baseline

Baseline date: 2026-08-03.
Repository: zarfld/pokeysHal@cd1f0dc8a0f64f92dc6bdce21bddcb36d33a14cd.

---

## Acceptance Table

| Criterion | Status | Evidence | Residual gap |
|---|---|---|---|
| 1. Both repositories inspected | PASS | pokeysHal inspected locally; LinuxCnc_PokeysLibComp accessed via GitHub API | LinuxCnc_PokeysLibComp not cloned; some files not inspected |
| 2. Open and closed issues searched | PARTIAL | pokeysHal: all issues listed; bodies inspected for #24, #30, #32, #33, #35, #36, #38, #118, #128. LinuxCnc_PokeysLibComp: bodies inspected for #16 (title only), #21, #24, #28, #29, #30, #31, #69, #79, #129, #157, #213, #216, #222, #223, #310, #326; body of #264 was empty via API. Bodies also inspected: pokeysHal #37, #39, #119–#126 (all bodies retrieved); LinuxCnc_PokeysLibComp #24, #30, #31, #73. Comments inspected: pokeysHal #33 (3 comments), #35 (1 comment); LinuxCnc_PokeysLibComp #216 (21), #222 (5), #264 (1), #310 (first 2). Not inspected: most LinuxCnc_PokeysLibComp comments. | Most LinuxCnc_PokeysLibComp issue comments (≥5 comments) partially inspected; issues with 0 comments confirmed empty |
| 3. Issue comments inspected where relevant | PARTIAL | pokeysHal: #32,34,36-38,41,116-133 have 0 comments (confirmed); #33 (3 inspected: PEv2 example, lifecycle issues, PoKeysLib ref); #35 (1 inspected: confirms adcin value-raw/value/scale/offset names); #39 (2 inspected: adcout/PWM implementation evidence). LinuxCnc_PokeysLibComp: #28,30,31,79,157,213,223 have 0-2 (confirmed); #21,24,28,79,157: 0-1 inspected; #216 (21), #222 (5), #264 (1), #310 (14): partially inspected. | LinuxCnc_PokeysLibComp #310 (14) and #326 (12) not fully inspected |
| 4. Official LinuxCNC rules recorded | PASS | HAL_NAME_LEN=47 confirmed (/usr/include/linuxcnc/hal.h; source A-001). HAL_NAME_LEN=55 confirmed at commit 71bf88009d64fa15edbebf9250b65ee4454f9a05 src/hal/hal.h (source A-001b). Official CDI source inspected at 71bf88009d64fa15edbebf9250b65ee4454f9a05 docs/src/hal/canonical-devices.adoc (source A-002): defines digin, digout, adcin, adcout only. Upstream hal.h independently verified via GitHub API: #define HAL_NAME_LEN 55 at 71bf8800 (blob 17372ccd); #define HAL_NAME_LEN 47 at v2.9.10 (blob 5480d937). | hal-canon/hal_canon.h consulted for pin-level detail beyond CDI adoc scope |
| 5. Exact hal-canon provenance recorded | PASS | Embedded tree deed4c10535530ce0383fb357ea8427896226c70 matches upstream commit 995d7057dd5403865d423aab64ba30d81ccd5ee0 (zarfld/linuxcnc-hal-canon, 2025-06-08). Independently verified by cloning and comparing tree SHAs. See CONFLICT-008 for dependency-tracking status. | None |
| 6. Legacy HAL interface extracted from source | PASS | Integration HAL files, Python layer, PoKeysCompIO.c (E-006), PoKeysCompEncoders.c (E-007), pokeys_rt/pokeys.comp (E-008), PokeysCompPulsEngine_base.c (E-009), pokeys_homecomp.comp (E-010) all inspected. Full PEv2 parity table in E-009 (163 parity rows, 162 active exports, 1 commented export; exact tuple validator passes; see legacy-pev2-parity.yaml). homecomp-owned pins documented as separate counterpart ABI (out-of-scope for pokeysHal). | E-009 parity table: 'absent' patterns from CONFLICT-007 not individually tracked as requirements; CONFLICT-012 is a counterpart defect, not a pokeysHal extraction gap |
| 7. Current HAL interface extracted from source | PASS | All PoKeysLib*Async.c files searched. hal_export_adcin and hal_export_adcout calls confirmed by full read of PoKeysLibIOAsync.c. hal-canon/hal_digital.c and hal_analog.c read in full; direction mismatches documented. | No automated enumeration tool |
| 8. Lifecycle and ownership documented | PARTIAL | pokeysHal internal lifecycle (Section A): all subsystems documented; hal_malloc, export_*_pins(), hal_ready, hal_exit confirmed. External counterpart (Section B): pokeys_homecomp is a separate component; CONFLICT-012 (unreachable init) is a counterpart defect unrelated to pokeysHal internal correctness. End-to-end integration lifecycle (Section C): integration-links.yaml is authoritative. | PARTIAL because CONFLICT-012 in the counterpart component remains unresolved; end-to-end integration lifecycle cannot be fully assessed |
| 9. Enumerations and bitmaps documented | PASS | ePK_PinCap, ePK_PEAxisState (14 values: {0,1,2,8,9,10,11,12,13,14,15,16,20,30}), ePK_PEv2_AxisConfig, ePK_PulseEngineV2_AxisSwitchOptions, ePK_PEState (15 values in PEV2G-006: PulseEngineState enum), pokeys_home_command_t (4 values in HOMECOMP-007) all documented in requirement-catalogue.yaml. | None |
| 10. Canonical and project-specific extensions distinguished | PASS | canonical-vs-legacy-matrix.md classifies ~72 HAL objects. Encoder reclassified as hal-canon convention (not CDI). adcout status updated to reflect implemented conversion path. HAL_NAME_LEN target version is an open decision (DEC-NAME-003), not missing evidence. | None |
| 11. Contradictions recorded | PASS | 14 conflicts documented. CONFLICT-009 added: hal-canon direction mismatches — digout.out as HAL_OUT blocks normal external HAL_OUT command-source wiring; digin.in and adcin.value have invalid writer ownership. Structural and characterization tests remain possible. Canonical compatibility cannot be claimed until corrected. CONFLICT-003/004 corrected. CONFLICT-010 added. | Additional conflicts may exist in uninspected issues |
| 12. No production code changed | PASS | Branch hal-compatibility contains ten Phase 0 documentation files plus the committed prompt (.github/prompts/HAL-compatibility_Phase 0 — Establish the HAL-interface knowledge baseline.prompt.md). No production source (.c, .h), test, fixture, submodule, or hal-canon file was modified. git diff --check passes with no whitespace errors. | n/a |
| 13. No compatibility tests designed | PASS | No test files created. No test specifications written. | n/a |
| 14. No unresolved claim presented as fact | PARTIAL | All 14 conflicts documented with evidence. Direction/type errors in traceability corrected. Issue-inventory cross-repo source-ID contamination corrected. HOMECOMP-007 conflicts field populated. | CONFLICT-013 and CONFLICT-014 remain unresolved; counterpart and current component AxesCommand semantics are not functionally compatible |

---

## Validation Evidence

Commands executed during Phase 0:

```
git status --short
git branch --show-current
git rev-parse HEAD
git remote -v
git submodule status
git ls-tree HEAD hal-canon
git rev-parse HEAD:hal-canon
git show HEAD:.gitmodules
cat .gitmodules
ls -la hal-canon/
cd hal-canon && git log --oneline -3 (shows main repo HEAD — confirms hal-canon is not a proper submodule)
cat hal-canon/hal_canon.h
cat hal-canon/README.md
grep -n [patterns] PoKeysLib*.c PoKeysLibHal.h PoKeysLib.h
grep -n [patterns] PoKeysLib*Async.c
cat PoKeysLibIOAsync.c
sed -n [ranges] PoKeysLibPulseEngine_v2Async.c
sed -n [ranges] experimental/pokeys_async.c
grep -rn [patterns] experimental/pokeys_async.c
cat experimental/pokeys_async.comp
gh repo view zarfld/LinuxCnc_PokeysLibComp
gh api repos/zarfld/LinuxCnc_PokeysLibComp/git/ref/heads/main
gh api repos/zarfld/LinuxCnc_PokeysLibComp/git/trees/main?recursive=1
gh api repos/zarfld/LinuxCnc_PokeysLibComp/contents/DM542_XXYZ_mill/Pokeys_DM542_XXYZ_mill.hal
gh api repos/zarfld/LinuxCnc_PokeysLibComp/contents/DM542_XXYZ_mill/pokeys_homing.hal
gh api repos/zarfld/LinuxCnc_PokeysLibComp/contents/pokeys_py/digital_io.py
gh issue list --repo zarfld/pokeysHal --state all --limit 50 [...]
gh issue view 33 --repo zarfld/pokeysHal [...]
gh issue view 35 --repo zarfld/pokeysHal [...]
gh issue view 118 --repo zarfld/pokeysHal [...]
gh issue view 128 --repo zarfld/pokeysHal [...]
gh issue view 32 --repo zarfld/pokeysHal [...]
gh issue view 310 --repo zarfld/LinuxCnc_PokeysLibComp [...]
gh issue view 326 --repo zarfld/LinuxCnc_PokeysLibComp [...]
grep -r HAL_NAME_LEN /usr/include/linuxcnc/
```

Commands that could not be run:
- `git submodule update --init hal-canon` (not run — out of scope; would modify state)
- pokeyslib source inspection (submodule not initialized; SHA c08da06 not checked out)
- Direct clone of LinuxCnc_PokeysLibComp (not needed; API access sufficient for Phase 0 scope)
- Running `make` or `halcompile` to verify build (not run — out of scope for Phase 0)

YAML validity: All four YAML artifacts validated using Python 3 and PyYAML:
  source-register.yaml, requirement-catalogue.yaml, legacy-pev2-parity.yaml,
  integration-links.yaml — all parsed successfully with no errors.

No source, build, test, fixture or submodule files were changed.

```
git diff --stat
```
Expected output: no tracked file modifications.

---

## Repository Revisions Examined

| Repository | Revision |
|---|---|
| zarfld/pokeysHal | `cd1f0dc8a0f64f92dc6bdce21bddcb36d33a14cd` |
| zarfld/LinuxCnc_PokeysLibComp | `0c058e6c7136ddb28a6b9f463a8af3d973496246` (main HEAD, via GitHub API) |
| hal-canon (embedded tree) | tree `deed4c10535530ce0383fb357ea8427896226c70` |
| pokeyslib submodule | `c08da06747d56962640fdf03ff954c0675b1f563` (pinned, not checked out) |

---

## Files Created

All files are in `docs/hal-interface/phase-0/`:

1. `README.md` — objective, scope, methodology, principal findings
2. `source-register.yaml` — 56 source entries, classes A–G
3. `requirement-catalogue.yaml` — 51 interface requirement entries
4. `lifecycle-ownership-matrix.md` — per-subsystem ownership table
5. `canonical-vs-legacy-matrix.md` — ~72 interface items classified
6. `conflict-register.md` — 14 conflicts documented
7. `traceability.md` — 14 traceability chains
8. `issue-inventory.md` — issue and PR inventory
9. `open-decisions.md` — 23 decisions grouped by topic
10. `phase-0-completion-report.md` — this file
11. `legacy-pev2-parity.yaml` — 163 rows parity table (162 active, 1 commented)
12. `integration-links.yaml` — 4 integration link records
13. `tools/extract_legacy_pev2_exports.py` — exact-tuple extractor/validator

## Summary Statistics

| Category | Count |
|---|---|
| Source register entries | 56 |
| Requirement catalogue entries | 51 |
| Conflicts registered | 14 |
| Open decisions required | 23 |
| Traceability chains | 14 |
| Issues inventoried (pokeysHal) | 45 |
| Issues inventoried (LinuxCnc_PokeysLibComp) | 18 |
| Pull requests inventoried | 1 |

---

## Most Consequential Unresolved Decisions

1. **DEC-HALCANON-001** — hal-canon direction bugs: digout.out as HAL_OUT blocks
   normal external HAL_OUT command-source wiring; digin.in and adcin.value have
   invalid writer ownership. Structural/characterization tests remain possible;
   canonical compatibility cannot be claimed until corrected. (CONFLICT-009)

2. **DEC-COMPAT-001** — Component name: `pokeys` vs `pokeys-async`. Every HAL file
   and compatibility test depends on this. (CONFLICT-006)

3. **DEC-CARD-001** — Conditional PEv2 axis creation: REQ #118 vs ADR #128 vs
   implementation (always 8). (CONFLICT-001)

4. **DEC-COMPAT-002** — Which PEv2 pins from issue #33 are required for backward
   compatibility. Without this list, PEv2 compatibility cannot be validated. (CONFLICT-007)

5. **DEC-CANON-001** — Fix adcin supplementary pin names ('in.raw' vs 'value-raw')
   and correct default for scale parameter (0.0 is not useful). (CONFLICT-003)

---

## Suggested Next Steps

Review the Phase 0 findings as a team. Specifically:

1. Resolve the 14 conflicts in `conflict-register.md`. Priority order:
   CONFLICT-009 (hal-canon directions), CONFLICT-006 (component naming),
   CONFLICT-002 (AxisEnable missing), CONFLICT-001 (nrOfAxes), CONFLICT-007
   (issue #33 incomplete).
2. Make the 23 open decisions in `open-decisions.md`, starting with
   DEC-HALCANON-001 (Phase 1 blocker), DEC-COMPAT-001, DEC-CARD-001.
3. Only then begin Phase 1 (compatibility test design).

---

```
PHASE 0 BASELINE INCOMPLETE
```

## Missing Evidence Requiring Resolution Before Phase 1

Derived from criteria currently marked PARTIAL:

2 (issues searched): Issue #41 (pokeysHal) body not retrieved.

3 (comments): LinuxCnc_PokeysLibComp #216 (21 comments) and #310 (14 comments)
  partially inspected; remaining content may contain implementation evidence.

8 (lifecycle/init): CONFLICT-012 unresolved — volatile_home=1 unreachable;
  actual initial value is 0 (zeroed shmem). No runtime read of volatile_home
  found in inspected source; external or generated consumers (LinuxCNC homing
  module) have not been fully verified. Runtime impact remains unresolved.

14 (no contradictions): Traceability direction/type errors corrected; CONFLICT-013
  (AxesCommand enum mismatch) and CONFLICT-014 (no reachable consumer) remain
  unresolved. Issue-inventory cross-repo source-ID contamination corrected
  (LC_PKComp #24 and #129 no longer carry zarfld/pokeysHal source IDs).
  HOMECOMP-007 conflicts field now references CONFLICT-013 and CONFLICT-014.
  Remaining contradictions: CONFLICT-013 and CONFLICT-014 are registered but not
  resolved; they must be resolved before this criterion can be PASS.

No PASS criterion appears above.
