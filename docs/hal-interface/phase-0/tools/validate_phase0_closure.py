#!/usr/bin/env python3
"""
Phase 0 Closure Validator — strengthened edition
Independently derives all 14 acceptance criteria from parsed artifacts.
"""
import sys, os, re, subprocess, yaml

PHASE0_DIR       = os.path.normpath(os.path.join(os.path.dirname(__file__), '..'))
REPO_ROOT        = os.path.normpath(os.path.join(PHASE0_DIR, '..', '..', '..'))
IMMUTABLE_COMMIT = '4f0ab5e'
PRODUCTION_BASE  = 'cd1f0dc8a0f64f92dc6bdce21bddcb36d33a14cd'

errors = []
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
if not ilinks:  err("integration-links has zero entries")

conf_txt = open(os.path.join(PHASE0_DIR, "conflict-register.md"), encoding='utf-8').read()
od_txt   = open(os.path.join(PHASE0_DIR, "open-decisions.md"), encoding='utf-8').read()
tr_txt   = open(os.path.join(PHASE0_DIR, "traceability.md"), encoding='utf-8').read()
inv_txt  = open(os.path.join(PHASE0_DIR, "issue-inventory.md"), encoding='utf-8').read()
rpt_txt  = open(os.path.join(PHASE0_DIR, "phase-0-completion-report.md"), encoding='utf-8').read()
rm_txt   = open(os.path.join(PHASE0_DIR, "README.md"), encoding='utf-8').read()
lm_txt   = open(os.path.join(PHASE0_DIR, "lifecycle-ownership-matrix.md"), encoding='utf-8').read()

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
            tpl = (row['repo'], row['obj'])
            if tpl in inv_tuples: err(f"Duplicate inventory tuple: {tpl}")
            inv_tuples.add(tpl)
            inv_rows.append(row)

if not inv_rows: err("issue-inventory: zero rows parsed")

pk_n = sum(1 for r in inv_rows if r['repo']=='pk' and not r['obj'].startswith('PR'))
lc_n = sum(1 for r in inv_rows if r['repo']=='lc' and not r['obj'].startswith('PR'))
pr_n = sum(1 for r in inv_rows if r['obj'].startswith('PR'))

# Registered conflicts and decisions
reg_conflicts = set(re.findall(r'^Conflict ID: (CONFLICT-\d+)', conf_txt, re.M))
reg_decisions = set(re.findall(r'^### (DEC-[A-Z0-9_-]+)', od_txt, re.M))
n_conf = len(reg_conflicts)
n_dec  = len(reg_decisions)

# ── Stale-phrase scanners ─────────────────────────────────────────────────────
def check_stale(txt, label):
    for stale in ['140 rows','139 active','56 patterns','CONFLICT-012','DEC-LIFE-002']:
        if stale in txt and 'removed' not in txt.lower() and 'excluded' not in txt.lower():
            # Allow CONFLICT-012 only in the external-observations section
            if stale == 'CONFLICT-012':
                idx = txt.find('CONFLICT-012')
                ctx = txt[max(0,idx-200):idx+200]
                if 'External Observations' in ctx or 'out-of-scope' in ctx.lower(): continue
            err(f"C14: stale reference {stale!r} in {label}")

check_stale(rpt_txt, "completion-report")
check_stale(rm_txt,  "README")
check_stale(tr_txt,  "traceability")
check_stale(od_txt,  "open-decisions")
check_stale(lm_txt,  "lifecycle-matrix")

# Scan all prose count occurrences (not just tables)
def check_prose_count(txt, label, count_label, live_value):
    for pat in [rf'\b{count_label}\b[^|]', rf'[^|]\b{count_label}\b']:
        for m in re.finditer(rf'\b(\d+)\s+(?:open\s+)?{re.escape(label.lower())}', txt, re.I):
            found = int(m.group(1))
            if found != live_value:
                err(f"C14: prose count mismatch in context — found '{m.group(0)}' but live={live_value}")

check_prose_count(rpt_txt, "conflict", "conflicts", n_conf)
check_prose_count(rpt_txt, "decision", "decisions", n_dec)
check_prose_count(rpt_txt, "open decision", "decisions", n_dec)

