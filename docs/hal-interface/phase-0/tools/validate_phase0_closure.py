#!/usr/bin/env python3
"""
Phase 0 Closure Validator
docs/hal-interface/phase-0/tools/validate_phase0_closure.py

Independently derives all 14 acceptance criteria from parsed artifacts.
Fails on any cross-document contradiction or missing evidence.
"""
import sys, os, re, subprocess, yaml

PHASE0_DIR      = os.path.normpath(os.path.join(os.path.dirname(__file__), '..'))
REPO_ROOT       = os.path.normpath(os.path.join(PHASE0_DIR, '..', '..', '..'))
IMMUTABLE_COMMIT = '4f0ab5e'
PRODUCTION_BASE  = 'cd1f0dc8a0f64f92dc6bdce21bddcb36d33a14cd'

errors  = []
warnings = []

def err(msg):  errors.append(msg)
def warn(msg): warnings.append(msg)

def load_yaml(fname):
    path = os.path.join(PHASE0_DIR, fname)
    try:
        return yaml.safe_load(open(path, encoding='utf-8').read())
    except Exception as e:
        err(f"YAML parse failed: {fname}: {e}")
        return {}

src_data = load_yaml("source-register.yaml")
req_data = load_yaml("requirement-catalogue.yaml")
par_data = load_yaml("legacy-pev2-parity.yaml")
lnk_data = load_yaml("integration-links.yaml")

sources  = src_data.get("sources", []) or []
reqs     = req_data.get("requirements", []) or []
parity   = par_data.get("parity_rows", []) or []
ilinks   = lnk_data.get("integration_links", []) or []
src_map  = {s["source_id"]: s for s in sources}
req_map  = {r["interface_id"]: r for r in reqs}

# Guard: non-empty
if len(sources) == 0:  err("source-register.yaml has zero entries")
if len(reqs) == 0:     err("requirement-catalogue.yaml has zero entries")
if len(parity) == 0:   err("legacy-pev2-parity.yaml has zero rows")
if len(ilinks) == 0:   err("integration-links.yaml has zero entries")

conf_txt = open(os.path.join(PHASE0_DIR, "conflict-register.md"), encoding='utf-8').read()
od_txt   = open(os.path.join(PHASE0_DIR, "open-decisions.md"), encoding='utf-8').read()
tr_txt   = open(os.path.join(PHASE0_DIR, "traceability.md"), encoding='utf-8').read()
inv_txt  = open(os.path.join(PHASE0_DIR, "issue-inventory.md"), encoding='utf-8').read()
rpt_txt  = open(os.path.join(PHASE0_DIR, "phase-0-completion-report.md"), encoding='utf-8').read()
rm_txt   = open(os.path.join(PHASE0_DIR, "README.md"), encoding='utf-8').read()
lm_txt   = open(os.path.join(PHASE0_DIR, "lifecycle-ownership-matrix.md"), encoding='utf-8').read()

# Stale parity count detector
def check_stale_parity_count(txt, label):
    for stale in ['140 rows','139 active','56 patterns']:
        if stale in txt: err(f"C6/C14: stale {stale!r} in {label}")

# ── Parse inventory ───────────────────────────────────────────────────────────
inv_lines = inv_txt.split('\n')
in_pk = in_lc = False
inv_rows = []
inv_tuples = set()
for l in inv_lines:
    if '## pokeysHal' in l:  in_pk=True;  in_lc=False
    elif '## LinuxCnc' in l: in_lc=True;  in_pk=False
    if (l.startswith('| Issue') or l.startswith('| PR ')) and '---|' not in l:
        p = [x.strip() for x in l.split('|')][1:-1]
        if len(p) >= 11:
            row = dict(zip(['obj','title','state','sid','rel','body','cmt_cnt','cmt_s','ev','conf','retain'], p[:11]))
            row['repo'] = 'pk' if in_pk else 'lc'
            inv_rows.append(row)
            tpl = (row['repo'], row['obj'])
            if tpl in inv_tuples: err(f"Duplicate inventory tuple: {tpl}")
            inv_tuples.add(tpl)

if len(inv_rows) == 0: err("issue-inventory.md: zero data rows parsed")

