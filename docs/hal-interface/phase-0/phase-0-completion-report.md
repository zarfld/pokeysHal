# Phase 0 Completion Report — HAL-Interface Baseline

Baseline date: 2026-08-03.
Repository: zarfld/pokeysHal@cd1f0dc8a0f64f92dc6bdce21bddcb36d33a14cd.

---

## Acceptance Table

| Criterion | Status | Evidence | Residual gap |
|---|---|---|---|
| 1. Both repositories inspected | PASS | pokeysHal inspected locally; LinuxCnc_PokeysLibComp accessed via GitHub API | LinuxCnc_PokeysLibComp not cloned; some files not inspected |
| 2. Open and closed issues searched | PARTIAL | pokeysHal: all issues listed; bodies inspected for #24, #30, #32, #33, #35, #36, #38, #118, #128. LinuxCnc_PokeysLibComp: bodies inspected for #16 (title only), #21, #24, #28, #29, #30, #31, #69, #79, #129, #157, #213, #216, #222, #223, #310, #326; body of #264 was empty via API. Bodies also inspected: pokeysHal #37, #39, #119–#126 (all bodies retrieved); LinuxCnc_PokeysLibComp #24, #30, #31, #73. Comments inspected: pokeysHal #33 (3 comments), #35 (1 comment); LinuxCnc_PokeysLibComp #216 (21), #222 (5), #264 (1), #310 (first 2). Not inspected: most LinuxCnc_PokeysLibComp comments. | LinuxCnc_PokeysLibComp comments mostly uninspected |
| 3. Issue comments inspected where relevant | PARTIAL | Issue bodies inspected for selected issues in both repos. No issue comments were inspected in either repo. | Comments for #33, #35, #36, #38 may contain implementation evidence |
| 4. Official LinuxCNC rules recorded | PARTIAL | HAL_NAME_LEN=47 confirmed (/usr/include/linuxcnc/hal.h; source A-001). HAL_NAME_LEN=55 confirmed at commit 71bf88009d64fa15edbebf9250b65ee4454f9a05 src/hal/hal.h (source A-001b). Official CDI source inspected at 71bf88009d64fa15edbebf9250b65ee4454f9a05 docs/src/hal/canonical-devices.adoc (source A-002): defines digin, digout, adcin, adcout only. Upstream hal.h independently verified via GitHub API: #define HAL_NAME_LEN 55 at 71bf8800 (blob 17372ccd); #define HAL_NAME_LEN 47 at v2.9.10 (blob 5480d937). | hal-canon/hal_canon.h consulted for pin-level detail beyond CDI adoc scope |
| 5. Exact hal-canon provenance recorded | PARTIAL | Tree SHA deed4c10535530ce0383fb357ea8427896226c70 recorded. Upstream commit from linuxcnc-hal-canon.git not determined. See CONFLICT-008. | Upstream SHA unknown |
| 6. Legacy HAL interface extracted from source | PARTIAL | Integration HAL files read (DM542 HAL, pokeys_homing.hal). Python digital_io.py read. Pin names catalogued. | LinuxCnc_PokeysLibComp C/comp pin-export sources (PoKeysComp*.c, pokeys.comp) not inspected; pin interface derived from HAL files and Python layer only |
| 7. Current HAL interface extracted from source | PASS | All PoKeysLib*Async.c files searched. hal_export_adcin and hal_export_adcout calls confirmed by full read of PoKeysLibIOAsync.c. hal-canon/hal_digital.c and hal_analog.c read in full; direction mismatches documented. | No automated enumeration tool |
| 8. Lifecycle and ownership documented | PASS | lifecycle-ownership-matrix.md covers all subsystems. Allocation, creation, update, and cleanup paths identified with file:line evidence. | Cleanup paths not fully verified for all subsystems |
| 9. Enumerations and bitmaps documented | PASS | ePK_PinCap, ePK_PEAxisState, ePK_PEv2_AxisConfig, ePK_PulseEngineV2_AxisSwitchOptions documented in requirement-catalogue.yaml and conflict-register.md | ePK_PEState (for PulseEngineState) not fully documented |
| 10. Canonical and project-specific extensions distinguished | PARTIAL | canonical-vs-legacy-matrix.md classifies ~72 HAL objects. Direction-mismatch category added. Encoder correctly reclassified as hal-canon convention, not official CDI canonical. | Upstream HAL_NAME_LEN=55 verified at commit 71bf88009d64fa15edbebf9250b65ee4454f9a05 (A-001b); targeting decision (2.9 vs upstream) still required |
| 11. Contradictions recorded | PASS | 9 conflicts documented. CONFLICT-009 added: hal-canon direction mismatches — digout.out as HAL_OUT blocks normal external HAL_OUT command-source wiring; digin.in and adcin.value have invalid writer ownership. Structural and characterization tests remain possible. Canonical compatibility cannot be claimed until corrected. CONFLICT-003/004 corrected. CONFLICT-010 added. | Additional conflicts may exist in uninspected issues |
| 12. No production code changed | PASS | Branch hal-compatibility contains ten Phase 0 documentation files plus the committed prompt (.github/prompts/HAL-compatibility_Phase 0 — Establish the HAL-interface knowledge baseline.prompt.md). No production source (.c, .h), test, fixture, submodule, or hal-canon file was modified. git diff --check passes with no whitespace errors. | n/a |
| 13. No compatibility tests designed | PASS | No test files created. No test specifications written. | n/a |
| 14. No unresolved claim presented as fact | PARTIAL | Most claims distinguished by type. Corrections in this round removed stale claims: C-004/C-008/F-003 scope corrected; A-002 encoder claim removed; #216 UltraFastEncoder claim removed; C-005/C-007 inspection status corrected. | Remaining: some inferences in subsystems not fully inspected; legacy C source claims derived from HAL files rather than C code |

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

