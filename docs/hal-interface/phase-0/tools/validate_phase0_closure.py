#!/usr/bin/env python3
"""
Phase 0 Closure Validator — docs/hal-interface/phase-0/tools/validate_phase0_closure.py

Documentation-only validator. Not a compatibility test.
Exits 0 iff all Phase 0 acceptance criteria are met.
Exits 1 with a list of failures otherwise.
"""
import sys, os, re, subprocess, hashlib, yaml

PHASE0_DIR = os.path.join(os.path.dirname(__file__), '..')
REPO_ROOT   = os.path.join(PHASE0_DIR, '..', '..', '..')
BASE_COMMIT = '4f0ab5e'   # starting commit; immutable files pinned here

errors = []

def err(msg):
    errors.append(msg)

# ── Load all artifacts ──────────────────────────────────────────────────────
def load_yaml(fname):
    path = os.path.join(PHASE0_DIR, fname)
    try:
        return yaml.safe_load(open(path, encoding='utf-8').read())
    except Exception as e:
        err(f"YAML parse failed: {fname}: {e}")
        return {}

src_data  = load_yaml("source-register.yaml")
req_data  = load_yaml("requirement-catalogue.yaml")
par_data  = load_yaml("legacy-pev2-parity.yaml")
lnk_data  = load_yaml("integration-links.yaml")

sources = src_data.get("sources", [])
reqs    = req_data.get("requirements", [])
parity  = par_data.get("parity_rows", [])
ilinks  = lnk_data.get("integration_links", [])
src_ids = {s["source_id"] for s in sources}
req_ids = [r["interface_id"] for r in reqs]
conf_register = open(os.path.join(PHASE0_DIR, "conflict-register.md"), encoding='utf-8').read()
od_txt   = open(os.path.join(PHASE0_DIR, "open-decisions.md"), encoding='utf-8').read()
tr_txt   = open(os.path.join(PHASE0_DIR, "traceability.md"), encoding='utf-8').read()
inv_txt  = open(os.path.join(PHASE0_DIR, "issue-inventory.md"), encoding='utf-8').read()
rpt_txt  = open(os.path.join(PHASE0_DIR, "phase-0-completion-report.md"), encoding='utf-8').read()
rm_txt   = open(os.path.join(PHASE0_DIR, "README.md"), encoding='utf-8').read()
lm_txt   = open(os.path.join(PHASE0_DIR, "lifecycle-ownership-matrix.md"), encoding='utf-8').read()

# ── YAML uniqueness ─────────────────────────────────────────────────────────
sids = [s["source_id"] for s in sources]
if len(sids) != len(set(sids)):
    err(f"Duplicate source IDs: {[x for x in sids if sids.count(x)>1]}")
if len(req_ids) != len(set(req_ids)):
    err(f"Duplicate requirement IDs: {[x for x in req_ids if req_ids.count(x)>1]}")

# ── All source references resolve ───────────────────────────────────────────
for r in reqs:
    for s in r.get("requirement_sources", []) + r.get("implementation_sources", []):
        if s not in src_ids:
            err(f"Broken source ref {s!r} in {r['interface_id']}")

registered_conflicts = set(re.findall(r'^Conflict ID: (CONFLICT-\d+)', conf_register, re.M))
for r in reqs:
    for c in r.get("conflicts", []):
        if c not in registered_conflicts:
            err(f"Unregistered conflict {c!r} in {r['interface_id']}")

# ── Parity counts ───────────────────────────────────────────────────────────
n_act = sum(1 for r in parity if r['active_or_commented'] == 'active')
n_com = sum(1 for r in parity if r['active_or_commented'] == 'commented')
if len(parity) != 163: err(f"Parity count {len(parity)} != 163")
if n_act != 162:        err(f"Active parity count {n_act} != 162")
if n_com != 1:          err(f"Commented parity count {n_com} != 1")

# ── Parity validator ─────────────────────────────────────────────────────────
extractor = os.path.join(PHASE0_DIR, "tools", "extract_legacy_pev2_exports.py")
parity_file = os.path.join(PHASE0_DIR, "legacy-pev2-parity.yaml")
r = subprocess.run([sys.executable, extractor, "--check-parity", parity_file],
                   capture_output=True, text=True)
if r.returncode != 0:
    err(f"Parity validator failed: {r.stdout.strip()} {r.stderr.strip()}")

# ── Immutable files unchanged ────────────────────────────────────────────────
for immutable in ["legacy-pev2-parity.yaml", "tools/extract_legacy_pev2_exports.py"]:
    fpath = os.path.join(PHASE0_DIR, immutable)
    r = subprocess.run(
        ['git', 'diff', '--exit-code', BASE_COMMIT, '--', fpath],
        capture_output=True, cwd=REPO_ROOT
    )
    if r.returncode != 0:
        err(f"Immutable file changed: {immutable}")

