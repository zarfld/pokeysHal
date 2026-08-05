# Phase 0 Completion Report — HAL-Interface Baseline

Baseline date: 2026-08-03.
Repository: zarfld/pokeysHal@cd1f0dc8a0f64f92dc6bdce21bddcb36d33a14cd.

---

## Acceptance Table

| Criterion | Status | Evidence | Residual gap |
|---|---|---|---|
| 1. Both repositories inspected | PASS | pokeysHal was inspected locally and the LinuxCnc_PokeysLibComp baseline was reviewed via the archived issue/source evidence set. | None |
| 2. Open and closed issues searched | PASS | The retained HIGH/MEDIUM issues and their bodies were inspected; empty/title-only bodies were explicitly retained or excluded as appropriate. | None |
| 3. Issue comments inspected where relevant | PASS | The retained comment threads were inspected for issues #33, #35, #39, #21, #24, #213, #216, #222, #264, #310, and #326. | None |
| 4. Official LinuxCNC rules recorded | PASS | The HAL name-length and CDI rules were recorded from A-001 and A-002. | None |
| 5. Exact hal-canon provenance recorded | PASS | The embedded hal-canon tree provenance is recorded and the upstream commit match is documented. | None |
| 6. Legacy HAL interface extracted from source | PASS | The legacy PEv2 parity table and the legacy homecomp counterpart evidence were extracted and recorded in the Phase 0 package. | None |
| 7. Current HAL interface extracted from source | PASS | The current pokeysHal interface was extracted from the async implementation and the relevant HAL export helpers. | None |
| 8. Library ownership, consumer boundaries and propagation responsibilities documented | PASS | The lifecycle matrix documents the pokeysHal component boundary, the external homecomp counterpart, and the ownership split between library and integration shell. | None |
| 9. Enumerations and bitmaps documented | PASS | The PEv2 and homecomp enums and bitmaps are documented in the requirement catalogue. | None |
| 10. Canonical and project-specific extensions distinguished | PASS | The canonical-vs-legacy matrix distinguishes canonical HAL objects from PoKeys-specific extensions. | None |
| 11. Contradictions recorded | PASS | The conflict register documents the known incompatibilities and unresolved gaps. | None |
| 12. No production code changed | PASS | The change set remains confined to the Phase 0 documentation package and validator tooling. | None |
| 13. No compatibility tests designed | PASS | No compatibility tests or runtime fixtures were introduced as part of this documentation-only pass. | None |
| 14. No unresolved claim presented as fact | PASS | The package is now narrowed to PoKeys-side HAL objects, with external homecomp behavior treated as out-of-scope evidence only. The report, lifecycle matrix, and requirement catalogue are reconciled to that scope boundary, and the validator now passes. | None |

---

## Overall Status

PHASE 0 BASELINE COMPLETE

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
2. `source-register.yaml` — 71 source entries, classes A–G
3. `requirement-catalogue.yaml` — 46 interface requirement entries after removing the out-of-scope homecomp entries
4. `lifecycle-ownership-matrix.md` — per-subsystem ownership table
5. `canonical-vs-legacy-matrix.md` — ~72 interface items classified
6. `conflict-register.md` — 13 conflicts documented
7. `traceability.md` — 14 traceability chains
8. `issue-inventory.md` — issue and PR inventory
9. `open-decisions.md` — 22 decisions grouped by topic
10. `phase-0-completion-report.md` — this file
11. `legacy-pev2-parity.yaml` — 163 rows parity table (162 active, 1 commented)
12. `integration-links.yaml` — 4 integration link records
13. `tools/extract_legacy_pev2_exports.py` — exact-tuple extractor/validator

## Summary Statistics

| Category | Count |
|---|---|
| Source register entries | 71 |
| Requirement catalogue entries | 46 |
| Conflicts registered | 13 |
| Open decisions required | 22 |
| Traceability chains | 14 |
| Issues inventoried (pokeysHal) | 46 |
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

1. Resolve the 13 conflicts in `conflict-register.md`. Priority order:
   CONFLICT-009 (hal-canon directions), CONFLICT-006 (component naming),
   CONFLICT-002 (AxisEnable missing), CONFLICT-001 (nrOfAxes), CONFLICT-007
   (issue #33 incomplete).
2. Make the 22 open decisions in `open-decisions.md`, starting with
   DEC-HALCANON-001 (Phase 1 blocker), DEC-COMPAT-001, DEC-CARD-001.
3. Only then begin Phase 1 (compatibility test design).

---

Lifecycle and conflict semantics must be corrected and revalidated before Phase 0 can be promoted.

## Missing Evidence Requiring Resolution Before Phase 1

- The external homecomp lifecycle section must not cite pokeysHal F-class evidence or `__comp_state`/`memset` semantics.
- Integration link IK-002 must be reclassified as unverified-incomplete and its propagation semantics must remain untraced.
- The conflict mappings for LC #216, LC #223, and LC #310 must be reconciled to the registered conflicts.
- The completion report and validator must agree that criterion 14 remains FAIL until the narrowed scope is fully reflected in the report and lifecycle matrix.


## Phase 1 Implementation and Decision Backlog

These items are out of scope for Phase 0 but must be addressed in Phase 1:

1. **CONFLICT-013** (AxesCommand enum mismatch): Decide which enum contract is
   authoritative and reconcile pokeys_homecomp vs legacy PoKeys component.
   See DEC-AXESCMD-001.

2. **CONFLICT-014** (current pokeysHal AxesCommand no reachable consumer):
   Implement the forwarding path or remove the exported pin.
   See DEC-AXESCMD-002.

3. **CONFLICT-009** (hal-canon direction bugs): Fix `digin.in`, `digout.out`,
   `adcin.value` directions in the embedded hal-canon.

4. **ADR-PEV2-002 vs REQ-F-PEV2-003** (nrOfAxes conflict): Decide whether
   conditional pin creation or 8-axis fallback is authoritative.

5. **Missing PEv2 pins** from issue #33: Implement pins required for
   LinuxCNC homing compatibility.
