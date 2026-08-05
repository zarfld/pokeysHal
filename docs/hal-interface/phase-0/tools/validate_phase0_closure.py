#!/usr/bin/env python3
"""
Phase 0 Closure Validator — comprehensive strengthened edition.
Independently derives all 14 acceptance criteria from parsed artifacts.
"""
import sys, os, re, subprocess, yaml

PHASE0_DIR       = os.path.normpath(os.path.join(os.path.dirname(__file__), '..'))
REPO_ROOT        = os.path.normpath(os.path.join(PHASE0_DIR, '..', '..', '..'))
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
        err(f"YAML: {fname}: {e}"); return {}

src_data = load_yaml("source-register.yaml")
req_data = load_yaml("requirement-catalogue.yaml")
par_data = load_yaml("legacy-pev2-parity.yaml")
lnk_data = load_yaml("integration-links.yaml")

sources = src_data.get("sources", []) or []
reqs    = req_data.get("requirements", []) or []
parity  = par_data.get("parity_rows", []) or []
ilinks  = lnk_data.get("integration_links", []) or []
src_map = {s["source_id"]: s for s in sources}
req_map = {r["interface_id"]: r for r in reqs}

if not sources: err("source-register has zero entries")
if not reqs:    err("requirement-catalogue has zero entries")
if not parity:  err("parity table has zero rows")

# Load all text artifacts
def txt(fname):
    return open(os.path.join(PHASE0_DIR, fname), encoding='utf-8').read()

conf_txt = txt("conflict-register.md")
od_txt   = txt("open-decisions.md")
tr_txt   = txt("traceability.md")
inv_txt  = txt("issue-inventory.md")
rpt_txt  = txt("phase-0-completion-report.md")
rm_txt   = txt("README.md")
lm_txt   = txt("lifecycle-ownership-matrix.md")
sr_raw   = open(os.path.join(PHASE0_DIR, "source-register.yaml"), encoding='utf-8').read()
rc_raw   = open(os.path.join(PHASE0_DIR, "requirement-catalogue.yaml"), encoding='utf-8').read()
cm_path  = os.path.join(PHASE0_DIR, "canonical-vs-legacy-matrix.md")
cm_txt   = open(cm_path).read() if os.path.exists(cm_path) else ""
inv_path = txt("issue-inventory.md")  # same as inv_txt

ALL_DOCS = {
    "report": rpt_txt, "README": rm_txt, "traceability": tr_txt,
    "open-decisions": od_txt, "lifecycle": lm_txt,
    "source-register": sr_raw, "req-catalogue": rc_raw,
    "conflict-register": conf_txt, "canonical-matrix": cm_txt,
}

reg_conflicts = set(re.findall(r'^Conflict ID: (CONFLICT-\d+)', conf_txt, re.M))
reg_decisions = set(re.findall(r'^### (DEC-[A-Z0-9_-]+)', od_txt, re.M))
n_conf = len(reg_conflicts); n_dec = len(reg_decisions)

# ── Parse inventory ─────────────────────────────────────────────────────────
inv_lines = inv_txt.split('\n')
in_pk = in_lc = False
inv_rows = []
inv_tuples = set()
for l in inv_lines:
    if '## pokeysHal' in l: in_pk=True; in_lc=False
    elif '## LinuxCnc' in l: in_lc=True; in_pk=False
    if (l.startswith('| Issue') or l.startswith('| PR ')) and '---|' not in l:
        p = [x.strip() for x in l.split('|')][1:-1]
        if len(p) >= 11:
            row = dict(zip(['obj','title','state','sid','rel','body','cmt_cnt','cmt_s','ev','conf','retain'], p[:11]))
            row['repo'] = 'pk' if in_pk else 'lc'
            tpl = (row['repo'], row['obj'])
            if tpl in inv_tuples: err(f"Duplicate inv tuple: {tpl}")
            inv_tuples.add(tpl); inv_rows.append(row)

if not inv_rows: err("issue-inventory: zero rows parsed")

pk_n  = sum(1 for r in inv_rows if r['repo']=='pk' and not r['obj'].startswith('PR'))
lc_n  = sum(1 for r in inv_rows if r['repo']=='lc' and not r['obj'].startswith('PR'))
pr_n  = sum(1 for r in inv_rows if r['obj'].startswith('PR'))