# References to removed conflict/decision IDs
for doc_name, doc_txt in [("report",rpt_txt),("traceability",tr_txt),
                            ("lifecycle",lm_txt),("req-cat", open(os.path.join(PHASE0_DIR,"requirement-catalogue.yaml")).read())]:
    for cid in re.findall(r'CONFLICT-\d+', doc_txt):
        if cid not in reg_conflicts:
            err(f"C11/C14: {doc_name} references unregistered {cid}")
    for did in re.findall(r'DEC-[A-Z0-9_-]+', doc_txt):
        if did not in reg_decisions:
            err(f"C14: {doc_name} references unregistered decision {did}")

# ── C1: Repository and revision identified ───────────────────────────────────
has_pk  = any('pokeysHal' in s.get('repository','') for s in sources)
has_lc  = any('LinuxCnc_PokeysLibComp' in s.get('repository','') for s in sources)
has_a   = any(s.get('authority_class','')=='A' and s.get('inspected') for s in sources)
if not has_pk: err("C1: no pokeysHal source entries")
if not has_lc: err("C1: no LinuxCnc_PokeysLibComp source entries")
if not has_a:  err("C1: no inspected A-class authority entries")
c1 = has_pk and has_lc and has_a

# ── C2: Issue bodies ──────────────────────────────────────────────────────────
c2_ok = True
for r in inv_rows:
    if r['retain'] in ('YES','MAYBE') and r['rel'] in ('HIGH','MEDIUM'):
        if r['body'] not in ('complete','empty','title-only'):
            err(f"C2: {r['repo']}/{r['obj']} body={r['body']!r}")
            c2_ok = False
c2 = c2_ok

# ── C3: Comments and source coverage ─────────────────────────────────────────
c3_ok = True
for r in inv_rows:
    if r['retain'] in ('YES','MAYBE') and r['rel'] in ('HIGH','MEDIUM'):
        cnt = r['cmt_cnt']
        cmt_s = r['cmt_s']
        # Exact comment_count/status consistency
        if cnt == '0' and cmt_s not in ('none',''):
            err(f"C3: {r['repo']}/{r['obj']} cmt_cnt=0 but cmt_s={cmt_s!r} (must be 'none')")
            c3_ok = False
        if cnt not in ('0','') and cmt_s not in ('complete','partial'):
            err(f"C3: {r['repo']}/{r['obj']} cmt_cnt={cnt} but cmt_s={cmt_s!r} (must be complete or partial)")
            c3_ok = False
        # Source ID: required for any issue with material evidence or comments
        if r['sid'] in (None,'None',''):
            if r['rel'] == 'HIGH' or (cnt not in ('0','') and cmt_s == 'complete'):
                warn(f"C3: {r['repo']}/{r['obj']} retained HIGH/MEDIUM with no source ID")
        elif r['sid'] not in src_map:
            err(f"C3: {r['repo']}/{r['obj']} source {r['sid']!r} not in register")
            c3_ok = False
        else:
            src = src_map[r['sid']]
            src_repo = src.get('repository','')
            pio = str(src.get('path_or_issue',''))
            # Cross-repo check
            if r['repo']=='pk' and 'LinuxCnc' in src_repo:
                err(f"C3: cross-repo {r['obj']} (pk) has LC source {r['sid']!r}")
                c3_ok = False
            if r['repo']=='lc' and 'pokeysHal' in src_repo and 'LinuxCnc' not in src_repo:
                err(f"C3: cross-repo {r['obj']} (lc) has pk source {r['sid']!r}")
                c3_ok = False
            issue_m = re.search(r'#(\d+)', r['obj'])
            if issue_m and issue_m.group(1) not in pio:
                err(f"C3: {r['repo']}/{r['obj']} source {r['sid']!r} pio={pio!r} mismatch")
                c3_ok = False
# Explicit cross-repo
lc_s = inv_txt[inv_txt.find('## LinuxCnc'):]
if re.search(r'\| Issue #24 \|[^|]+\| C-012 \|', lc_s): err("C3: LC #24 has C-012"); c3_ok=False
if re.search(r'\| Issue #129 \|[^|]+\| D-003 \|', lc_s): err("C3: LC #129 has D-003"); c3_ok=False
c3 = c3_ok

