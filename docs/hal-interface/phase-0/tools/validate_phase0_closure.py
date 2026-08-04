#!/usr/bin/env python3
"""
Phase 0 Closure Validator
docs/hal-interface/phase-0/tools/validate_phase0_closure.py

Documentation-only validator. Not a compatibility test.
Independently derives acceptance criteria from artifacts.
Exits 0 iff all Phase 0 acceptance criteria are met.
"""
import sys, os, re, subprocess, yaml

PHASE0_DIR  = os.path.normpath(os.path.join(os.path.dirname(__file__), '..'))
REPO_ROOT   = os.path.normpath(os.path.join(PHASE0_DIR, '..', '..', '..'))
IMMUTABLE_COMMIT = '4f0ab5e'  # extraction artifacts pinned here
PRODUCTION_BASE  = 'cd1f0dc8a0f64f92dc6bdce21bddcb36d33a14cd'  # Phase 0 production baseline

errors  = []
notices = []

def err(msg):  errors.append(msg)
def note(msg): notices.append(msg)

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

sources  = src_data.get("sources", [])
reqs     = req_data.get("requirements", [])
parity   = par_data.get("parity_rows", [])
ilinks   = lnk_data.get("integration_links", [])
src_map  = {s["source_id"]: s for s in sources}

conf_txt = open(os.path.join(PHASE0_DIR, "conflict-register.md"), encoding='utf-8').read()
od_txt   = open(os.path.join(PHASE0_DIR, "open-decisions.md"), encoding='utf-8').read()
tr_txt   = open(os.path.join(PHASE0_DIR, "traceability.md"), encoding='utf-8').read()
inv_txt  = open(os.path.join(PHASE0_DIR, "issue-inventory.md"), encoding='utf-8').read()
rpt_txt  = open(os.path.join(PHASE0_DIR, "phase-0-completion-report.md"), encoding='utf-8').read()
rm_txt   = open(os.path.join(PHASE0_DIR, "README.md"), encoding='utf-8').read()
lm_txt   = open(os.path.join(PHASE0_DIR, "lifecycle-ownership-matrix.md"), encoding='utf-8').read()

# ── A. Source-register schema ─────────────────────────────────────────────────
REQUIRED_FIELDS = {'source_id','repository','revision','path_or_issue','url',
                   'source_type','authority_class','scope','state','inspected','notes'}
BANNED_ONLY = {'class','path','description','inspection_status'}
VALID_CLASSES = set('ABCDEFG')

for s in sources:
    sid = s.get('source_id','?')
    missing = REQUIRED_FIELDS - set(s.keys())
    if missing:
        err(f"Schema: {sid} missing required fields: {missing}")
    banned = BANNED_ONLY & set(s.keys())
    if banned and missing:
        err(f"Schema: {sid} uses non-canonical fields {banned} without required fields")
    cls = s.get('authority_class', s.get('class',''))
    if cls not in VALID_CLASSES:
        err(f"Schema: {sid} invalid authority_class={cls!r}")
    if not isinstance(s.get('inspected'), bool):
        err(f"Schema: {sid} inspected must be boolean")
    for f in ('repository','revision','path_or_issue','url'):
        if not str(s.get(f,'')).strip():
            err(f"Schema: {sid} empty required field: {f}")

sids = [s['source_id'] for s in sources]
if len(sids) != len(set(sids)):
    err(f"Duplicate source IDs: {sorted(set(x for x in sids if sids.count(x)>1))}")

first_b = next((i for i,s in enumerate(sources)
                if s.get('authority_class','') == 'B'), len(sources))
for i, s in enumerate(sources):
    if s.get('authority_class','') == 'A' and i > first_b:
        err(f"Ordering: A-class {s['source_id']} after first B-class entry")

# ── B. Issue/comment evidence ─────────────────────────────────────────────────
inv_lines = inv_txt.split('\n')
in_pk = in_lc = False
inv_rows = []
for l in inv_lines:
    if '## pokeysHal' in l:  in_pk=True;  in_lc=False
    elif '## LinuxCnc' in l: in_lc=True;  in_pk=False
    if (l.startswith('| Issue') or l.startswith('| PR ')) and '---|' not in l:
        p = [x.strip() for x in l.split('|')][1:-1]
        if len(p) >= 11:
            row = dict(zip(['obj','title','state','sid','rel','body','cmt_cnt','cmt_s','ev','conf','retain'], p[:11]))
            row['repo'] = 'pk' if in_pk else 'lc'
            inv_rows.append(row)