# ── C1: Repository and revision identified ───────────────────────────────────
has_pokeyshal = any('pokeysHal' in s.get('repository','') for s in sources)
has_lc_auth   = any('LinuxCnc_PokeysLibComp' in s.get('repository','') for s in sources)
has_linuxcnc  = any('LinuxCNC' in s.get('repository','') for s in sources
                    if s.get('authority_class','') == 'A')
if not has_pokeyshal:  err("C1: no pokeysHal source entries")
if not has_lc_auth:    err("C1: no LinuxCnc_PokeysLibComp source entries")
if not has_linuxcnc:   err("C1: no LinuxCNC A-class authority entries")
c1 = has_pokeyshal and has_lc_auth and has_linuxcnc

# ── C2: Issue bodies inspected ───────────────────────────────────────────────
c2_fails = []
for r in inv_rows:
    if r['retain'] in ('YES','MAYBE') and r['rel'] in ('HIGH','MEDIUM'):
        if r['body'] not in ('complete','empty','title-only'):
            c2_fails.append(f"{r['repo']}/{r['obj']} body={r['body']!r}")
for f in c2_fails: err(f"C2: {f}")
c2 = (len(c2_fails) == 0)

# ── C3: Comment inspection ───────────────────────────────────────────────────
c3_fails = []
for r in inv_rows:
    if r['retain'] in ('YES','MAYBE') and r['rel'] in ('HIGH','MEDIUM'):
        if r['cmt_cnt'] not in ('0','') and r['cmt_s'] not in ('complete','none'):
            c3_fails.append(f"{r['repo']}/{r['obj']} cmt_cnt={r['cmt_cnt']} cmt_s={r['cmt_s']!r}")
        # Source ID validation for issues with material evidence
        if r['sid'] not in (None, 'None', ''):
            if r['sid'] not in src_map:
                err(f"C3: {r['repo']}/{r['obj']} source {r['sid']!r} not in register")
            else:
                src = src_map[r['sid']]
                src_repo = src.get('repository', '')
                pio = str(src.get('path_or_issue', ''))
                # Cross-repo check
                if r['repo'] == 'pk' and 'LinuxCnc' in src_repo:
                    err(f"C3: cross-repo: {r['obj']} (pk) has LC source {r['sid']!r}")
                if r['repo'] == 'lc' and 'pokeysHal' in src_repo and 'LinuxCnc' not in src_repo:
                    err(f"C3: cross-repo: {r['obj']} (lc) has pk source {r['sid']!r}")
                # Issue number match
                issue_m = re.search(r'#(\d+)', r['obj'])
                if issue_m and issue_m.group(1) not in pio:
                    err(f"C3: {r['repo']}/{r['obj']} source {r['sid']!r} pio={pio!r} mismatch")

for f in c3_fails: err(f"C3: {f}")
# Explicit cross-repo check
lc_s = inv_txt[inv_txt.find('## LinuxCnc'):]
if re.search(r'\| Issue #24 \|[^|]+\| C-012 \|', lc_s): err("C3: LC #24 has C-012")
if re.search(r'\| Issue #129 \|[^|]+\| D-003 \|', lc_s): err("C3: LC #129 has D-003")
c3 = (len(c3_fails) == 0) and not any('C3:' in e for e in errors)

# ── C4: LinuxCNC authority ───────────────────────────────────────────────────
hal_h = next((s for s in sources if s.get('source_id')=='A-001'), None)
cdi   = next((s for s in sources if s.get('source_id')=='A-002'), None)
if not hal_h or not hal_h.get('inspected'): err("C4: A-001 (hal.h) missing or not inspected")
if not cdi or not cdi.get('inspected'):     err("C4: A-002 (CDI spec) missing or not inspected")
c4 = bool(hal_h and hal_h.get('inspected') and cdi and cdi.get('inspected'))

# ── C5: hal-canon provenance ─────────────────────────────────────────────────
b_ids = {s['source_id'] for s in sources if s.get('authority_class','')=='B'}
for bid in ['B-001','B-002','B-003']:
    if bid not in b_ids: err(f"C5: {bid} missing")
c5 = all(bid in b_ids for bid in ['B-001','B-002','B-003'])