# Derive HIGH/MEDIUM retained with >0 comments
hm_comment_threads = [r for r in inv_rows
                       if r['retain'] in ('YES','MAYBE') and r['rel'] in ('HIGH','MEDIUM')
                       and r['cmt_cnt'] not in ('0','') and r['cmt_s'] == 'complete']

# ── Stale-phrase scanners ───────────────────────────────────────────────────
STALE_PHRASES = ['140 rows','139 active','56 patterns','legitimately have Source ID None',
                 '2026-08-05','count=29 pins specified','29 pins specified']
REMOVED_IDS   = ['CONFLICT-012', 'DEC-LIFE-002']

def scan_stale(txt_str, label):
    for phrase in STALE_PHRASES:
        if phrase in txt_str:
            err(f"C14: stale phrase {phrase!r} in {label}")
    for rid in REMOVED_IDS:
        for m in re.finditer(re.escape(rid), txt_str):
            ctx = txt_str[max(0,m.start()-300):m.end()+200]
            if not any(x in ctx for x in ['External Observations','out-of-scope note','excluded from baseline']):
                err(f"C14: active reference to removed {rid!r} in {label}"); break

for label, doc in ALL_DOCS.items():
    scan_stale(doc, label)

# Scan all docs for conflict/decision refs
for label, doc in ALL_DOCS.items():
    for cid in re.findall(r'CONFLICT-\d+', doc):
        if cid not in reg_conflicts:
            err(f"C11/C14: {label} references unregistered {cid}")
    for did in re.findall(r'DEC-[A-Z0-9_-]+', doc):
        if did not in reg_decisions:
            err(f"C14: {label} references unregistered decision {did}")

# Prose count consistency
def check_prose_count(doc_name, doc_txt, keyword, live_val):
    for m in re.finditer(rf'\b(\d+)\s+(?:open\s+)?{re.escape(keyword)}', doc_txt, re.I):
        found = int(m.group(1))
        if found != live_val:
            err(f"C14: {doc_name} prose says {m.group(0)!r} but live={live_val}")

for label, doc in [("report", rpt_txt), ("README", rm_txt)]:
    check_prose_count(label, doc, "conflict", n_conf)
    check_prose_count(label, doc, "decision", n_dec)

# ── C1: Repository and revision identified ──────────────────────────────────
has_pk  = any('pokeysHal' in s.get('repository','') for s in sources)
has_lc  = any('LinuxCnc_PokeysLibComp' in s.get('repository','') for s in sources)
has_a   = any(s.get('authority_class','')=='A' and s.get('inspected') for s in sources)
has_sha = 'cd1f0dc8a0f64f92dc6bdce21bddcb36d33a14cd' in sr_raw
has_lc_sha = '0c058e6c7136ddb28a6b9f463a8af3d973496246' in sr_raw
for check, msg in [(has_pk,"no pokeysHal entries"),(has_lc,"no LC entries"),
                    (has_a,"no A-class entries"),(has_sha,"main SHA missing"),
                    (has_lc_sha,"LC SHA missing")]:
    if not check: err(f"C1: {msg}")
c1 = has_pk and has_lc and has_a and has_sha and has_lc_sha

# ── C2: Issue body inspection ───────────────────────────────────────────────
c2_ok = True
for r in inv_rows:
    if r['retain'] in ('YES','MAYBE') and r['rel'] in ('HIGH','MEDIUM'):
        if r['body'] not in ('complete','empty','title-only'):
            err(f"C2: {r['repo']}/{r['obj']} body={r['body']!r}"); c2_ok=False

# Reconcile README vs inventory
for r in inv_rows:
    issue_m = re.search(r'#(\d+)', r['obj'])
    if issue_m and r['retain'] in ('YES','MAYBE') and r['rel'] in ('HIGH','MEDIUM') and r['body']=='complete':
        inum = issue_m.group(1)
        if re.search(rf'#{inum}[^0-9].*not.body.inspected', rm_txt, re.I):
            err(f"C2: README says #{inum} not-body-inspected but inventory=complete"); c2_ok=False

c2 = c2_ok