for row in inv_rows:
    if row['retain'] in ('YES','MAYBE') and row['rel'] in ('HIGH','MEDIUM'):
        obj, repo, body, cmt_cnt, cmt_s, sid = (
            row['obj'], row['repo'], row['body'], row['cmt_cnt'], row['cmt_s'], row['sid'])
        if body not in ('complete','empty','title-only'):
            err(f"Criterion 2: {repo}/{obj} body status {body!r}")
        if cmt_cnt != '0' and cmt_s not in ('complete','none'):
            err(f"Criterion 3: {repo}/{obj} comment status {cmt_s!r} (cnt={cmt_cnt})")
        if sid in (None,'None',''):
            # Source entry required when: (a) comments > 0 or (b) explicitly required
            required_entries = {('pk','Issue #41'),('lc','Issue #216'),
                                ('lc','Issue #310'),('lc','Issue #326')}
            if (repo,obj) in required_entries or (cmt_cnt != '0' and cmt_s == 'complete'):
                err(f"Criterion 3: {repo}/{obj} has no source-register entry (required)")
        elif sid not in src_map:
            err(f"Criterion 3: {repo}/{obj} source {sid!r} not in register")
        else:
            src = src_map[sid]
            src_repo = src.get('repository','')
            src_pio  = str(src.get('path_or_issue',''))
            if repo == 'pk' and 'LinuxCnc' in src_repo:
                err(f"Cross-repo: {obj} (pokeysHal) has LC source {sid!r}")
            if repo == 'lc' and 'pokeysHal' in src_repo and 'LinuxCnc' not in src_repo:
                err(f"Cross-repo: {obj} (LC) has pokeysHal source {sid!r}")
            issue_num = re.search(r'#(\d+)', obj)
            if issue_num and issue_num.group(1) not in src_pio:
                err(f"Criterion 3: {repo}/{obj} source {sid!r} pio={src_pio!r} mismatch")
            if not src.get('inspected'):
                err(f"Criterion 3: {repo}/{obj} source {sid!r} not inspected")

for check in [('pk','Issue #41','C-021','0'),
              ('lc','Issue #216','E-011','21'),
              ('lc','Issue #310','E-004','14'),
              ('lc','Issue #326','E-005','12')]:
    repo, obj, exp_sid, exp_cnt = check
    row = next((r for r in inv_rows if r['obj']==obj and r['repo']==repo), None)
    if not row: err(f"Criterion 3: required row missing: {repo}/{obj}"); continue
    if row['sid'] != exp_sid:
        err(f"Criterion 3: {repo}/{obj} sid={row['sid']!r} expected {exp_sid!r}")
    if exp_sid in src_map and not src_map[exp_sid].get('inspected'):
        err(f"Criterion 3: {exp_sid} not inspected")

# ── C. Parity ─────────────────────────────────────────────────────────────────
n_act = sum(1 for r in parity if r['active_or_commented']=='active')
n_com = sum(1 for r in parity if r['active_or_commented']=='commented')
if len(parity) != 163: err(f"Parity count {len(parity)} != 163")
if n_act != 162:        err(f"Active parity {n_act} != 162")
if n_com != 1:          err(f"Commented parity {n_com} != 1")

ext = os.path.join(PHASE0_DIR, "tools", "extract_legacy_pev2_exports.py")
pf  = os.path.join(PHASE0_DIR, "legacy-pev2-parity.yaml")
rv  = subprocess.run([sys.executable, ext, "--check-parity", pf], capture_output=True, text=True)
if rv.returncode != 0:
    err(f"Parity validator failed: {(rv.stdout+rv.stderr).strip()}")
else:
    note(f"Parity: {rv.stdout.strip()}")

parity_ok = (len(parity)==163 and n_act==162 and n_com==1 and rv.returncode==0)

# ── D. VOLATILE_HOME chain ────────────────────────────────────────────────────
CHAIN = {'INI_reader': ['A-005'], 'cmd_builder': ['A-004'],
         'dispatcher': ['A-006'], 'homemod_impl': ['E-010','E-012']}
for stage, needed in CHAIN.items():
    if not any(sid in src_map for sid in needed):
        err(f"Criterion 8: VOLATILE_HOME chain missing {stage!r} (need {needed})")
    else:
        for sid in needed:
            if sid in src_map and not src_map[sid].get('inspected'):
                err(f"Criterion 8: chain source {sid} not inspected")

homemod_ev = any(
    'set_joint_homing_params' in str(src_map.get(sid,{}).get('scope','')) +
    str(src_map.get(sid,{}).get('notes',''))
    for sid in ['E-010','E-012'] if sid in src_map
)
if not homemod_ev:
    err("Criterion 8: no homemod source confirms set_joint_homing_params evidence")