# ── C6: Parity extraction ────────────────────────────────────────────────────
n_act = sum(1 for r in parity if r.get('active_or_commented')=='active')
n_com = sum(1 for r in parity if r.get('active_or_commented')=='commented')
if len(parity) != 163: err(f"C6: parity count {len(parity)} != 163")
if n_act != 162:        err(f"C6: active parity {n_act} != 162")
if n_com != 1:          err(f"C6: commented parity {n_com} != 1")
ext = os.path.join(PHASE0_DIR, "tools", "extract_legacy_pev2_exports.py")
pf  = os.path.join(PHASE0_DIR, "legacy-pev2-parity.yaml")
rv  = subprocess.run([sys.executable, ext, "--check-parity", pf], capture_output=True, text=True)
if rv.returncode != 0: err(f"C6: parity validator: {(rv.stdout+rv.stderr).strip()}")
check_stale_parity_count(open(os.path.join(PHASE0_DIR,"source-register.yaml")).read(), "source-register")
check_stale_parity_count(rpt_txt, "completion-report")
c6 = (len(parity)==163 and n_act==162 and n_com==1 and rv.returncode==0)

# ── C7: Current interface extracted ──────────────────────────────────────────
f_ids = {s['source_id'] for s in sources if s.get('authority_class','')=='F'}
for fid in ['F-001','F-002','F-003','F-005','F-008']:
    if fid not in f_ids: err(f"C7: {fid} missing")
# Check all requirement source refs resolve
for r in reqs:
    for sr in r.get('requirement_sources',[])+r.get('implementation_sources',[]):
        if sr not in src_map: err(f"C7: broken src ref {sr!r} in {r['interface_id']}")
# Check AxesCommand and index-enable have pokeysHal-owned entries
has_axescmd = any(
    r.get('interface_id','').startswith('PEV2A') and
    'AxesCommand' in r.get('name_pattern','') and
    r.get('owner','') != 'pokeys_homecomp'
    for r in reqs)
has_idxen = any(
    r.get('interface_id','').startswith('PEV2A') and
    'index-enable' in r.get('name_pattern','')
    for r in reqs)
if not has_axescmd: err("C7: no pokeysHal-owned PEV2A entry for AxesCommand")
if not has_idxen:   err("C7: no PEV2A entry for index-enable")
c7 = (all(fid in f_ids for fid in ['F-001','F-002','F-003','F-005','F-008'])
      and has_axescmd and has_idxen and not any('broken src ref' in e for e in errors))

# ── C8: Library ownership ────────────────────────────────────────────────────
has_sec_a = '## A. pokeysHal Component Lifecycle' in lm_txt or '## A. pokeysHal' in lm_txt
has_sec_b = '## B. External Counterpart' in lm_txt
has_sec_c = '## C. Integration Lifecycle' in lm_txt
# Section A must not contain homecomp lifecycle row
sA_end = lm_txt.find('\n## B.')
sA = lm_txt[:sA_end] if sA_end > 0 else lm_txt
if '| **pokeys_homecomp' in sA: err("C8: homecomp row in Section A")
if 'Device→HAL' not in lm_txt: err("C8: Device→HAL update ownership missing")
if 'HAL→device' not in lm_txt: err("C8: HAL→device update ownership missing")
# Homecomp entries must be external counterpart
for r in reqs:
    if r['interface_id'].startswith('HOMECOMP-'):
        if 'pokeys_homecomp' not in r.get('owner',''):
            err(f"C8: {r['interface_id']} owner not pokeys_homecomp")
        for sr in r.get('implementation_sources',[]):
            if sr.startswith('F-'): err(f"C8: {r['interface_id']} has F-class impl src")
c8 = (has_sec_a and has_sec_b and has_sec_c and
      '| **pokeys_homecomp' not in sA and
      'Device→HAL' in lm_txt and 'HAL→device' in lm_txt and
      not any('C8:' in e for e in errors))

