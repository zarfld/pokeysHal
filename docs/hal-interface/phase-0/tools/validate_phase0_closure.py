#!/usr/bin/env python3
"""
Phase 0 Closure Validator — comprehensive edition.
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
        err(f"YAML parse: {fname}: {e}"); return {}

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

# Load text artifacts
def load_txt(fname):
    return open(os.path.join(PHASE0_DIR, fname), encoding='utf-8').read()

conf_txt = load_txt("conflict-register.md")
od_txt   = load_txt("open-decisions.md")
tr_txt   = load_txt("traceability.md")
inv_txt  = load_txt("issue-inventory.md")
rpt_txt  = load_txt("phase-0-completion-report.md")
rm_txt   = load_txt("README.md")
lm_txt   = load_txt("lifecycle-ownership-matrix.md")
sr_raw   = open(os.path.join(PHASE0_DIR, "source-register.yaml"), encoding='utf-8').read()
rc_raw   = open(os.path.join(PHASE0_DIR, "requirement-catalogue.yaml"), encoding='utf-8').read()

reg_conflicts = set(re.findall(r'^Conflict ID: (CONFLICT-\d+)', conf_txt, re.M))
reg_decisions = set(re.findall(r'^### (DEC-[A-Z0-9_-]+)', od_txt, re.M))
n_conf = len(reg_conflicts)
n_dec  = len(reg_decisions)

# ── Parse inventory rows ─────────────────────────────────────────────────────
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
            tpl = (row['repo'], row['obj'])
            if tpl in inv_tuples: err(f"Duplicate inventory tuple: {tpl}")
            inv_tuples.add(tpl)
            inv_rows.append(row)

if not inv_rows: err("issue-inventory: zero rows parsed")

pk_n = sum(1 for r in inv_rows if r['repo']=='pk' and not r['obj'].startswith('PR'))
lc_n = sum(1 for r in inv_rows if r['repo']=='lc' and not r['obj'].startswith('PR'))
pr_n = sum(1 for r in inv_rows if r['obj'].startswith('PR'))

# ── Stale-phrase scanners ─────────────────────────────────────────────────────
STALE_PHRASES = [
    '140 rows','139 active','56 patterns',
    'legitimately have Source ID None',
    'not body-inspected',
]
REMOVED_IDS = ['CONFLICT-012', 'DEC-LIFE-002']

ALL_DOCS = {
    "completion-report": rpt_txt,
    "README": rm_txt,
    "traceability": tr_txt,
    "open-decisions": od_txt,
    "lifecycle-matrix": lm_txt,
    "source-register": sr_raw,
    "req-catalogue": rc_raw,
}

def scan_stale(txt, label):
    for phrase in STALE_PHRASES:
        if phrase in txt:
            err(f"C14: stale phrase {phrase!r} in {label}")
    for rid in REMOVED_IDS:
        # Must not appear as active references (outside the explicit out-of-scope note)
        for m in re.finditer(re.escape(rid), txt):
            ctx = txt[max(0,m.start()-300):m.end()+300]
            is_excluded = any(kw in ctx for kw in [
                'External Observations', 'out-of-scope note', 'removed from baseline',
                'excluded from the baseline', 'not a pokeysHal', 'belongs to'
            ])
            if not is_excluded:
                err(f"C14: active reference to removed ID {rid!r} in {label} (context: {ctx[270:330]!r})")
                break

for label, txt in ALL_DOCS.items():
    scan_stale(txt, label)

# All doc references to conflict/decision IDs must resolve
for doc_name, doc_txt in ALL_DOCS.items():
    for cid in re.findall(r'CONFLICT-\d+', doc_txt):
        if cid not in reg_conflicts:
            err(f"C11/C14: {doc_name} references unregistered {cid}")
    for did in re.findall(r'DEC-[A-Z0-9_-]+', doc_txt):
        if did not in reg_decisions:
            err(f"C14: {doc_name} references unregistered decision {did}")

# Prose count consistency (not just table rows)
def check_prose_count(txt, label, keyword, live_value):
    for m in re.finditer(rf'\b(\d+)\s+(?:open\s+)?{re.escape(keyword)}', txt, re.I):
        found = int(m.group(1))
        if found != live_value:
            err(f"C14: prose count mismatch in {label}: found '{m.group(0)}' expected {live_value}")

for label, txt in [("report", rpt_txt), ("README", rm_txt)]:
    check_prose_count(txt, label, "conflict", n_conf)
    check_prose_count(txt, label, "decision", n_dec)
    check_prose_count(txt, label, "open decision", n_dec)

# ── C1: Repository and revision identified ───────────────────────────────────
# Require pinned revisions in source register
has_pk = any('pokeysHal' in s.get('repository','') for s in sources)
has_lc = any('LinuxCnc_PokeysLibComp' in s.get('repository','') for s in sources)
has_linuxcnc = any(s.get('authority_class','')=='A' and s.get('inspected') for s in sources)
# Check specific tree SHA appears in register
has_main_sha = 'cd1f0dc8a0f64f92dc6bdce21bddcb36d33a14cd' in sr_raw
has_lc_sha   = '0c058e6c7136ddb28a6b9f463a8af3d973496246' in sr_raw
if not has_pk: err("C1: no pokeysHal source entries")
if not has_lc: err("C1: no LinuxCnc_PokeysLibComp source entries")
if not has_linuxcnc: err("C1: no A-class LinuxCNC authority entries")
if not has_main_sha: err("C1: main repo commit SHA cd1f0dc8 not found in source register")
if not has_lc_sha:   err("C1: LC_PKComp commit 0c058e6c not found in source register")
c1 = has_pk and has_lc and has_linuxcnc and has_main_sha and has_lc_sha

# ── C2: Issue body inspection ────────────────────────────────────────────────
c2_ok = True
for r in inv_rows:
    if r['retain'] in ('YES','MAYBE') and r['rel'] in ('HIGH','MEDIUM'):
        if r['body'] not in ('complete','empty','title-only'):
            err(f"C2: {r['repo']}/{r['obj']} body={r['body']!r}")
            c2_ok = False
        # Reconcile with README methodology
        issue_m = re.search(r'#(\d+)', r['obj'])
        if issue_m and r['body'] == 'complete':
            inum = issue_m.group(1)
            # README must not claim this issue is not inspected
            if re.search(rf'#?{inum}[^0-9].*not body.inspected', rm_txt, re.I):
                err(f"C2: README says #{inum} not body-inspected but inventory says complete")
                c2_ok = False
c2 = c2_ok

# ── C3: Comment inspection and source coverage ───────────────────────────────
c3_ok = True
for r in inv_rows:
    if r['retain'] in ('YES','MAYBE') and r['rel'] in ('HIGH','MEDIUM'):
        cnt  = r['cmt_cnt']
        cs   = r['cmt_s']
        sid  = r['sid']
        # Comment/count consistency
        if cnt == '0' and cs not in ('none',''):
            err(f"C3: {r['repo']}/{r['obj']} cmt_cnt=0 but cmt_s={cs!r} (must be 'none')")
            c3_ok = False
        if cnt not in ('0','') and cs not in ('complete','partial'):
            err(f"C3: {r['repo']}/{r['obj']} cmt_cnt={cnt} but cmt_s={cs!r} (must be complete or partial)")
            c3_ok = False
        # Source ID: required for every retained HIGH/MEDIUM row
        if sid in (None, 'None', ''):
            err(f"C3: {r['repo']}/{r['obj']} (retained {r['rel']}) has no source-register entry")
            c3_ok = False
        elif sid not in src_map:
            err(f"C3: {r['repo']}/{r['obj']} source {sid!r} not in register")
            c3_ok = False
        else:
            src = src_map[sid]
            src_repo = src.get('repository','')
            pio = str(src.get('path_or_issue',''))
            # Cross-repo check
            if r['repo']=='pk' and 'LinuxCnc' in src_repo:
                err(f"C3: cross-repo: {r['obj']} (pk) has LC source {sid!r}")
                c3_ok = False
            if r['repo']=='lc' and 'pokeysHal' in src_repo and 'LinuxCnc' not in src_repo:
                err(f"C3: cross-repo: {r['obj']} (lc) has pk source {sid!r}")
                c3_ok = False
            # Issue number match
            issue_m = re.search(r'#(\d+)', r['obj'])
            if issue_m and issue_m.group(1) not in pio:
                err(f"C3: {r['repo']}/{r['obj']} source {sid!r} pio={pio!r} mismatch")
                c3_ok = False
            # Source inspection status
            if not src.get('inspected'):
                err(f"C3: source {sid!r} for {r['repo']}/{r['obj']} not marked inspected")
                c3_ok = False
            # Comment count validation: if cmt_cnt>0, source notes must mention count
            if cnt not in ('0','') and cs == 'complete':
                src_notes = str(src.get('notes','')) + str(src.get('scope',''))
                if not re.search(rf'\b{cnt}\b', src_notes) and 'comment' not in src_notes.lower():
                    warn(f"C3: {r['repo']}/{r['obj']} source {sid!r} notes don't mention {cnt} comments")

# Explicit cross-repo check
lc_s = inv_txt[inv_txt.find('## LinuxCnc'):]
if re.search(r'\| Issue #24 \|[^|]+\| C-012 \|', lc_s): err("C3: LC #24 has C-012"); c3_ok=False
if re.search(r'\| Issue #129 \|[^|]+\| D-003 \|', lc_s): err("C3: LC #129 has D-003"); c3_ok=False

# Traceability vs inventory: inspection status must agree
for r in inv_rows:
    if r['retain'] in ('YES','MAYBE') and r['rel'] in ('HIGH','MEDIUM') and r['body'] == 'complete':
        issue_m = re.search(r'#(\d+)', r['obj'])
        if issue_m:
            inum = issue_m.group(1)
            # Find this issue in traceability
            tr_match = re.search(rf'#{inum}[^)]*\([^)]*not inspected[^)]*\)', tr_txt, re.I)
            if tr_match:
                err(f"C3/C14: traceability says #{inum} not inspected but inventory says body=complete")
                c3_ok = False

c3 = c3_ok

# ── C4: LinuxCNC authority sources ──────────────────────────────────────────
a1 = next((s for s in sources if s.get('source_id')=='A-001'), None)
a2 = next((s for s in sources if s.get('source_id')=='A-002'), None)
if not a1 or not a1.get('inspected'): err("C4: A-001 (hal.h) missing/not inspected")
if not a2 or not a2.get('inspected'): err("C4: A-002 (CDI spec) missing/not inspected")
c4 = bool(a1 and a1.get('inspected') and a2 and a2.get('inspected'))

# ── C5: hal-canon provenance ─────────────────────────────────────────────────
b_ids = {s['source_id'] for s in sources if s.get('authority_class','')=='B'}
for bid in ['B-001','B-002','B-003']:
    if bid not in b_ids: err(f"C5: {bid} missing")
c5 = all(bid in b_ids for bid in ['B-001','B-002','B-003'])

# ── C6: Parity extraction ────────────────────────────────────────────────────
n_act = sum(1 for r in parity if r.get('active_or_commented')=='active')
n_com = sum(1 for r in parity if r.get('active_or_commented')=='commented')
if len(parity)!=163: err(f"C6: parity count {len(parity)} != 163")
if n_act!=162:        err(f"C6: active parity {n_act} != 162")
if n_com!=1:          err(f"C6: commented parity {n_com} != 1")

ext = os.path.join(PHASE0_DIR, "tools", "extract_legacy_pev2_exports.py")
pf  = os.path.join(PHASE0_DIR, "legacy-pev2-parity.yaml")
rv  = subprocess.run([sys.executable, ext, "--check-parity", pf], capture_output=True, text=True)
if rv.returncode != 0: err(f"C6: parity validator: {(rv.stdout+rv.stderr).strip()}")

# Semantic field mapping
la013 = next((r for r in parity if r.get('legacy_id')=='LA-013'), None)
la022 = next((r for r in parity if r.get('legacy_id')=='LA-022'), None)
if la013 and la013.get('current_interface_id') != 'PEV2A-006':
    err(f"C6/C14: LA-013 current_interface_id={la013.get('current_interface_id')!r} expected PEV2A-006")
if la022 and la022.get('current_interface_id') != 'PEV2A-007':
    err(f"C6/C14: LA-022 current_interface_id={la022.get('current_interface_id')!r} expected PEV2A-007")

c6 = (len(parity)==163 and n_act==162 and n_com==1 and rv.returncode==0
      and (not la013 or la013.get('current_interface_id')=='PEV2A-006')
      and (not la022 or la022.get('current_interface_id')=='PEV2A-007'))

# ── C7: Current HAL interface extracted ──────────────────────────────────────
# F-class sources for key implementation files
f_ids = {s['source_id'] for s in sources if s.get('authority_class','')=='F'}
for fid in ['F-001','F-002','F-003','F-005','F-008']:
    if fid not in f_ids: err(f"C7: {fid} missing")

# All requirement source refs resolve
for r in reqs:
    for sr_ref in r.get('requirement_sources',[])+r.get('implementation_sources',[]):
        if sr_ref not in src_map: err(f"C7: broken src ref {sr_ref!r} in {r['interface_id']}")

# Current interface coverage: PEV2A-006 and PEV2A-007 required
has_axescmd = 'PEV2A-006' in req_map
has_idxen   = 'PEV2A-007' in req_map
if not has_axescmd: err("C7: PEV2A-006 (AxesCommand) missing from catalogue")
if not has_idxen:   err("C7: PEV2A-007 (index-enable) missing from catalogue")

# Validate PEV2A-006: must not claim PK_PEv2_PulseEngineMovePVAsync without qualification
p6 = req_map.get('PEV2A-006', {})
pl6 = str(p6.get('pokeyslib_function',''))
if 'PulseEngineMovePVAsync' in pl6 and 'none' not in pl6.lower() and 'unverified' not in pl6.lower():
    err("C7: PEV2A-006 claims PK_PEv2_PulseEngineMovePVAsync without qualification")

# Validate PEV2A-007: must not claim encoder driver clears or PK_EncoderValuesGetAsync
p7 = req_map.get('PEV2A-007', {})
up7 = str(p7.get('update_phase',''))
pl7 = str(p7.get('pokeyslib_function',''))
if 'encoder driver clears' in up7: err("C7: PEV2A-007 unsupported 'encoder driver clears' claim")
if 'PK_EncoderValuesGetAsync' in pl7 and 'none' not in pl7.lower():
    err("C7: PEV2A-007 cites PK_EncoderValuesGetAsync without code path evidence")
if 'motion controller sets' in up7: err("C7: PEV2A-007 unsupported 'motion controller sets' claim")

# Default/init claims need cited evidence
for r in reqs:
    dv = str(r.get('default_value',''))
    if 'rtapi_shmem_new' in dv.lower() and 'zeroed' in dv.lower():
        err(f"C7: {r['interface_id']} claims rtapi_shmem_new zero-init without registered source")

c7 = (all(fid in f_ids for fid in ['F-001','F-002','F-003','F-005','F-008'])
      and has_axescmd and has_idxen
      and not any('C7:' in e for e in errors))

# ── C8: Library ownership ────────────────────────────────────────────────────
has_sec_a = '## A. pokeysHal' in lm_txt
has_sec_b = '## B. External Counterpart' in lm_txt
has_sec_c = '## C. Integration Lifecycle' in lm_txt
sA_end = lm_txt.find('\n## B.')
sA = lm_txt[:sA_end] if sA_end>0 else lm_txt
if '| **pokeys_homecomp' in sA: err("C8: homecomp row in Section A")
if 'Device→HAL' not in lm_txt: err("C8: Device→HAL ownership missing")
if 'HAL→device' not in lm_txt: err("C8: HAL→device ownership missing")
# No unsupported zero-init claims
for phrase in ['All pin defaults are 0 from zeroed shmem',
               'rtapi_shmem_new zeroed allocation']:
    if phrase in lm_txt: err(f"C8: unsupported claim {phrase!r} in lifecycle matrix")

# HOMECOMP-* must be external counterpart
for r in reqs:
    if r['interface_id'].startswith('HOMECOMP-'):
        if 'pokeys_homecomp' not in r.get('owner',''):
            err(f"C8: {r['interface_id']} owner not pokeys_homecomp")
        for sr_ref in r.get('implementation_sources',[]):
            if sr_ref.startswith('F-'): err(f"C8: {r['interface_id']} has F-class impl src")

c8 = (has_sec_a and has_sec_b and has_sec_c
      and not any('C8:' in e for e in errors))

# ── C9: Enumerations ─────────────────────────────────────────────────────────
has_state_enum = any('ePK_PEAxisState' in str(r.get('enumeration','')) for r in reqs)
has_cmd_enum   = any(r.get('interface_id')=='PEV2A-006' and r.get('enumeration') for r in reqs)
if not has_state_enum: err("C9: ePK_PEAxisState enum missing")
if not has_cmd_enum:   err("C9: PEV2A-006 AxesCommand enum missing")
c9 = has_state_enum and has_cmd_enum

# ── C10: Canonical classification ────────────────────────────────────────────
cm_path = os.path.join(PHASE0_DIR, "canonical-vs-legacy-matrix.md")
if not os.path.exists(cm_path):
    err("C10: canonical-vs-legacy-matrix.md missing"); c10 = False
else:
    cm = open(cm_path).read()
    # Must have explicit canonical vs non-canonical rows
    has_canonical_row = bool(re.search(r'\*\*canonical\*\*|\*\*canonical and compatible\*\*', cm, re.I))
    has_pokeys_spec   = bool(re.search(r'PoKeys-specific', cm))
    has_adcout_row    = 'adcout' in cm.lower()
    has_adcin_row     = 'adcin' in cm.lower()
    if not has_canonical_row: err("C10: no explicit canonical classification rows in matrix")
    if not has_pokeys_spec:   err("C10: no PoKeys-specific classification in matrix")
    if not has_adcout_row:    err("C10: adcout not classified in matrix")
    if not has_adcin_row:     err("C10: adcin not classified in matrix")
    if 'conversion unverified' in cm.lower(): err("C10/C14: matrix says 'conversion unverified'")
    c10 = not any('C10:' in e for e in errors)

# ── C11: Conflicts ───────────────────────────────────────────────────────────
for r in reqs:
    for c in r.get('conflicts',[]):
        if c not in reg_conflicts: err(f"C11: unregistered conflict {c!r} in {r['interface_id']}")
for lk in ilinks:
    for c in lk.get('conflicts',[]):
        if c not in reg_conflicts: err(f"C11: unregistered conflict {c!r} in link {lk['link_id']}")
c11 = not any('C11:' in e for e in errors)

# ── C12: No production changed ───────────────────────────────────────────────
changed_raw = subprocess.run(['git','diff','--name-only',PRODUCTION_BASE],
                              capture_output=True, text=True, cwd=REPO_ROOT).stdout.strip()
for f in changed_raw.split('\n'):
    if not f: continue
    if f.startswith('docs/hal-interface/phase-0/'): continue
    if '.github/prompts/' in f: continue
    err(f"C12: non-Phase-0 file changed: {f}")
gc = subprocess.run(['git','diff','--check'], capture_output=True, text=True, cwd=REPO_ROOT)
if gc.returncode!=0: err(f"C12: diff --check: {gc.stdout[:200]}")
# Extractor must be immutable
rv2 = subprocess.run(['git','diff','--exit-code',IMMUTABLE_COMMIT,'--',
                       os.path.join(PHASE0_DIR,"tools/extract_legacy_pev2_exports.py")],
                      capture_output=True, cwd=REPO_ROOT)
if rv2.returncode!=0: err("C12: extract_legacy_pev2_exports.py modified")
c12 = not any('C12:' in e for e in errors)

# ── C13: No test artifacts ───────────────────────────────────────────────────
cf = changed_raw.split('\n') if changed_raw else []
test_added = any(re.search(r'test.*\.(py|c)$|hil_', f, re.I) and 'tools/validate' not in f
                 for f in cf if f)
if test_added: err("C13: test artifacts added")
c13 = not test_added

# ── C14: Cross-document consistency ──────────────────────────────────────────
c14_ok = True

# ADCOUT consistency
for dn, dt in [("traceability",tr_txt),("open-decisions",od_txt),("req-catalogue",rc_raw)]:
    for phrase in ['conversion unverified','conversion path unverified']:
        if phrase in dt.lower():
            err(f"C14: {dn} says '{phrase}'"); c14_ok=False

# Count table consistency in report
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
            err(f"C14: report '{label}' says {m.group(1)} not {expected}"); c14_ok=False

# Issue count consistency
for lbl, cnt in [('Issues inventoried (pokeysHal)',pk_n),
                  ('Issues inventoried (LinuxCnc_PokeysLibComp)',lc_n),
                  ('Pull requests inventoried',pr_n)]:
    for m in re.finditer(rf'\| {re.escape(lbl)} \| (\d+) \|', rpt_txt):
        if int(m.group(1))!=cnt:
            err(f"C14: report '{lbl}' says {m.group(1)} not {cnt}"); c14_ok=False

# Scope boundary
BANNED = [("pokeysHal is a HOMEMOD","homemod misattribute"),
          ("pokeys_homecomp is a pokeysHal subsystem","subsystem misattribute"),
          ("pokeysHal owns joint","joint.N.* ownership")]
for phrase, label in BANNED:
    for dn, dt in [("lifecycle",lm_txt),("traceability",tr_txt)]:
        if phrase.lower() in dt.lower(): err(f"C14: {label} in {dn}"); c14_ok=False

# No unsupported rtapi_shmem_new zero claim in requirement catalogue
if 'rtapi_shmem_new zeroed allocation' in rc_raw:
    err("C14: rtapi_shmem_new zeroed allocation claim in requirement-catalogue"); c14_ok=False

c14 = c14_ok and not any('C14:' in e for e in errors)

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
        elif not cpass and rs=='PASS':
            err(f"Criterion {cnum}: computed NOT PASS but report says PASS")

# Final status
if all(computed.values()):
    if 'PHASE 0 BASELINE COMPLETE' not in rpt_txt:
        err("All criteria PASS but report not COMPLETE")
else:
    if 'PHASE 0 BASELINE INCOMPLETE' not in rpt_txt:
        err("Criteria not all PASS but report not INCOMPLETE")

# ── Summary ───────────────────────────────────────────────────────────────────
print(f"Sources:{len(sources)} Reqs:{len(reqs)} Parity:{len(parity)} "
      f"Conf:{n_conf} Dec:{n_dec}")
print(f"Inventory: pk={pk_n} lc={lc_n} prs={pr_n}")
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
    print(f"\n  {rv.stdout.strip().split(chr(10))[-1]}")
    print("\nALL PHASE 0 CLOSURE CHECKS PASSED")
    print("PHASE 0 BASELINE COMPLETE")
    sys.exit(0)