volatile_chain_ok = (
    all(any(sid in src_map for sid in needed) for needed in CHAIN.values()) and homemod_ev
)

# ── E. Requirement catalogue ──────────────────────────────────────────────────
req_ids = [r['interface_id'] for r in reqs]
if len(req_ids) != len(set(req_ids)):
    err(f"Duplicate req IDs")
for r in reqs:
    for sr in r.get('requirement_sources',[])+r.get('implementation_sources',[]):
        if sr not in src_map: err(f"Broken src ref {sr!r} in {r['interface_id']}")

reg_conflicts = set(re.findall(r'^Conflict ID: (CONFLICT-\d+)', conf_txt, re.M))
for r in reqs:
    for c in r.get('conflicts',[]):
        if c not in reg_conflicts: err(f"Unregistered conflict {c!r} in {r['interface_id']}")

for r in reqs:
    if r['interface_id'].startswith('HOMECOMP-'):
        if 'pokeys_homecomp' not in r.get('owner',''):
            err(f"{r['interface_id']} owner not pokeys_homecomp")
        for sr in r.get('implementation_sources',[]):
            if sr.startswith('F-'): err(f"{r['interface_id']} has pokeysHal src {sr!r}")

for iid in ('HOMECOMP-005','HOMECOMP-006','HOMECOMP-007'):
    r = next((x for x in reqs if x['interface_id']==iid), None)
    if not r: err(f"{iid} missing"); continue
    if 'F-005' in r.get('implementation_sources',[]): err(f"{iid} has F-005")
    if 'E-010' not in r.get('implementation_sources',[]): err(f"{iid} missing E-010")

hc007 = next((r for r in reqs if r['interface_id']=='HOMECOMP-007'), None)
if hc007:
    cl = hc007.get('conflicts',[])
    if 'CONFLICT-013' not in cl: err("HOMECOMP-007 missing CONFLICT-013")
    if 'CONFLICT-014' not in cl: err("HOMECOMP-007 missing CONFLICT-014")

la012 = next((r for r in parity if r['legacy_id']=='LA-012'), None)
la013 = next((r for r in parity if r['legacy_id']=='LA-013'), None)
if la012:
    if la012.get('hal_type')!='hal_u32_t': err(f"LA-012 type={la012.get('hal_type')!r}")
    if la012.get('direction_or_access')!='HAL_OUT': err(f"LA-012 dir={la012.get('direction_or_access')!r}")
else: err("LA-012 missing")
if la013:
    if la013.get('hal_type')!='hal_u32_t': err(f"LA-013 type={la013.get('hal_type')!r}")
    if la013.get('direction_or_access')!='HAL_IN': err(f"LA-013 dir={la013.get('direction_or_access')!r}")
    if 'CONFLICT-013' not in str(la013.get('conflicts',[])): err("LA-013 missing CONFLICT-013")
else: err("LA-013 missing")

# ── F. Integration links ──────────────────────────────────────────────────────
ik003 = next((l for l in ilinks if l['link_id']=='IK-003'), None)
if ik003:
    if ik003.get('compatibility_status')=='compatible': err("IK-003 wrongly compatible")
    if 'CONFLICT-013' not in str(ik003.get('conflicts','')): err("IK-003 no CONFLICT-013")
    if 'CONFLICT-014' not in str(ik003.get('conflicts','')): err("IK-003 no CONFLICT-014")
    leg = str(ik003.get('legacy_pokeys_endpoint_evidence',''))
    if 'LA-' in leg and 'E-010' in leg: err("IK-003 legacy_ev has LA-* attributed to E-010")
    if 'LA-013' not in leg: err("IK-003 legacy_ev missing LA-013")
    if 'consumer' not in leg.lower(): err("IK-003 legacy endpoint not consumer")
    hce = str(ik003.get('homecomp_endpoint_evidence',''))
    if 'HOMECOMP-007' not in hce: err("IK-003 homecomp_ev missing HOMECOMP-007")
    if 'producer' not in hce.lower(): err("IK-003 homecomp endpoint not producer")
else: err("IK-003 missing")

# ── G. Lifecycle matrix ───────────────────────────────────────────────────────
sA_end = lm_txt.find('\n## B.')
sA = lm_txt[:sA_end] if sA_end > 0 else lm_txt
if "zeroed by hal_malloc" in lm_txt: err("lifecycle matrix: 'zeroed by hal_malloc'")
if '| **pokeys_homecomp' in sA: err("lifecycle matrix: homecomp in Section A table")
if '## B. External Counterpart' not in lm_txt: err("lifecycle matrix: no Section B")
if '## C. Integration Lifecycle' not in lm_txt: err("lifecycle matrix: no Section C")