# Traceability inspection status vs inventory
for r in inv_rows:
    if r['retain'] in ('YES','MAYBE') and r['rel'] in ('HIGH','MEDIUM'):
        obj = r['obj']
        inv_body = r['body']
        # Search traceability for this issue
        m = re.search(rf'{re.escape(obj)}[^)]*\(([^)]+)\)', tr_txt)
        if m:
            tr_status = m.group(1).lower()
            if inv_body == 'complete' and ('not inspected' in tr_status or 'not fully' in tr_status):
                err(f"C3/C14: traceability says {obj} not inspected but inventory says body=complete")

# ── C4: LinuxCNC authority ───────────────────────────────────────────────────
a1 = next((s for s in sources if s.get('source_id')=='A-001'), None)
a2 = next((s for s in sources if s.get('source_id')=='A-002'), None)
if not a1 or not a1.get('inspected'): err("C4: A-001 missing/not inspected")
if not a2 or not a2.get('inspected'): err("C4: A-002 missing/not inspected")
c4 = bool(a1 and a1.get('inspected') and a2 and a2.get('inspected'))

# ── C5: hal-canon provenance ─────────────────────────────────────────────────
b_ids = {s['source_id'] for s in sources if s.get('authority_class','')=='B'}
for bid in ['B-001','B-002','B-003']:
    if bid not in b_ids: err(f"C5: {bid} missing")
c5 = all(bid in b_ids for bid in ['B-001','B-002','B-003'])

# ── C6: Parity extraction ────────────────────────────────────────────────────
n_act = sum(1 for r in parity if r.get('active_or_commented')=='active')
n_com = sum(1 for r in parity if r.get('active_or_commented')=='commented')
if len(parity)!=163: err(f"C6: parity={len(parity)}")
if n_act!=162:        err(f"C6: active={n_act}")
if n_com!=1:          err(f"C6: commented={n_com}")
ext = os.path.join(PHASE0_DIR, "tools", "extract_legacy_pev2_exports.py")
pf  = os.path.join(PHASE0_DIR, "legacy-pev2-parity.yaml")
rv  = subprocess.run([sys.executable, ext, "--check-parity", pf], capture_output=True, text=True)
if rv.returncode!=0: err(f"C6: parity validator: {(rv.stdout+rv.stderr).strip()}")
# Validate semantic fields updated
la013 = next((r for r in parity if r.get('legacy_id')=='LA-013'), None)
la022 = next((r for r in parity if r.get('legacy_id')=='LA-022'), None)
if la013 and la013.get('current_interface_id') not in ('PEV2A-006',):
    err(f"C6/C14: LA-013 current_interface_id={la013.get('current_interface_id')!r} expected PEV2A-006")
if la022 and la022.get('current_interface_id') not in ('PEV2A-007',):
    err(f"C6/C14: LA-022 current_interface_id={la022.get('current_interface_id')!r} expected PEV2A-007")
for doc, txt in [("source-reg", open(os.path.join(PHASE0_DIR,"source-register.yaml")).read()),
                  ("report", rpt_txt)]:
    for stale in ['140 rows','139 active','56 patterns']:
        if stale in txt: err(f"C6/C14: stale {stale!r} in {doc}")
c6 = (len(parity)==163 and n_act==162 and n_com==1 and rv.returncode==0
      and (not la013 or la013.get('current_interface_id')=='PEV2A-006')
      and (not la022 or la022.get('current_interface_id')=='PEV2A-007'))

# ── C7: Current interface extracted ──────────────────────────────────────────
f_ids = {s['source_id'] for s in sources if s.get('authority_class','')=='F'}
for fid in ['F-001','F-002','F-003','F-005','F-008']:
    if fid not in f_ids: err(f"C7: {fid} missing")
for r in reqs:
    for sr in r.get('requirement_sources',[])+r.get('implementation_sources',[]):
        if sr not in src_map: err(f"C7: broken src ref {sr!r} in {r['interface_id']}")