# ── C3: Comment inspection and source coverage ──────────────────────────────
c3_ok = True
for r in inv_rows:
    if r['retain'] in ('YES','MAYBE') and r['rel'] in ('HIGH','MEDIUM'):
        cnt = r['cmt_cnt']; cs = r['cmt_s']; sid = r['sid']
        # Exact comment_status consistency
        if cnt == '0' and cs not in ('none',''):
            err(f"C3: {r['repo']}/{r['obj']} cmt_cnt=0 cmt_s={cs!r}"); c3_ok=False
        if cnt not in ('0','') and cs not in ('complete','partial'):
            err(f"C3: {r['repo']}/{r['obj']} cmt_cnt={cnt} cmt_s={cs!r}"); c3_ok=False
        # Source ID required
        if sid in (None,'None',''):
            err(f"C3: {r['repo']}/{r['obj']} (retained {r['rel']}) has no source ID"); c3_ok=False
        elif sid not in src_map:
            err(f"C3: {r['repo']}/{r['obj']} source {sid!r} missing"); c3_ok=False
        else:
            src = src_map[sid]
            src_repo = src.get('repository','')
            pio = str(src.get('path_or_issue',''))
            if r['repo']=='pk' and 'LinuxCnc' in src_repo:
                err(f"C3: cross-repo {r['obj']}"); c3_ok=False
            if r['repo']=='lc' and 'pokeysHal' in src_repo and 'LinuxCnc' not in src_repo:
                err(f"C3: cross-repo {r['obj']}"); c3_ok=False
            issue_m = re.search(r'#(\d+)', r['obj'])
            if issue_m and issue_m.group(1) not in pio:
                err(f"C3: {r['repo']}/{r['obj']} source {sid!r} pio={pio!r}"); c3_ok=False
            if not src.get('inspected'):
                err(f"C3: source {sid!r} not inspected"); c3_ok=False
            # For >0-comment issues: source notes must mention the numeric count
            if cnt not in ('0','') and cs=='complete':
                src_text = str(src.get('notes',''))+str(src.get('scope',''))
                if not re.search(rf'\b{cnt}\b', src_text):
                    err(f"C3: {r['repo']}/{r['obj']} source {sid!r} notes missing count {cnt}")
                    c3_ok=False

# Cross-repo explicit checks
lc_sec = inv_txt[inv_txt.find('## LinuxCnc'):]
if re.search(r'\| Issue #24 \|[^|]+\| C-012 \|', lc_sec): err("C3: LC #24 has C-012"); c3_ok=False
if re.search(r'\| Issue #129 \|[^|]+\| D-003 \|', lc_sec): err("C3: LC #129 has D-003"); c3_ok=False

# Traceability vs inventory consistency
for r in inv_rows:
    if r['retain'] in ('YES','MAYBE') and r['rel'] in ('HIGH','MEDIUM') and r['body']=='complete':
        issue_m = re.search(r'#(\d+)', r['obj'])
        if issue_m and re.search(rf'#{issue_m.group(1)}[^)]*not.inspected', tr_txt, re.I):
            err(f"C3/C14: traceability says #{issue_m.group(1)} not inspected but inv=complete")
            c3_ok=False

# Derive comment threads and compare to completion report
thread_str = ', '.join(f"{r['repo']}/{r['obj']} ({r['cmt_cnt']} comments)" for r in hm_comment_threads)
# Verify all expected threads are listed in report
for r in hm_comment_threads:
    issue_m = re.search(r'#(\d+)', r['obj'])
    if issue_m:
        inum = issue_m.group(1)
        if inum not in rpt_txt[rpt_txt.find('| 3.'):rpt_txt.find('| 4.')]:
            warn(f"C3: report criterion 3 doesn't mention issue #{inum} comments")

c3 = c3_ok

# ── C4: LinuxCNC authority ──────────────────────────────────────────────────
a1 = next((s for s in sources if s.get('source_id')=='A-001'), None)
a2 = next((s for s in sources if s.get('source_id')=='A-002'), None)
for src, label in [(a1,"A-001"),(a2,"A-002")]:
    if not src or not src.get('inspected'): err(f"C4: {label} missing/not inspected")
c4 = bool(a1 and a1.get('inspected') and a2 and a2.get('inspected'))