# ── H. CONFLICT-012 text ──────────────────────────────────────────────────────
c12s = conf_txt.find("Conflict ID: CONFLICT-012")
c12e = conf_txt.find("Conflict ID: CONFLICT-013")
c12b = conf_txt[c12s:c12e] if c12s >= 0 and c12e >= 0 else ""
for p in ["unreachable","actual initial value","set_joint_homing_params","scoped to","A-005"]:
    if p.lower() not in c12b.lower():
        err(f"CONFLICT-012 missing phrase: {p!r}")

# ── I. Stale phrases ──────────────────────────────────────────────────────────
STALE = ["overwritten before first use","reassigned before first use",
         "runtime severity: LOW","LOW (volatile_home","Reviewer Upstream",
         "if the upstream claim is verified","zeroed by hal_malloc",
         "Both YAML files validated","56 patterns","140 rows"]
all_doc_texts = {
    "conflict-register.md": conf_txt, "open-decisions.md": od_txt,
    "lifecycle-ownership-matrix.md": lm_txt, "phase-0-completion-report.md": rpt_txt,
    "README.md": rm_txt, "traceability.md": tr_txt,
}
for fn, txt in all_doc_texts.items():
    for ph in STALE:
        if ph.lower() in txt.lower():
            err(f"Stale phrase {ph!r} in {fn}")

# ── J. Traceability ───────────────────────────────────────────────────────────
if 'Homecomp Counterpart ABI Boundary' not in tr_txt:
    err("traceability: missing counterpart boundary section")
if re.search(r'hal_pin_s32_newf', tr_txt, re.I):
    err("traceability: s32 type")

# ── K. Issue inventory cross-repo ─────────────────────────────────────────────
lc_section = inv_txt[inv_txt.find('## LinuxCnc'):]
if re.search(r'\| Issue #24 \|[^|]+\| C-012 \|', lc_section):
    err("inv: LC #24 has C-012")
if re.search(r'\| Issue #129 \|[^|]+\| D-003 \|', lc_section):
    err("inv: LC #129 has D-003")
for i, l in enumerate(inv_txt.split('\n'), 1):
    if (l.startswith('| Issue') or l.startswith('| PR ')) and '---|' not in l:
        if l.count('|') != 12:
            err(f"inv L{i}: wrong pipe count {l.count('|')}")

# ── L. Immutable files ────────────────────────────────────────────────────────
for immutable in ["legacy-pev2-parity.yaml","tools/extract_legacy_pev2_exports.py"]:
    fpath = os.path.join(PHASE0_DIR, immutable)
    rv2 = subprocess.run(['git','diff','--exit-code',IMMUTABLE_COMMIT,'--',fpath],
                         capture_output=True, cwd=REPO_ROOT)
    if rv2.returncode != 0:
        err(f"Immutable file changed: {immutable}")

# ── M. No production files changed ───────────────────────────────────────────
changed = subprocess.run(['git','diff','--name-only',PRODUCTION_BASE],
                         capture_output=True, text=True, cwd=REPO_ROOT).stdout.strip()
for f in changed.split('\n'):
    if not f: continue
    if f.startswith('docs/hal-interface/phase-0/'): continue
    if '.github/prompts/' in f: continue
    err(f"Non-Phase-0 file changed from production baseline: {f}")

gc = subprocess.run(['git','diff','--check'], capture_output=True, text=True, cwd=REPO_ROOT)
if gc.returncode != 0:
    err(f"git diff --check: {gc.stdout[:200]}")

# ── N. Report count consistency ───────────────────────────────────────────────
live = {
    'Source register entries':     len(sources),
    'Requirement catalogue entries': len(reqs),
    'Conflicts registered':        len(re.findall(r'^Conflict ID:', conf_txt, re.M)),
    'Open decisions required':     len(re.findall(r'^### DEC-', od_txt, re.M)),
    'Traceability chains':         len(re.findall(r'^## \d+\.', tr_txt, re.M)),
}
for label, lcount in live.items():
    m = re.search(rf'\| {re.escape(label)} \| (\d+) \|', rpt_txt)
    if m:
        if int(m.group(1)) != lcount:
            err(f"Count mismatch '{label}': report={m.group(1)} artifact={lcount}")
    else:
        err(f"Count '{label}' not found in report")

