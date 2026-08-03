# Phase 0 Completion Report — HAL-Interface Baseline

Baseline date: 2026-08-03.
Repository: zarfld/pokeysHal@cd1f0dc8a0f64f92dc6bdce21bddcb36d33a14cd.

---

## Acceptance Table

| Criterion | Status | Evidence | Residual gap |
|---|---|---|---|
| 1. Both repositories inspected | PASS | pokeysHal inspected locally; LinuxCnc_PokeysLibComp accessed via GitHub API | LinuxCnc_PokeysLibComp not cloned; some files not inspected |
| 2. Open and closed issues searched | PARTIAL | pokeysHal: all issues listed; selected issues inspected (#24, #32-#39, #116-#133). LinuxCnc_PokeysLibComp: #310, #326 inspected; others in issue-inventory.md listed but bodies not read | ~15 LinuxCnc_PokeysLibComp issues not body-inspected |
| 3. Issue comments inspected where relevant | PARTIAL | Issue bodies inspected for selected issues in both repos. No issue comments were inspected in either repo. | Comments for #33, #35, #36, #38 may contain implementation evidence |
| 4. Official LinuxCNC rules recorded | PARTIAL | HAL_NAME_LEN=47 confirmed (/usr/include/linuxcnc/hal.h; source A-001). Upstream HAL_NAME_LEN=55 at commit 71bf88009d64fa15edbebf9250b65ee4454f9a05 (source A-001b; UNVERIFIED-BY-FETCH). CDI rules recorded via hal-canon README/hal_canon.h; canonical-devices.html not fetched directly. | canonical-devices.html and upstream hal.h not independently fetched; encoder canonical status asserted without primary source |
| 5. Exact hal-canon provenance recorded | PARTIAL | Tree SHA deed4c10535530ce0383fb357ea8427896226c70 recorded. Upstream commit from linuxcnc-hal-canon.git not determined. See CONFLICT-008. | Upstream SHA unknown |
| 6. Legacy HAL interface extracted from source | PASS | Integration HAL files read (DM542 HAL, pokeys_homing.hal). Python digital_io.py read. Pin names catalogued. | Not all legacy component C source inspected |
| 7. Current HAL interface extracted from source | PASS | All PoKeysLib*Async.c files searched. hal_export_adcin and hal_export_adcout calls confirmed by full read of PoKeysLibIOAsync.c. hal-canon/hal_digital.c and hal_analog.c read in full; direction mismatches documented. | No automated enumeration tool |
| 8. Lifecycle and ownership documented | PASS | lifecycle-ownership-matrix.md covers all subsystems. Allocation, creation, update, and cleanup paths identified with file:line evidence. | Cleanup paths not fully verified for all subsystems |
| 9. Enumerations and bitmaps documented | PASS | ePK_PinCap, ePK_PEAxisState, ePK_PEv2_AxisConfig, ePK_PulseEngineV2_AxisSwitchOptions documented in requirement-catalogue.yaml and conflict-register.md | ePK_PEState (for PulseEngineState) not fully documented |
| 10. Canonical and project-specific extensions distinguished | PARTIAL | canonical-vs-legacy-matrix.md classifies ~72 HAL objects. Direction-mismatch category added. Encoder correctly reclassified as hal-canon convention, not official CDI canonical. | Upstream HAL_NAME_LEN (55) not independently verified with commit SHA |
| 11. Contradictions recorded | PASS | 9 conflicts documented. CONFLICT-009 added: hal-canon direction mismatches (digin.in as HAL_IN, digout.out as HAL_OUT, adcin.value as HAL_IN). CONFLICT-003 and CONFLICT-004 corrected: analog canonical helpers ARE called; residual conflicts are name mismatch and functional-conversion gap. | Additional conflicts may exist in uninspected issues |
| 12. No production code changed | PASS | git status --short shows only one untracked file: the .prompt.md file (user-created). No source, header, or submodule modified. | n/a |
| 13. No compatibility tests designed | PASS | No test files created. No test specifications written. | n/a |
| 14. No unresolved claim presented as fact | PASS | Requirements, architecture decisions, legacy behavior, current behavior, inferences, and unresolved interpretations are distinguished throughout all documents. | Minor inferences present where primary source not available |

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
- LinuxCNC canonical-devices.html fetch (not run — network access not attempted)
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
2. `source-register.yaml` — 25 source entries, classes A–G
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
| Source register entries | 25 |
| Requirement catalogue entries | 35+ |
| Conflicts registered | 9 |
| Open decisions required | 19 |
| Traceability chains | 11 |
| Issues inventoried (pokeysHal) | 31 |
| Issues inventoried (LinuxCnc_PokeysLibComp) | 17 |

---

## Most Consequential Unresolved Decisions

1. **DEC-HALCANON-001** — hal-canon direction bugs (digin.in as HAL_IN, digout.out
   as HAL_OUT, adcin.value as HAL_IN) block all functional tests for I/O and analog
   input. Must be fixed first in Phase 1. (CONFLICT-009)

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

1. hal-canon upstream commit SHA (from `linuxcnc-hal-canon.git`) not recorded;
   current embedding is a tree, not a submodule commit pointer (CONFLICT-008).
2. LinuxCNC upstream/master `HAL_NAME_LEN = 55` at commit
   `71bf88009d64fa15edbebf9250b65ee4454f9a05` (source A-001b): SHA provided by reviewer
   but not independently fetched during Phase 0 — recorded as UNVERIFIED-BY-FETCH.
3. Issue comments not inspected for #33, #35, #36, #38, #118, #128 in pokeysHal;
   potential implementation evidence missed.
4. pokeysHal issue bodies not inspected: #37, #39, #119–#126.
5. LinuxCnc_PokeysLibComp issue body for #264 was empty via API; body may be
   available with different access.
6. No primary LinuxCNC source fetched to confirm encoder is absent from the
   official canonical-devices.html (assertion is based on reviewer instruction,
   not independent document fetch).
7. adcout functional conversion path (adcout.J.value → PWM duty cycle) not
   verified by any test or code trace (issues #37, #39 remain OPEN).