# ── C9: Enumerations ─────────────────────────────────────────────────────────
has_pev2_state_enum = any('ePK_PEAxisState' in str(r.get('enumeration','')) for r in reqs)
has_axescmd_enum = any('AxesCommand' in r.get('name_pattern','') and r.get('enumeration') for r in reqs)
if not has_pev2_state_enum: err("C9: ePK_PEAxisState enum missing from catalogue")
if not has_axescmd_enum:    err("C9: AxesCommand enum missing from catalogue")
c9 = has_pev2_state_enum and has_axescmd_enum

# ── C10: Canonical classification ────────────────────────────────────────────
canon_matrix_path = os.path.join(PHASE0_DIR, "canonical-vs-legacy-matrix.md")
has_canon_matrix = os.path.exists(canon_matrix_path)
if not has_canon_matrix: err("C10: canonical-vs-legacy-matrix.md missing")
else:
    cm_txt = open(canon_matrix_path).read()
    has_canonical_col  = 'canonical' in cm_txt.lower()
    has_pokeys_spec    = 'pokeys-specific' in cm_txt.lower()
    has_adcout_class   = 'adcout' in cm_txt.lower()
    if not has_canonical_col: err("C10: no canonical classification in matrix")
    if not has_pokeys_spec:   err("C10: no PoKeys-specific classification in matrix")
    if not has_adcout_class:  err("C10: adcout not classified in matrix")
    # Check that ADCOUT status is consistent: no "conversion unverified" if IMPLEMENTED
    if 'conversion unverified' in cm_txt.lower():
        err("C10/C14: canonical matrix still says 'conversion unverified'")
c10 = (has_canon_matrix and has_canonical_col and has_pokeys_spec and has_adcout_class
       and not any('C10' in e for e in errors))

# ── C11: Contradictions registered ───────────────────────────────────────────
reg_conflicts = set(re.findall(r'^Conflict ID: (CONFLICT-\d+)', conf_txt, re.M))
# All conflict references in reqs/ilinks must resolve
for r in reqs:
    for c in r.get('conflicts',[]):
        if c not in reg_conflicts: err(f"C11: unregistered conflict {c!r} in {r['interface_id']}")
for lk in ilinks:
    for c in lk.get('conflicts', []):
        if c not in reg_conflicts: err(f"C11: unregistered conflict {c!r} in link {lk['link_id']}")
# No conflict about VOLATILE_HOME only
for cid in reg_conflicts:
    bs = conf_txt.find(f"Conflict ID: {cid}")
    ne = conf_txt.find("Conflict ID: CONFLICT-", bs+1)
    blk = conf_txt[bs:ne] if ne>0 else conf_txt[bs:]
    s_line = next((l for l in blk.split('\n') if 'Subject:' in l), '')
    if ('volatile_home' in s_line.lower() and 'AxesCommand' not in blk and 'digout' not in blk):
        err(f"C11: {cid} is solely about volatile_home (out of scope)")
# All docs must reference only registered conflict IDs
for doc_name, doc_txt in [
    ("traceability.md", tr_txt), ("lifecycle-matrix.md", lm_txt),
    ("req-catalogue.yaml", open(os.path.join(PHASE0_DIR,"requirement-catalogue.yaml")).read()),
]:
    for m in re.finditer(r'CONFLICT-\d+', doc_txt):
        cid = m.group(0)
        if cid not in reg_conflicts:
            err(f"C11: {doc_name} references unregistered {cid}")
c11 = not any('C11:' in e for e in errors)

# ── C12: No production files changed ─────────────────────────────────────────
changed = subprocess.run(['git','diff','--name-only',PRODUCTION_BASE],
                         capture_output=True, text=True, cwd=REPO_ROOT).stdout.strip()
for f in changed.split('\n'):
    if not f: continue
    if f.startswith('docs/hal-interface/phase-0/'): continue
    if '.github/prompts/' in f: continue
    err(f"C12: non-Phase-0 file changed: {f}")
gc = subprocess.run(['git','diff','--check'], capture_output=True, text=True, cwd=REPO_ROOT)
if gc.returncode != 0: err(f"C12: diff --check: {gc.stdout[:200]}")
# Immutable files
for immutable in ["legacy-pev2-parity.yaml","tools/extract_legacy_pev2_exports.py"]:
    rv2 = subprocess.run(['git','diff','--exit-code',IMMUTABLE_COMMIT,'--',
                          os.path.join(PHASE0_DIR,immutable)], capture_output=True, cwd=REPO_ROOT)
    if rv2.returncode != 0: err(f"C12: immutable changed: {immutable}")