# ── C5: hal-canon provenance ────────────────────────────────────────────────
b_ids = {s['source_id'] for s in sources if s.get('authority_class','')=='B'}
for bid in ['B-001','B-002','B-003']:
    if bid not in b_ids: err(f"C5: {bid} missing")
c5 = all(bid in b_ids for bid in ['B-001','B-002','B-003'])

# ── C6: Parity extraction ───────────────────────────────────────────────────
n_act = sum(1 for r in parity if r.get('active_or_commented')=='active')
n_com = sum(1 for r in parity if r.get('active_or_commented')=='commented')
if len(parity)!=163: err(f"C6: parity {len(parity)}")
if n_act!=162:        err(f"C6: active {n_act}")
if n_com!=1:          err(f"C6: commented {n_com}")
ext = os.path.join(PHASE0_DIR, "tools", "extract_legacy_pev2_exports.py")
pf  = os.path.join(PHASE0_DIR, "legacy-pev2-parity.yaml")
rv  = subprocess.run([sys.executable, ext, "--check-parity", pf], capture_output=True, text=True)
if rv.returncode!=0: err(f"C6: parity validator: {(rv.stdout+rv.stderr).strip()}")
la013 = next((r for r in parity if r.get('legacy_id')=='LA-013'), None)
la022 = next((r for r in parity if r.get('legacy_id')=='LA-022'), None)
if la013 and la013.get('current_interface_id')!='PEV2A-006':
    err(f"C6/C14: LA-013 iid={la013.get('current_interface_id')!r}")
if la022 and la022.get('current_interface_id')!='PEV2A-007':
    err(f"C6/C14: LA-022 iid={la022.get('current_interface_id')!r}")
for label, doc in [("source-reg", sr_raw), ("report", rpt_txt)]:
    for s in ['140 rows','139 active','56 patterns']:
        if s in doc: err(f"C6/C14: stale {s!r} in {label}")
c6 = (len(parity)==163 and n_act==162 and n_com==1 and rv.returncode==0
      and (not la013 or la013.get('current_interface_id')=='PEV2A-006')
      and (not la022 or la022.get('current_interface_id')=='PEV2A-007'))

# ── C7: Current interface extraction ────────────────────────────────────────
f_ids = {s['source_id'] for s in sources if s.get('authority_class','')=='F'}
for fid in ['F-001','F-002','F-003','F-005','F-008']:
    if fid not in f_ids: err(f"C7: {fid} missing")
for r in reqs:
    for sr_ref in r.get('requirement_sources',[])+r.get('implementation_sources',[]):
        if sr_ref not in src_map: err(f"C7: broken src ref {sr_ref!r} in {r['interface_id']}")
# PEV2A-006 and PEV2A-007 required
has_axescmd = 'PEV2A-006' in req_map
has_idxen   = 'PEV2A-007' in req_map
if not has_axescmd: err("C7: PEV2A-006 (AxesCommand) missing")
if not has_idxen:   err("C7: PEV2A-007 (index-enable) missing")
# PEV2A-007 must not claim unsupported encoder functions
p7 = req_map.get('PEV2A-007',{})
if 'encoder driver clears' in str(p7.get('update_phase','')): err("C7: PEV2A-007 encoder driver claim")
if 'motion controller sets' in str(p7.get('update_phase','')): err("C7: PEV2A-007 motion controller claim")
# No rtapi_shmem_new zero-init claim in catalogue
if 'rtapi_shmem_new zeroed allocation' in rc_raw: err("C7: rtapi_shmem_new claim in catalogue")
# HOMECOMP-* defaults must not use bare 0 without qualification for HAL_OUT pins
for r in reqs:
    if r['interface_id'].startswith('HOMECOMP-'):
        dv = str(r.get('default_value',''))
        dir_ = r.get('direction_or_access','')
        if dir_=='HAL_OUT' and dv.strip() in ('0','0 (IDLE)'):
            err(f"C7: {r['interface_id']} HAL_OUT uses bare default_value without initialization evidence")
c7 = (all(fid in f_ids for fid in ['F-001','F-002','F-003','F-005','F-008'])
      and has_axescmd and has_idxen and not any('C7:' in e for e in errors))