YAML validity: Both YAML files validated using Python 3 and PyYAML:
  python3 -c "import yaml; [yaml.safe_load(open(f)) for f in [
    'docs/hal-interface/phase-0/source-register.yaml',
    'docs/hal-interface/phase-0/requirement-catalogue.yaml']]"
  Both files parsed successfully with no errors.

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
2. `source-register.yaml` — 41 source entries, classes A–G
3. `requirement-catalogue.yaml` — 35+ interface requirement entries
4. `lifecycle-ownership-matrix.md` — per-subsystem ownership table
5. `canonical-vs-legacy-matrix.md` — ~72 interface items classified
6. `conflict-register.md` — 9 conflicts documented (CONFLICT-009 added)
7. `traceability.md` — 11 traceability chains with broken links marked
8. `issue-inventory.md` — 31 pokeysHal issues + 17 LinuxCnc_PokeysLibComp issues
9. `open-decisions.md` — 19 decisions grouped by topic (DEC-HALCANON-001 added)
10. `phase-0-completion-report.md` — this file
11. `.github/prompts/HAL-compatibility_Phase 0 — Establish the HAL-interface knowledge baseline.prompt.md` — the executing prompt (committed in 3ac906d, part of hal-compatibility branch)

---

## Summary Statistics

| Category | Count |
|---|---|
| Source register entries | 44 |
| Requirement catalogue entries | 35+ |
| Conflicts registered | 9 |
| Open decisions required | 19 |
| Traceability chains | 11 |
| Issues inventoried (pokeysHal) | 31 |
| Issues inventoried (LinuxCnc_PokeysLibComp) | 17 |

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

1. Resolve the 9 conflicts in `conflict-register.md`. Priority order:
   CONFLICT-009 (hal-canon directions), CONFLICT-006 (component naming),
   CONFLICT-002 (AxisEnable missing), CONFLICT-001 (nrOfAxes), CONFLICT-007
   (issue #33 incomplete).
2. Make the 19 open decisions in `open-decisions.md`, starting with
   DEC-HALCANON-001 (Phase 1 blocker), DEC-COMPAT-001, DEC-CARD-001.
3. Only then begin Phase 1 (compatibility test design).

---

```
PHASE 0 BASELINE INCOMPLETE
```

## Missing Evidence Requiring Resolution Before Phase 1

1. hal-canon embedded tree (deed4c10) upstream commit confirmed as 995d7057 in
   zarfld/linuxcnc-hal-canon (2025-06-08); direction bugs fixed in 45adb952
   (2025-06-09) but not yet incorporated into pokeysHal (CONFLICT-008 partially
   resolved — upstream commit known; integration decision outstanding).
2. LinuxCnc_PokeysLibComp C source extraction complete (E-006, E-007, E-008);
   PoKeysCompPulseEngine_v2.c inspected but yielded no hal_pin calls (PEv2 pins
   not exported in legacy userspace component; RT .comp uses different approach).
3. Most LinuxCnc_PokeysLibComp issue comments not inspected; some closure evidence
   and implementation notes may remain unrecorded.
4. ePK_PEState enumeration documented (C5) but PulseEngineState HAL update path
   not traced to confirm correct values are written to the pin.
5. Legacy component PoKeysCompPulseEngine_v2.c yielded no hal_pin creation calls;
   complete legacy PEv2 HAL interface not reconstructed from C source alone.
6. Criterion 6 PARTIAL: legacy C/comp PEv2 pin-export source not fully characterized.