c12 = not any('C12:' in e for e in errors)

# ── C13: No test artifacts ───────────────────────────────────────────────────
changed_files = changed.split('\n') if changed else []
test_added = any(
    re.search(r'test.*\.(py|c)$|hil_', f, re.I) and 'tools/validate' not in f
    for f in changed_files if f)
if test_added: err("C13: test artifacts added")
c13 = not test_added

# ── C14: Cross-document consistency ──────────────────────────────────────────
# 1. ADCOUT: no "conversion unverified" if code path is IMPLEMENTED
ADCOUT_UNVERIFIED_DOCS = [
    ("traceability.md", tr_txt),
    ("open-decisions.md", od_txt),
    ("requirement-catalogue.yaml", open(os.path.join(PHASE0_DIR,"requirement-catalogue.yaml")).read()),
]
for doc_name, doc_txt in ADCOUT_UNVERIFIED_DOCS:
    if 'conversion unverified' in doc_txt.lower():
        err(f"C14: {doc_name} says 'conversion unverified' (contradicts IMPLEMENTED in other docs)")
    if 'conversion path unverified' in doc_txt.lower():
        err(f"C14: {doc_name} says 'conversion path unverified'")
# 2. Stale conflict/decision counts
live_conf = len(reg_conflicts)
live_dec  = len(re.findall(r'^### DEC-', od_txt, re.M))
live_src  = len(sources)
live_req  = len(reqs)
live_ch   = len(re.findall(r'^## \d+\.', tr_txt, re.M))
COUNT_DOCS = [("report", rpt_txt), ("readme", rm_txt), ("source-reg", open(os.path.join(PHASE0_DIR,"source-register.yaml")).read())]
for doc_name, doc_txt in COUNT_DOCS:
    check_stale_parity_count(doc_txt, doc_name)
for label, expected, patterns in [
    ('Conflicts registered', live_conf, [r'\| Conflicts registered \| (\d+) \|']),
    ('Open decisions required', live_dec, [r'\| Open decisions required \| (\d+) \|']),
    ('Source register entries', live_src, [r'\| Source register entries \| (\d+) \|']),
    ('Requirement catalogue entries', live_req, [r'\| Requirement catalogue entries \| (\d+) \|']),
    ('Traceability chains', live_ch, [r'\| Traceability chains \| (\d+) \|']),
]:
    for pat in patterns:
        for m in re.finditer(pat, rpt_txt):
            if int(m.group(1)) != expected:
                err(f"C14: report '{label}' says {m.group(1)} but artifact has {expected}")
# 3. CONFLICT-012 and DEC-LIFE-002 must not appear as active items
for doc_name, doc_txt in [("report", rpt_txt), ("open-decisions", od_txt)]:
    if re.search(r'CONFLICT-012', doc_txt) and '## Suggested Next Steps' in rpt_txt:
        # Only fail if it appears in active criteria or backlog
        if re.search(r'(?:criterion|backlog|Phase 1).{0,200}CONFLICT-012', doc_txt, re.DOTALL):
            err(f"C14: CONFLICT-012 appears in active criteria/backlog in {doc_name}")
    if re.search(r'DEC-LIFE-002', doc_txt):
        # Check if it appears as active decision
        for i, l in enumerate(doc_txt.split('\n'), 1):
            if 'DEC-LIFE-002' in l and not re.match(r'\s*[-#*].*removed|excluded|out.of.scope', l, re.I):
                err(f"C14: DEC-LIFE-002 still referenced in {doc_name} L{i}: {l[:80]}")
# 4. Inventory row count must match report
pk_n  = sum(1 for r in inv_rows if r['repo']=='pk' and not r['obj'].startswith('PR'))
lc_n  = sum(1 for r in inv_rows if r['repo']=='lc' and not r['obj'].startswith('PR'))
pr_n  = sum(1 for r in inv_rows if r['obj'].startswith('PR'))
for label, lcount in [('Issues inventoried (pokeysHal)', pk_n),
                      ('Issues inventoried (LinuxCnc_PokeysLibComp)', lc_n),
                      ('Pull requests inventoried', pr_n)]:
    m = re.search(rf'\| {re.escape(label)} \| (\d+) \|', rpt_txt)
    if m and int(m.group(1)) != lcount:
        err(f"C14: report '{label}' says {m.group(1)} but inventory has {lcount}")