# ── C8: Library ownership ───────────────────────────────────────────────────
sA_end = lm_txt.find('\n## B.')
sA = lm_txt[:sA_end] if sA_end>0 else lm_txt
if '| **pokeys_homecomp' in sA: err("C8: homecomp row in Section A")
if 'Device→HAL' not in lm_txt: err("C8: Device→HAL missing")
if 'HAL→device' not in lm_txt: err("C8: HAL→device missing")
for phrase in ['All pin defaults are 0 from zeroed shmem','rtapi_shmem_new zeroed allocation']:
    if phrase in lm_txt: err(f"C8: unsupported claim {phrase!r} in lifecycle matrix")
for r in reqs:
    if r['interface_id'].startswith('HOMECOMP-'):
        for sr_ref in r.get('implementation_sources',[]):
            if sr_ref.startswith('F-'): err(f"C8: {r['interface_id']} has F-class impl src")
c8 = not any('C8:' in e for e in errors)

# ── C9: Enumerations ────────────────────────────────────────────────────────
has_state_enum = any('ePK_PEAxisState' in str(r.get('enumeration','')) for r in reqs)
has_cmd_enum   = any(r.get('interface_id')=='PEV2A-006' and r.get('enumeration') for r in reqs)
if not has_state_enum: err("C9: ePK_PEAxisState enum missing")
if not has_cmd_enum:   err("C9: PEV2A-006 AxesCommand enum missing")
c9 = has_state_enum and has_cmd_enum

# ── C10: Canonical classification ───────────────────────────────────────────
if not cm_txt: err("C10: canonical-vs-legacy-matrix.md missing")
else:
    # Must have explicit rows, not just substring
    has_canonical_rows = bool(re.search(r'\*\*canonical.*compatible\*\*|\*\*canonical\*\*', cm_txt, re.I))
    has_pokeys_spec    = 'PoKeys-specific' in cm_txt
    has_adcout_row     = bool(re.search(r'\|\s*`adcout\b', cm_txt))
    has_adcin_row      = bool(re.search(r'\|\s*`adcin\b', cm_txt))
    if not has_canonical_rows: err("C10: no explicit canonical rows")
    if not has_pokeys_spec:    err("C10: no PoKeys-specific classification")
    if not has_adcout_row:     err("C10: adcout not in matrix")
    if not has_adcin_row:      err("C10: adcin not in matrix")
    if 'conversion unverified' in cm_txt.lower(): err("C10/C14: matrix has 'conversion unverified'")
c10 = not any('C10:' in e for e in errors)

# ── C11: Contradictions registered ─────────────────────────────────────────
for r in reqs:
    for c in r.get('conflicts',[]):
        if c not in reg_conflicts: err(f"C11: unregistered {c!r} in {r['interface_id']}")
for lk in ilinks:
    for c in lk.get('conflicts',[]):
        if c not in reg_conflicts: err(f"C11: unregistered {c!r} in IK {lk['link_id']}")

# Validate conflict subject relevance in inventory/source
conflict_subjects = {}
for m in re.finditer(r'Conflict ID: (CONFLICT-\d+)\nSubject: ([^\n]+)', conf_txt, re.M):
    conflict_subjects[m.group(1)] = m.group(2).strip()

c002_subject = conflict_subjects.get('CONFLICT-002','')
for r in inv_rows:
    if 'CONFLICT-002' in r.get('conf',''):
        if any(kw in r.get('title','').lower() for kw in ['limitoverride','limit override','homing pin','joint-in-homing']):
            err(f"C11: {r['repo']}/{r['obj']} links CONFLICT-002 ({c002_subject!r}) but topic differs")

# Validate the requested special-case mappings for LC #216, #223, #310
for row in inv_rows:
    if row['repo'] == 'lc' and row['obj'] in ('Issue #216', 'Issue #223', 'Issue #310'):
        expected = {'Issue #216': 'CONFLICT-007', 'Issue #223': 'CONFLICT-007', 'Issue #310': 'CONFLICT-005'}
        if expected[row['obj']] not in row.get('conf',''):
            err(f"C11: {row['repo']}/{row['obj']} missing expected conflict {expected[row['obj']]}" )