# ── HOMECOMP requirement records ────────────────────────────────────────────
for r in reqs:
    if r['interface_id'].startswith('HOMECOMP-'):
        owner = r.get('owner', '')
        if 'pokeys_homecomp' not in owner:
            err(f"{r['interface_id']} owner not pokeys_homecomp: {owner!r}")
        for s in r.get('implementation_sources', []):
            if s.startswith('F-'):
                err(f"{r['interface_id']} has pokeysHal impl source {s!r}")
for iid in ('HOMECOMP-005', 'HOMECOMP-006', 'HOMECOMP-007'):
    r = next((x for x in reqs if x['interface_id'] == iid), None)
    if not r: err(f"{iid} missing"); continue
    if 'F-005' in r.get('implementation_sources', []): err(f"{iid} has F-005")
    if 'E-010' not in r.get('implementation_sources', []): err(f"{iid} missing E-010")

# ── HOMECOMP-007 conflicts ───────────────────────────────────────────────────
hc007 = next((r for r in reqs if r['interface_id'] == 'HOMECOMP-007'), None)
if hc007:
    cl = hc007.get('conflicts', [])
    if 'CONFLICT-013' not in cl: err("HOMECOMP-007 missing CONFLICT-013")
    if 'CONFLICT-014' not in cl: err("HOMECOMP-007 missing CONFLICT-014")

# ── LA-012 and LA-013 parity facts ──────────────────────────────────────────
la012 = next((r for r in parity if r['legacy_id'] == 'LA-012'), None)
la013 = next((r for r in parity if r['legacy_id'] == 'LA-013'), None)
if la012:
    if la012['hal_type'] != 'hal_u32_t':         err(f"LA-012 type={la012['hal_type']}")
    if la012['direction_or_access'] != 'HAL_OUT': err(f"LA-012 dir={la012['direction_or_access']}")
    if la012['function_name'] != 'hal_pin_u32_newf': err(f"LA-012 fn={la012['function_name']}")
else: err("LA-012 missing")
if la013:
    if la013['hal_type'] != 'hal_u32_t':        err(f"LA-013 type={la013['hal_type']}")
    if la013['direction_or_access'] != 'HAL_IN': err(f"LA-013 dir={la013['direction_or_access']}")
    if la013['function_name'] != 'hal_pin_u32_newf': err(f"LA-013 fn={la013['function_name']}")
    if 'CONFLICT-013' not in str(la013.get('conflicts', [])): err("LA-013 missing CONFLICT-013")
else: err("LA-013 missing")

# ── HOMECOMP-006 and -007 catalogue facts ────────────────────────────────────
for iid, exp_dir, exp_type in [('HOMECOMP-006', 'HAL_IN', 'hal_u32_t'),
                                ('HOMECOMP-007', 'HAL_OUT', 'hal_u32_t')]:
    r = next((x for x in reqs if x['interface_id'] == iid), None)
    if r:
        if r['direction_or_access'] != exp_dir: err(f"{iid} direction={r['direction_or_access']} expected {exp_dir}")
        if r['hal_type'] != exp_type:           err(f"{iid} type={r['hal_type']} expected {exp_type}")
    else: err(f"{iid} missing from catalogue")

# ── Integration links ────────────────────────────────────────────────────────
for lk in ilinks:
    leg = lk.get('legacy_pokeys_endpoint_evidence', '')
    hc  = lk.get('homecomp_endpoint_evidence', '')
    if 'LA-' in str(leg) and 'E-010' in str(leg):
        err(f"{lk['link_id']} legacy_ev wrongly attributes LA-* to E-010")
    if 'LA-' in str(hc):
        err(f"{lk['link_id']} homecomp_ev contains LA-*: {hc!r}")

ik003 = next((l for l in ilinks if l['link_id'] == 'IK-003'), None)
if ik003:
    if ik003.get('compatibility_status') == 'compatible': err("IK-003 wrongly compatible")
    if 'CONFLICT-013' not in str(ik003.get('conflicts', '')): err("IK-003 no CONFLICT-013")
    if 'CONFLICT-014' not in str(ik003.get('conflicts', '')): err("IK-003 no CONFLICT-014")
    leg = str(ik003.get('legacy_pokeys_endpoint_evidence', ''))
    if 'LA-013' not in leg: err("IK-003 legacy_ev missing LA-013")
    if 'consumer' not in leg.lower(): err("IK-003 legacy endpoint not consumer")
    hc  = str(ik003.get('homecomp_endpoint_evidence', ''))
    if 'HOMECOMP-007' not in hc: err("IK-003 homecomp_ev missing HOMECOMP-007")
    if 'producer' not in hc.lower(): err("IK-003 homecomp endpoint not producer")