# 5. Scope boundary
BANNED = [
    ("pokeysHal is a HOMEMOD", "pokeysHal wrongly described as HOMEMOD"),
    ("pokeys_homecomp is a pokeysHal subsystem", "homecomp is a subsystem"),
    ("pokeysHal owns joint", "pokeysHal owns joint.N.*"),
]
for phrase, label in BANNED:
    for doc_name, doc_txt in [("lifecycle", lm_txt), ("traceability", tr_txt), ("report", rpt_txt)]:
        if phrase.lower() in doc_txt.lower():
            err(f"C14: {label} in {doc_name}")
# 6. Parity LA-013/LA-022 current_interface_id correctness
la013 = next((r for r in parity if r.get('legacy_id')=='LA-013'), None)
la022 = next((r for r in parity if r.get('legacy_id')=='LA-022'), None)
if la013:
    ciid = la013.get('current_interface_id')
    if ciid in (None, 'None', ''):
        warn(f"C14/immutable: LA-013 current_interface_id=None (parity immutable; gap documented)")
if la022:
    ciid = la022.get('current_interface_id')
    if ciid and ciid.startswith('HOMECOMP'):
        err(f"C14: LA-022 current_interface_id={ciid!r} maps to homecomp endpoint; "
            f"should map to PEV2A-007 (pokeysHal endpoint). Parity table immutable — "
            f"document in Missing Evidence until Phase 1 corrects the parity table.")
c14 = not any('C14:' in e for e in errors)

# ── Criterion statuses ────────────────────────────────────────────────────────
computed = {1:c1,2:c2,3:c3,4:c4,5:c5,6:c6,7:c7,8:c8,9:c9,10:c10,11:c11,12:c12,13:c13,14:c14}

for cnum, cpass in computed.items():
    m = re.search(rf'\| {cnum}\. .*?\| (PASS|PARTIAL|FAIL) \|', rpt_txt)
    if not m:
        err(f"Criterion {cnum} not found in report")
    else:
        rs = m.group(1)
        if cpass and rs not in ('PASS',):
            err(f"Criterion {cnum}: computed PASS but report says {rs}")
        elif not cpass and rs == 'PASS':
            err(f"Criterion {cnum}: computed NOT PASS but report says {rs}")

# Final status check
if all(computed.values()):
    if 'PHASE 0 BASELINE COMPLETE' not in rpt_txt:
        err("All criteria PASS but report does not say PHASE 0 BASELINE COMPLETE")
else:
    if 'PHASE 0 BASELINE INCOMPLETE' not in rpt_txt:
        err("Some criteria not PASS but report does not say PHASE 0 BASELINE INCOMPLETE")

# ── Summary ───────────────────────────────────────────────────────────────────
print(f"Sources:{live_src} Reqs:{live_req} Parity:{len(parity)} "
      f"Conf:{live_conf} Dec:{live_dec} Chains:{live_ch}")
print(f"Inventory rows: pk={pk_n} lc={lc_n} prs={pr_n}")
print()
print("Computed criterion statuses:")
for n in range(1,15):
    print(f"  Criterion {n:2d}: {'PASS' if computed.get(n) else 'FAIL'}")

if warnings:
    print(f"\nWARNINGS ({len(warnings)}):")
    for w in warnings: print(f"  {w}")

if errors:
    print(f"\nFAILURES ({len(errors)}):")
    for e in errors: print(f"  {e}")
    print("\nPHASE 0 BASELINE INCOMPLETE")
    sys.exit(1)
else:
    parity_note = rv.stdout.strip().split('\n')[-1]
    print(f"\n  Parity: {parity_note}")
    print("\nALL PHASE 0 CLOSURE CHECKS PASSED")
    print("PHASE 0 BASELINE COMPLETE")
    sys.exit(0)