# Ensure IK-002 is classified as unverified-incomplete and not as compatible
for lk in ilinks:
    if lk.get('link_id') == 'IK-002':
        if lk.get('compatibility_status') != 'unverified-incomplete':
            err("C11: IK-002 compatibility_status must be unverified-incomplete")
        if 'propagation' not in str(lk.get('semantic_contract','')).lower():
            err("C11: IK-002 semantic_contract must mention untraced propagation")

c11 = not any('C11:' in e for e in errors)

# ── C12: No production changed ──────────────────────────────────────────────
changed_raw = subprocess.run(['git','diff','--name-only',PRODUCTION_BASE],
                              capture_output=True,text=True,cwd=REPO_ROOT).stdout.strip()
for f in changed_raw.split('\n'):
    if not f: continue
    if f.startswith('docs/hal-interface/phase-0/'): continue
    if '.github/prompts/' in f: continue
    err(f"C12: non-Phase-0 file: {f}")
gc = subprocess.run(['git','diff','--check'], capture_output=True, text=True, cwd=REPO_ROOT)
if gc.returncode!=0: err(f"C12: diff --check: {gc.stdout[:200]}")
rv2 = subprocess.run(['git','diff','--exit-code',IMMUTABLE_COMMIT,'--',
                       os.path.join(PHASE0_DIR,"tools/extract_legacy_pev2_exports.py")],
                      capture_output=True, cwd=REPO_ROOT)
if rv2.returncode!=0: err("C12: extractor modified")
c12 = not any('C12:' in e for e in errors)

# ── C13: No test artifacts ──────────────────────────────────────────────────
cf = changed_raw.split('\n') if changed_raw else []
if any(re.search(r'test.*\.(py|c)$|hil_',f,re.I) and 'tools/validate' not in f for f in cf if f):
    err("C13: test artifacts added")
c13 = not any('C13:' in e for e in errors)

# ── C14: Cross-document consistency ─────────────────────────────────────────
c14_ok = True

# Criterion 14 must stay FAIL until the lifecycle/conflict corrections are present.
if 'PHASE 0 BASELINE INCOMPLETE' not in rpt_txt:
    err("C14: completion report not marked INCOMPLETE")
    c14_ok = False

# Lifecycle matrix must not attribute F-008/__comp_state/memset evidence to the external homecomp section.
section_b_match = re.search(r'^## B\. External Counterpart Evidence: pokeys_homecomp\s*(.*?)(?=^## |\Z)', lm_txt, re.M | re.S)
section_b = section_b_match.group(1) if section_b_match else lm_txt
if '__comp_state' in section_b:
    err("C14: lifecycle matrix still cites __comp_state in external homecomp section")
    c14_ok = False
if 'memset' in section_b:
    err("C14: lifecycle matrix still cites memset in external homecomp section")
    c14_ok = False

# HOMECOMP lifecycle phases must be explicit and tied to E-010 evidence.
for req_id in ['HOMECOMP-001','HOMECOMP-004','HOMECOMP-005','HOMECOMP-006','HOMECOMP-007']:
    entry = req_map.get(req_id)
    if not entry:
        err(f"C14: missing {req_id}")
        c14_ok = False
        continue
    fields = [entry.get('startup_pin_value'), entry.get('local_state_initial_value'), entry.get('first_propagation'), entry.get('steady_state_ownership')]
    if any(v is None for v in fields):
        err(f"C14: {req_id} missing lifecycle phase fields")
        c14_ok = False

# PEV2A-007/HOMECOMP-005/IK-002 chain must be explicit.
for req_id in ['PEV2A-007','HOMECOMP-005']:
    entry = req_map.get(req_id)
    if not entry:
        err(f"C14: missing {req_id}")
        c14_ok = False
        continue
    if 'IK-002' not in str(entry.get('evidence','')) and 'IK-002' not in str(entry.get('integration_link','')):
        err(f"C14: {req_id} missing IK-002 linkage")
        c14_ok = False

# All conflict mappings must be present and relevant.
for conflict_id in ['CONFLICT-005', 'CONFLICT-007']:
    if conflict_id not in reg_conflicts:
        err(f"C14: missing {conflict_id}")
        c14_ok = False