else: err("IK-003 missing")

ik004 = next((l for l in ilinks if l['link_id'] == 'IK-004'), None)
if ik004:
    leg = str(ik004.get('legacy_pokeys_endpoint_evidence', ''))
    if 'LA-012' not in leg: err("IK-004 legacy_ev missing LA-012")
    if 'producer' not in leg.lower(): err("IK-004 legacy endpoint not producer")
    hc  = str(ik004.get('homecomp_endpoint_evidence', ''))
    if 'HOMECOMP-006' not in hc: err("IK-004 homecomp_ev missing HOMECOMP-006")
    if 'consumer' not in hc.lower(): err("IK-004 homecomp endpoint not consumer")
else: err("IK-004 missing")

# ── Lifecycle matrix ──────────────────────────────────────────────────────────
if "zeroed by hal_malloc" in lm_txt: err("lifecycle matrix: 'zeroed by hal_malloc'")
sA_end = lm_txt.find('\n## B.')
sA = lm_txt[:sA_end] if sA_end > 0 else lm_txt
if '| **pokeys_homecomp' in sA: err("lifecycle matrix: homecomp row in Section A table")
if '## B. External Counterpart' not in lm_txt: err("lifecycle matrix: no Section B")
if '## C. Integration Lifecycle' not in lm_txt: err("lifecycle matrix: no Section C")

# ── CONFLICT-012 in conflict-register ────────────────────────────────────────
c12s = conf_register.find("Conflict ID: CONFLICT-012")
c12e = conf_register.find("Conflict ID: CONFLICT-013")
c12b = conf_register[c12s:c12e] if c12s >= 0 and c12e >= 0 else ""
for phrase in ["unreachable", "actual initial value", "set_joint_homing_params"]:
    if phrase.lower() not in c12b.lower():
        err(f"CONFLICT-012 missing: {phrase!r}")

# ── Stale phrases ────────────────────────────────────────────────────────────
stale_phrases = [
    "overwritten before first use", "reassigned before first use",
    "runtime severity: LOW", "LOW (volatile_home",
    "Reviewer Upstream", "if the upstream claim is verified",
    "56 patterns", "140 rows", "zeroed by hal_malloc",
    "Both YAML files validated",
]
all_docs = {
    "conflict-register.md": conf_register,
    "open-decisions.md": od_txt,
    "lifecycle-ownership-matrix.md": lm_txt,
    "phase-0-completion-report.md": rpt_txt,
    "README.md": rm_txt,
    "traceability.md": tr_txt,
}
for fn, txt in all_docs.items():
    for ph in stale_phrases:
        if ph.lower() in txt.lower():
            err(f"Stale phrase {ph!r} in {fn}")

# ── Traceability boundary and direction facts ─────────────────────────────────
if 'Homecomp Counterpart ABI Boundary' not in tr_txt:
    err("traceability: missing boundary section")
for banned_re, msg in [
    (r'hal_pin_s32_newf', "s32 type for AxesState/AxesCommand"),
    (r'hm2_\[HOSTMOT2\]', "hm2_[HOSTMOT2] name in traceability"),
]:
    if re.search(banned_re, tr_txt, re.I): err(f"traceability: {msg}")

# ── Issue inventory ────────────────────────────────────────────────────────────
lc_section = inv_txt[inv_txt.find('## LinuxCnc'):]
if re.search(r'\| Issue #24 \|[^|]+\| C-012 \|', lc_section): err("LC #24 has C-012")
if re.search(r'\| Issue #129 \|[^|]+\| D-003 \|', lc_section): err("LC #129 has D-003")

# All HIGH/MEDIUM retained issues have complete body/comment status
inv_lines = inv_txt.split('\n')
in_pk = in_lc = False
for l in inv_lines:
    if '## pokeysHal' in l: in_pk = True; in_lc = False
    elif '## LinuxCnc' in l: in_lc = True; in_pk = False
    if (l.startswith('| Issue') or l.startswith('| PR ')) and '---|' not in l:
        p = [x.strip() for x in l.split('|')][1:-1]
        if len(p) >= 11:
            obj, rel, body, cmt_cnt, cmt_s, ret = p[0], p[4], p[5], p[6], p[7], p[10]
            if ret in ('YES', 'MAYBE') and rel in ('HIGH', 'MEDIUM'):
                if body not in ('complete', 'empty', 'title-only'):
                    err(f"Incomplete body for {obj}: {body!r}")
                if cmt_s not in ('complete', 'none'):
                    err(f"Incomplete comments for {obj}: cmt_cnt={cmt_cnt} cmt_s={cmt_s!r}")
            # Source ID cross-repo check
            if len(p) >= 4:
                sid = p[3]
                if sid not in ('None', '') and sid in {s['source_id'] for s in sources}:
                    src = next(s for s in sources if s['source_id'] == sid)
                    src_repo = src.get('repository', src.get('url', ''))
                    is_lc = in_lc and not in_pk
                    if is_lc and 'pokeysHal' in str(src_repo) and 'LinuxCnc' not in str(src_repo):
                        err(f"Cross-repo source {sid!r} from pokeysHal in LC section for {obj}")