has_axescmd = any(r.get('interface_id')=='PEV2A-006' for r in reqs)
has_idxen   = any(r.get('interface_id')=='PEV2A-007' for r in reqs)
if not has_axescmd: err("C7: no PEV2A-006 (AxesCommand) entry")
if not has_idxen:   err("C7: no PEV2A-007 (index-enable) entry")
# PEV2A-006 must not claim PK_PEv2_PulseEngineMovePVAsync as identified function
p6 = req_map.get('PEV2A-006',{})
if 'PulseEngineMovePVAsync' in str(p6.get('pokeyslib_function','')) and 'unverified' not in str(p6.get('pokeyslib_function','')):
    if 'none' not in str(p6.get('pokeyslib_function','')).lower():
        err("C7: PEV2A-006 maps to PK_PEv2_PulseEngineMovePVAsync without evidence; must say 'none identified'")
# PEV2A-007 must not claim encoder driver clears without code path
p7 = req_map.get('PEV2A-007',{})
if 'encoder driver clears' in str(p7.get('update_phase','')):
    err("C7: PEV2A-007 claims 'encoder driver clears' without cited code path")
c7 = (all(fid in f_ids for fid in ['F-001','F-002','F-003','F-005','F-008'])
      and has_axescmd and has_idxen and not any('C7:' in e for e in errors))

# ── C8: Library ownership ────────────────────────────────────────────────────
has_sec_a = '## A. pokeysHal' in lm_txt
has_sec_b = '## B. External Counterpart' in lm_txt
has_sec_c = '## C. Integration Lifecycle' in lm_txt
sA_end = lm_txt.find('\n## B.')
sA = lm_txt[:sA_end] if sA_end>0 else lm_txt
if '| **pokeys_homecomp' in sA: err("C8: homecomp row in Section A")
if 'Device→HAL' not in lm_txt: err("C8: Device→HAL ownership missing")
if 'HAL→device' not in lm_txt: err("C8: HAL→device ownership missing")
# No unsupported zero-init claim
if 'All pin defaults are 0 from zeroed shmem' in lm_txt:
    err("C8: unsupported zero-init claim present")
for r in reqs:
    if r['interface_id'].startswith('HOMECOMP-'):
        if 'pokeys_homecomp' not in r.get('owner',''):
            err(f"C8: {r['interface_id']} owner not pokeys_homecomp")
        for sr in r.get('implementation_sources',[]):
            if sr.startswith('F-'): err(f"C8: {r['interface_id']} has F-class impl src")
c8 = (has_sec_a and has_sec_b and has_sec_c and
      'All pin defaults are 0 from zeroed shmem' not in lm_txt and
      not any('C8:' in e for e in errors))

# ── C9: Enumerations ─────────────────────────────────────────────────────────
has_state_enum = any('ePK_PEAxisState' in str(r.get('enumeration','')) for r in reqs)
has_cmd_enum   = any(r.get('interface_id')=='PEV2A-006' and r.get('enumeration') for r in reqs)
if not has_state_enum: err("C9: ePK_PEAxisState enum missing")
if not has_cmd_enum:   err("C9: PEV2A-006 AxesCommand enum missing")
c9 = has_state_enum and has_cmd_enum

# ── C10: Canonical classification ────────────────────────────────────────────
cm_path = os.path.join(PHASE0_DIR, "canonical-vs-legacy-matrix.md")
if not os.path.exists(cm_path): err("C10: canonical-vs-legacy-matrix.md missing")
else:
    cm = open(cm_path).read()
    if 'canonical' not in cm.lower(): err("C10: no canonical classification")
    if 'pokeys-specific' not in cm.lower(): err("C10: no PoKeys-specific classification")
    if 'adcout' not in cm.lower(): err("C10: adcout not in matrix")
    if 'conversion unverified' in cm.lower(): err("C10/C14: matrix says 'conversion unverified'")
c10 = not any('C10:' in e for e in errors)

# ── C11: Conflicts registered ────────────────────────────────────────────────
for r in reqs:
    for c in r.get('conflicts',[]):
        if c not in reg_conflicts: err(f"C11: unregistered conflict {c!r} in {r['interface_id']}")
for lk in ilinks:
    for c in lk.get('conflicts',[]):
        if c not in reg_conflicts: err(f"C11: unregistered conflict {c!r} in link {lk['link_id']}")