# The completion report should contain the explicit missing-evidence note.
if 'Lifecycle and conflict semantics must be corrected' not in rpt_txt:
    err("C14: completion report missing lifecycle/conflict remediation note")
    c14_ok = False

# ADCOUT consistency
for dn, dt in [("traceability",tr_txt),("open-decisions",od_txt),("req-catalogue",rc_raw)]:
    if 'conversion unverified' in dt.lower() or 'conversion path unverified' in dt.lower():
        err(f"C14: {dn} has 'conversion unverified'"); c14_ok=False

# Count table consistency
live = {
    'Source register entries':       len(sources),
    'Requirement catalogue entries': len(reqs),
    'Conflicts registered':          n_conf,
    'Open decisions required':       n_dec,
    'Traceability chains':           len(re.findall(r'^## \d+\.', tr_txt, re.M)),
}
for label, expected in live.items():
    for m in re.finditer(rf'\| {re.escape(label)} \| (\d+) \|', rpt_txt):
        if int(m.group(1))!=expected:
            err(f"C14: report '{label}'={m.group(1)} expected {expected}"); c14_ok=False

for lbl, cnt in [('Issues inventoried (pokeysHal)',pk_n),
                  ('Issues inventoried (LinuxCnc_PokeysLibComp)',lc_n),
                  ('Pull requests inventoried',pr_n)]:
    for m in re.finditer(rf'\| {re.escape(lbl)} \| (\d+) \|', rpt_txt):
        if int(m.group(1))!=cnt:
            err(f"C14: report '{lbl}'={m.group(1)} expected {cnt}"); c14_ok=False

# HOMECOMP defaults: HAL_OUT must not use bare unverified-but-stated 0
for r in reqs:
    if r['interface_id'].startswith('HOMECOMP-'):
        dv = str(r.get('default_value',''))
        dir_ = r.get('direction_or_access','')
        if dir_=='HAL_OUT' and re.match(r"^'?0'?$", dv.strip()):
            err(f"C14: {r['interface_id']} HAL_OUT default_value=0 without citation")
            c14_ok=False

# Scope boundary
for phrase, label in [("pokeysHal is a HOMEMOD","homemod"),
                       ("pokeys_homecomp is a pokeysHal subsystem","subsystem")]:
    for dn, dt in [("lifecycle",lm_txt),("traceability",tr_txt)]:
        if phrase.lower() in dt.lower(): err(f"C14: {label} in {dn}"); c14_ok=False

c14 = c14_ok and not any('C14:' in e for e in errors)

# ── Criterion statuses ──────────────────────────────────────────────────────
computed = {1:c1,2:c2,3:c3,4:c4,5:c5,6:c6,7:c7,8:c8,9:c9,10:c10,11:c11,12:c12,13:c13,14:c14}

for cnum, cpass in computed.items():
    m = re.search(rf'\| {cnum}\. .*?\| (PASS|PARTIAL|FAIL) \|', rpt_txt)
    if not m:
        err(f"Criterion {cnum} not in report")
    else:
        rs = m.group(1)
        if cpass and rs not in ('PASS',):
            err(f"Criterion {cnum}: computed PASS but report says {rs}")
        elif not cpass and rs=='PASS':
            err(f"Criterion {cnum}: computed NOT PASS but report says PASS")

if all(computed.values()):
    if 'PHASE 0 BASELINE COMPLETE' not in rpt_txt:
        err("All PASS but report not COMPLETE")
else:
    if 'PHASE 0 BASELINE INCOMPLETE' not in rpt_txt:
        err("Not all PASS but report not INCOMPLETE")

# ── Summary ─────────────────────────────────────────────────────────────────
print(f"Sources:{len(sources)} Reqs:{len(reqs)} Parity:{len(parity)} "
      f"Conf:{n_conf} Dec:{n_dec}")
print(f"Inventory: pk={pk_n} lc={lc_n} prs={pr_n}")
print(f"Comment threads: {len(hm_comment_threads)}")
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
    print("\nPHASE 0 BASELINE INCOMPLETE"); sys.exit(1)
else:
    print(f"\n  {rv.stdout.strip().split(chr(10))[-1]}")
    print("\nALL PHASE 0 CLOSURE CHECKS PASSED")
    print("PHASE 0 BASELINE COMPLETE"); sys.exit(0)