# Table column counts
for i, l in enumerate(inv_lines, 1):
    if (l.startswith('| Issue') or l.startswith('| PR ')) and '---|' not in l:
        if l.count('|') != 12:
            err(f"inv L{i}: {l.count('|')} pipes (expected 12)")

# ── All 14 criteria PASS ─────────────────────────────────────────────────────
for cnum in range(1, 15):
    m = re.search(rf'\| {cnum}\. .*?\| (PASS|PARTIAL|FAIL) \|', rpt_txt)
    if m:
        if m.group(1) != 'PASS': err(f"Criterion {cnum} is {m.group(1)}, expected PASS")
    else: err(f"Criterion {cnum} not found in report")

# ── Missing Evidence is None ─────────────────────────────────────────────────
me_m = re.search(r'## Missing Evidence.*?(?=\n##|\Z)', rpt_txt, re.DOTALL)
if me_m:
    me_body = me_m.group(0)
    if 'PARTIAL' in me_body or 'FAIL' in me_body:
        err("Missing Evidence section references PARTIAL/FAIL criteria")
    if 'None' not in me_body:
        err("Missing Evidence section does not say None")
else: err("Missing Evidence section not found")

# ── Final status ──────────────────────────────────────────────────────────────
if 'PHASE 0 BASELINE COMPLETE' not in rpt_txt:
    err("Report final status is not PHASE 0 BASELINE COMPLETE")
if 'PHASE 0 BASELINE INCOMPLETE' in rpt_txt.split('PHASE 0 BASELINE COMPLETE')[0]:
    err("Report still contains PHASE 0 BASELINE INCOMPLETE before final status")

# ── README completion status ──────────────────────────────────────────────────
if 'PHASE 0 BASELINE COMPLETE' not in rm_txt and 'COMPLETE' not in rm_txt:
    err("README completion status does not reflect COMPLETE")

# ── Only Phase 0 docs changed ────────────────────────────────────────────────
changed = subprocess.run(
    ['git', 'diff', '--name-only', BASE_COMMIT],
    capture_output=True, text=True, cwd=REPO_ROOT
).stdout.strip().split('\n')
for f in changed:
    if f and not f.startswith('docs/hal-interface/phase-0/'):
        err(f"Unexpected file changed outside Phase 0 docs: {f}")

# ── git diff --check ─────────────────────────────────────────────────────────
gc = subprocess.run(['git', 'diff', '--check'], capture_output=True, text=True, cwd=REPO_ROOT)
if gc.returncode != 0:
    err(f"git diff --check: {gc.stdout[:200]}")

# ── Report statistics agree with artifacts ───────────────────────────────────
live_src = len(sources)
live_req = len(reqs)
live_conf = len(re.findall(r'^Conflict ID:', conf_register, re.M))
live_dec  = len(re.findall(r'^### DEC-', od_txt, re.M))
live_chains = len(re.findall(r'^## \d+\.', tr_txt, re.M))

for label, live, pattern in [
    ('Source register entries', live_src, r'\| Source register entries \| (\d+) \|'),
    ('Requirement catalogue entries', live_req, r'\| Requirement catalogue entries \| (\d+) \|'),
    ('Conflicts registered', live_conf, r'\| Conflicts registered \| (\d+) \|'),
    ('Open decisions required', live_dec, r'\| Open decisions required \| (\d+) \|'),
]:
    m = re.search(pattern, rpt_txt)
    if m:
        rep_val = int(m.group(1))
        if rep_val != live:
            err(f"Report {label}: {rep_val} != artifact count {live}")

# ── Summary ───────────────────────────────────────────────────────────────────
n_pass = 14 - len([e for e in errors if 'Criterion' in e and 'is PASS' not in e])
print(f"Checks run. Sources:{live_src} Reqs:{live_req} Par:{len(parity)} Conf:{live_conf} Dec:{live_dec}")

if errors:
    print(f"\nFAILURES ({len(errors)}):")
    for e in errors:
        print(f"  {e}")
    sys.exit(1)
else:
    print("\nALL PHASE 0 CLOSURE CHECKS PASSED")
    print("PHASE 0 BASELINE COMPLETE")
    sys.exit(0)