for cid in reg_conflicts:
    bs = conf_txt.find(f"Conflict ID: {cid}")
    ne = conf_txt.find("Conflict ID: CONFLICT-", bs+1)
    blk = conf_txt[bs:ne] if ne>0 else conf_txt[bs:]
    sline = next((l for l in blk.split('\n') if 'Subject:' in l), '')
    if 'volatile_home' in sline.lower() and 'AxesCommand' not in blk and 'digout' not in blk:
        err(f"C11: {cid} is solely about volatile_home (out of scope)")
c11 = not any('C11:' in e for e in errors)

# ── C12: No production changed ───────────────────────────────────────────────
changed = subprocess.run(['git','diff','--name-only',PRODUCTION_BASE],
                         capture_output=True, text=True, cwd=REPO_ROOT).stdout.strip()
for f in changed.split('\n'):
    if not f: continue
    if f.startswith('docs/hal-interface/phase-0/'): continue
    if '.github/prompts/' in f: continue
    err(f"C12: non-Phase-0 file changed: {f}")
gc = subprocess.run(['git','diff','--check'], capture_output=True, text=True, cwd=REPO_ROOT)
if gc.returncode!=0: err(f"C12: diff --check: {gc.stdout[:200]}")
for imm in ["legacy-pev2-parity.yaml","tools/extract_legacy_pev2_exports.py"]:
    rv2 = subprocess.run(['git','diff','--exit-code',IMMUTABLE_COMMIT,'--',
                          os.path.join(PHASE0_DIR,imm)], capture_output=True, cwd=REPO_ROOT)
    if rv2.returncode!=0:
        # parity is editable (semantic fields only); extractor is immutable
        if 'extract_legacy_pev2_exports' in imm:
            err(f"C12: extractor is immutable and was changed")
        else:
            # Parity: verify only extracted fields unchanged (fingerprints)
            if rv.returncode==0:
                pass  # fingerprints confirmed unchanged by parity validator
            else:
                err(f"C12: parity fingerprints changed")
c12 = not any('C12:' in e for e in errors)

# ── C13: No test artifacts ───────────────────────────────────────────────────
cf = changed.split('\n') if changed else []
test_added = any(re.search(r'test.*\.(py|c)$|hil_', f, re.I) and 'tools/validate' not in f
                 for f in cf if f)
if test_added: err("C13: test artifacts added")
c13 = not test_added

# ── C14: Cross-document consistency ──────────────────────────────────────────
c14_ok = True
# ADCOUT consistency
ADCOUT_DOCS = [("traceability",tr_txt),("open-decisions",od_txt),
               ("req-cat", open(os.path.join(PHASE0_DIR,"requirement-catalogue.yaml")).read())]
for dn, dt in ADCOUT_DOCS:
    for phrase in ['conversion unverified','conversion path unverified']:
        if phrase in dt.lower(): err(f"C14: {dn} says '{phrase}'"); c14_ok=False
# Count table consistency
live = {'Source register entries':len(sources),'Requirement catalogue entries':len(reqs),
        'Conflicts registered':n_conf,'Open decisions required':n_dec,
        'Traceability chains':len(re.findall(r'^## \d+\.', tr_txt, re.M))}
for label, expected in live.items():
    for m in re.finditer(rf'\| {re.escape(label)} \| (\d+) \|', rpt_txt):
        if int(m.group(1))!=expected:
            err(f"C14: report '{label}' says {m.group(1)} not {expected}"); c14_ok=False
# Issue count table consistency
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
    for dn, dt in [("lifecycle",lm_txt),("traceability",tr_txt),("report",rpt_txt)]:
        if phrase.lower() in dt.lower(): err(f"C14: {label} in {dn}"); c14_ok=False
# Parity semantic fields correctness (already checked above in C6)
if la013 and la013.get('current_interface_id')!='PEV2A-006': c14_ok=False
if la022 and la022.get('current_interface_id')!='PEV2A-007': c14_ok=False
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
    print(f"\n  Parity: {rv.stdout.strip().split(chr(10))[-1]}")
    print("\nALL PHASE 0 CLOSURE CHECKS PASSED")
    print("PHASE 0 BASELINE COMPLETE")
    sys.exit(0)