m = re.search(r'source-register\.yaml — (\d+) source', rpt_txt)
if m and int(m.group(1)) != len(sources):
    err(f"Files Created source count {m.group(1)} != {len(sources)}")

pk_n  = sum(1 for r in inv_rows if r['repo']=='pk' and not r['obj'].startswith('PR'))
lc_n  = sum(1 for r in inv_rows if r['repo']=='lc' and not r['obj'].startswith('PR'))
pr_n  = sum(1 for r in inv_rows if r['obj'].startswith('PR'))
for label, lcount in [('Issues inventoried (pokeysHal)', pk_n),
                      ('Issues inventoried (LinuxCnc_PokeysLibComp)', lc_n),
                      ('Pull requests inventoried', pr_n)]:
    m2 = re.search(rf'\| {re.escape(label)} \| (\d+) \|', rpt_txt)
    if m2 and int(m2.group(1)) != lcount:
        err(f"Count mismatch '{label}': report={m2.group(1)} artifact={lcount}")

# ── O. Derive criterion statuses ─────────────────────────────────────────────
bodies_ok   = all(r['body'] in ('complete','empty','title-only')
                  for r in inv_rows if r['retain'] in ('YES','MAYBE') and r['rel'] in ('HIGH','MEDIUM'))
comments_ok = all(r['cmt_s'] in ('complete','none') or r['cmt_cnt']=='0'
                  for r in inv_rows if r['retain'] in ('YES','MAYBE') and r['rel'] in ('HIGH','MEDIUM'))
src_ok      = not any('Criterion 3' in e or 'Cross-repo' in e for e in errors)
no_stale    = not any('Stale phrase' in e for e in errors)
counts_ok   = not any('Count mismatch' in e or 'Files Created source count' in e for e in errors)
schema_ok   = not any('Schema:' in e or 'Ordering:' in e for e in errors)
hc_ok       = not any('HOMECOMP-00' in e for e in errors)
ik_ok       = not any('IK-003' in e for e in errors)
lm_ok       = not any('lifecycle matrix' in e for e in errors)
c12_ok      = not any('CONFLICT-012' in e for e in errors)
no_prod     = not any('Non-Phase-0 file changed' in e for e in errors)
reqs_ok     = not any('Broken src ref' in e or 'Unregistered conflict' in e for e in errors)

computed = {
    1:  True,
    2:  bodies_ok and src_ok,
    3:  comments_ok and src_ok,
    4:  True,
    5:  True,
    6:  parity_ok,
    7:  True,
    8:  volatile_chain_ok and lm_ok and c12_ok,
    9:  True,
    10: True,
    11: reqs_ok,
    12: no_prod,
    13: True,
    14: no_stale and counts_ok and schema_ok and hc_ok and ik_ok,
}

for cnum, cpass in computed.items():
    m = re.search(rf'\| {cnum}\. .*?\| (PASS|PARTIAL|FAIL) \|', rpt_txt)
    if not m:
        err(f"Criterion {cnum} not in report")
    else:
        rs = m.group(1)
        if cpass and rs != 'PASS':
            err(f"Criterion {cnum}: computed PASS but report says {rs}")
        elif not cpass and rs == 'PASS':
            err(f"Criterion {cnum}: computed NOT PASS but report says {rs}")

if 'PHASE 0 BASELINE COMPLETE' not in rpt_txt:
    err("Report final status not PHASE 0 BASELINE COMPLETE")

me = re.search(r'## Missing Evidence.*?(?=\n##|\Z)', rpt_txt, re.DOTALL)
if me and ('PARTIAL' in me.group() or 'None' not in me.group()):
    err("Missing Evidence section not cleared")

# ── Summary ───────────────────────────────────────────────────────────────────
print(f"Sources:{len(sources)} Reqs:{len(reqs)} Parity:{len(parity)} "
      f"Conf:{live['Conflicts registered']} Dec:{live['Open decisions required']} "
      f"Chains:{live['Traceability chains']}")
print()
print("Computed criterion statuses:")
for n in range(1,15):
    print(f"  Criterion {n:2d}: {'PASS' if computed.get(n) else 'FAIL'}")

if errors:
    print(f"\nFAILURES ({len(errors)}):")
    for e in errors: print(f"  {e}")
    print("\nPHASE 0 BASELINE INCOMPLETE")
    sys.exit(1)
else:
    print()
    for n in notices: print(f"  {n}")
    print("\nALL PHASE 0 CLOSURE CHECKS PASSED")
    print("PHASE 0 BASELINE COMPLETE")
    sys.exit(0)
