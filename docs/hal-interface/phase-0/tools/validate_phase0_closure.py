#!/usr/bin/env python3
"""
Phase 0 Closure Validator — library-scope edition
docs/hal-interface/phase-0/tools/validate_phase0_closure.py

Documentation-only validator. Not a compatibility test.
Independently derives acceptance criteria from artifacts.
Exits 0 iff all Phase 0 acceptance criteria are met.

Scope: pokeysHal LIBRARY interface baseline.
Out of scope: VOLATILE_HOME chain, Homecomp internals, LinuxCNC homemod policy.
"""
import sys, os, re, subprocess, yaml

PHASE0_DIR       = os.path.normpath(os.path.join(os.path.dirname(__file__), '..'))
REPO_ROOT        = os.path.normpath(os.path.join(PHASE0_DIR, '..', '..', '..'))
IMMUTABLE_COMMIT = '4f0ab5e'
PRODUCTION_BASE  = 'cd1f0dc8a0f64f92dc6bdce21bddcb36d33a14cd'

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

src_data = load_yaml("source-register.yaml")
req_data = load_yaml("requirement-catalogue.yaml")
par_data = load_yaml("legacy-pev2-parity.yaml")
lnk_data = load_yaml("integration-links.yaml")

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

# ── Criterion 1: Repository and revision identified ───────────────────────────
# Check that pinned repository revisions appear in the source register and/or report
has_pokeyshal_src  = any('pokeysHal' in s.get('repository','') for s in sources)
has_linuxcnc_src   = any('LinuxCNC' in s.get('repository','') for s in sources)
has_lc_pkcomp_src  = any('LinuxCnc_PokeysLibComp' in s.get('repository','') for s in sources)
computed_c1 = has_pokeyshal_src and has_linuxcnc_src and has_lc_pkcomp_src
if not has_pokeyshal_src:  err("C1: no pokeysHal sources in register")
if not has_linuxcnc_src:   err("C1: no LinuxCNC authority sources in register")
if not has_lc_pkcomp_src:  err("C1: no LinuxCnc_PokeysLibComp sources in register")

# ── Criterion 2: Issue bodies inspected ──────────────────────────────────────
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

bodies_ok = all(
    r['body'] in ('complete','empty','title-only')
    for r in inv_rows if r['retain'] in ('YES','MAYBE') and r['rel'] in ('HIGH','MEDIUM')
)
if not bodies_ok:
    for r in inv_rows:
        if r['retain'] in ('YES','MAYBE') and r['rel'] in ('HIGH','MEDIUM'):
            if r['body'] not in ('complete','empty','title-only'):
                err(f"C2: {r['repo']}/{r['obj']} body status {r['body']!r}")
computed_c2 = bodies_ok

# ── Criterion 3: Comment inspection ──────────────────────────────────────────
# Required dedicated source entries: only for issues with >0 comments
# Additional explicit requirements:
REQUIRED_SOURCES = [('pk','Issue #41','C-021'),
                    ('lc','Issue #216','E-011'),
                    ('lc','Issue #310','E-004'),
                    ('lc','Issue #326','E-005')]

comments_ok = True
for r in inv_rows:
    if r['retain'] in ('YES','MAYBE') and r['rel'] in ('HIGH','MEDIUM'):
        if r['cmt_cnt'] != '0' and r['cmt_s'] not in ('complete','none'):
            err(f"C3: {r['repo']}/{r['obj']} comment status {r['cmt_s']!r}")
            comments_ok = False

src_ok = True
for repo, obj, exp_sid in REQUIRED_SOURCES:
    row = next((r for r in inv_rows if r['obj']==obj and r['repo']==repo), None)
    if not row:
        err(f"C3: required row missing: {repo}/{obj}")
        src_ok = False
        continue
    if row['sid'] != exp_sid:
        err(f"C3: {repo}/{obj} sid={row['sid']!r} expected {exp_sid!r}")
        src_ok = False
    if exp_sid in src_map and not src_map[exp_sid].get('inspected'):
        err(f"C3: {exp_sid} not inspected")
        src_ok = False

# Cross-repo contamination
lc_section = inv_txt[inv_txt.find('## LinuxCnc'):]
if re.search(r'\| Issue #24 \|[^|]+\| C-012 \|', lc_section): err("C3: LC #24 has C-012"); src_ok=False
if re.search(r'\| Issue #129 \|[^|]+\| D-003 \|', lc_section): err("C3: LC #129 has D-003"); src_ok=False
computed_c3 = comments_ok and src_ok

# ── Criterion 4: Official LinuxCNC rules recorded ────────────────────────────
# Check A-class authority sources for HAL API and CDI exist and are inspected
hal_h_src  = any(s.get('source_id')=='A-001' and s.get('inspected') for s in sources)
cdi_src    = any(s.get('source_id')=='A-002' and s.get('inspected') for s in sources)
computed_c4 = hal_h_src and cdi_src
if not hal_h_src: err("C4: A-001 (LinuxCNC hal.h) missing or not inspected")
if not cdi_src:   err("C4: A-002 (CDI spec) missing or not inspected")

# ── Criterion 5: hal-canon provenance ────────────────────────────────────────
b_sources = [s for s in sources if s.get('authority_class','') == 'B']
has_halcanon = len(b_sources) >= 2 and any('homing.c' not in s.get('path_or_issue','') for s in b_sources)
# B-001 through B-005 should exist
b_ids = {s['source_id'] for s in b_sources}
for bid in ['B-001','B-002','B-003']:
    if bid not in b_ids: err(f"C5: {bid} missing from source register")
computed_c5 = all(bid in b_ids for bid in ['B-001','B-002','B-003'])

# ── Criterion 6: Parity extraction ───────────────────────────────────────────
n_act = sum(1 for r in parity if r['active_or_commented']=='active')
n_com = sum(1 for r in parity if r['active_or_commented']=='commented')
parity_count_ok = (len(parity)==163 and n_act==162 and n_com==1)
if not parity_count_ok:
    err(f"C6: parity count {len(parity)}/active {n_act}/commented {n_com}")

ext = os.path.join(PHASE0_DIR, "tools", "extract_legacy_pev2_exports.py")
pf  = os.path.join(PHASE0_DIR, "legacy-pev2-parity.yaml")
rv  = subprocess.run([sys.executable, ext, "--check-parity", pf], capture_output=True, text=True)
if rv.returncode != 0:
    err(f"C6: parity validator failed: {(rv.stdout+rv.stderr).strip()}")
else:
    note(f"Parity: {rv.stdout.strip()}")
computed_c6 = parity_count_ok and rv.returncode == 0

# Check no stale parity counts in source register or report
for fname, txt in [('source-register.yaml', open(os.path.join(PHASE0_DIR,"source-register.yaml")).read()),
                   ('phase-0-completion-report.md', rpt_txt)]:
    for stale in ['140 rows', '139 active', '56 patterns']:
        if stale in txt: err(f"C6: stale parity phrase {stale!r} in {fname}")

# ── Criterion 7: Current HAL interface extracted ──────────────────────────────
# Check that F-class sources for key implementation files exist
f_ids = {s['source_id'] for s in sources if s.get('authority_class','')=='F'}
required_f = ['F-001','F-002','F-003','F-005','F-008']
for fid in required_f:
    if fid not in f_ids: err(f"C7: {fid} missing from source register")
# Check requirement references resolve
reqs_ok = True
for r in reqs:
    for sr in r.get('requirement_sources',[])+r.get('implementation_sources',[]):
        if sr not in src_map:
            err(f"C7: broken src ref {sr!r} in {r['interface_id']}")
            reqs_ok = False
computed_c7 = all(fid in f_ids for fid in required_f) and reqs_ok

# ── Criterion 8: Library ownership, consumer boundaries, propagation ──────────
# Check lifecycle-ownership-matrix.md has Sections A, B, C
has_section_a = '## A. pokeysHal Component Lifecycle' in lm_txt or '## A. pokeysHal' in lm_txt
has_section_b = '## B. External Counterpart' in lm_txt
has_section_c = '## C. Integration Lifecycle' in lm_txt
# No homecomp row in section A table
sA_end = lm_txt.find('\n## B.')
sA = lm_txt[:sA_end] if sA_end > 0 else lm_txt
homecomp_in_a = '| **pokeys_homecomp' in sA or 'homecomp-owned pins' in sA.lower()
if homecomp_in_a: err("C8: homecomp lifecycle in Section A")

# Criterion 8 must NOT be based on VOLATILE_HOME or Homecomp internals
c8_text = ''
m8 = re.search(r'\| 8\. .*?\| PASS \|.*?\|.*?\|', rpt_txt, re.DOTALL)
if m8:
    c8_text = m8.group(0)
    banned_in_c8 = ['VOLATILE_HOME','volatile_home','HOMEMOD','set_unhomed','homing_init']
    for banned in banned_in_c8:
        if banned in c8_text: err(f"C8: report contains out-of-scope term {banned!r}")
else:
    err("C8: criterion 8 not found as PASS in report")

# Device→HAL and HAL→device ownership must appear in lifecycle matrix
has_device_hal = 'Device→HAL' in lm_txt or 'device.*hal.*update' in lm_txt.lower()
has_hal_device = 'HAL→device' in lm_txt or 'hal.*device.*update' in lm_txt.lower()
if not has_device_hal: err("C8: lifecycle matrix missing Device→HAL ownership")
if not has_hal_device: err("C8: lifecycle matrix missing HAL→device ownership")
if not has_section_a:  err("C8: lifecycle matrix missing Section A")
if not has_section_b:  err("C8: lifecycle matrix missing Section B")
if not has_section_c:  err("C8: lifecycle matrix missing Section C")

computed_c8 = (has_section_a and has_section_b and has_section_c and
               not homecomp_in_a and has_device_hal and has_hal_device)

# ── Criterion 9: Enumerations and bitmaps ────────────────────────────────────
# Check that key enum records exist in requirement-catalogue
enum_req_ids = [r['interface_id'] for r in reqs if r.get('enumeration') or r.get('bitmap_definition')]
has_pev2_enum = any('PEV2' in rid for rid in enum_req_ids) or any(
    'ePK_PEAxisState' in str(r.get('enumeration','')) for r in reqs)
has_digin_req  = any(r['interface_id'].startswith('DIGIN') or 'digin' in r.get('name_pattern','').lower() for r in reqs)
computed_c9 = has_pev2_enum
if not has_pev2_enum: err("C9: no PEv2 enum records in requirement-catalogue")

# ── Criterion 10: Canonical vs project-specific ───────────────────────────────
canon_matrix_exists = os.path.exists(os.path.join(PHASE0_DIR, "canonical-vs-legacy-matrix.md"))
has_canonical_class = any('canonical' in str(r.get('linuxcnc_canonical_status','')).lower() for r in reqs)
computed_c10 = canon_matrix_exists or has_canonical_class
if not computed_c10: err("C10: no canonical classification evidence")

# ── Criterion 11: Contradictions registered ───────────────────────────────────
reg_conflicts = set(re.findall(r'^Conflict ID: (CONFLICT-\d+)', conf_txt, re.M))
# No conflict may be solely about VOLATILE_HOME
for cid in reg_conflicts:
    block_start = conf_txt.find(f"Conflict ID: {cid}")
    next_block  = conf_txt.find("Conflict ID: CONFLICT-", block_start + 1)
    block = conf_txt[block_start:next_block] if next_block > 0 else conf_txt[block_start:]
    subject_line = next((l for l in block.split('\n') if 'Subject:' in l), '')
    if ('volatile_home' in subject_line.lower() or 'set_unhomed' in subject_line.lower() or
        'VOLATILE_HOME' in subject_line):
        err(f"C11: {cid} subject is about VOLATILE_HOME/set_unhomed (out of scope)")

# All conflict references in reqs resolve
for r in reqs:
    for c in r.get('conflicts',[]):
        if c not in reg_conflicts:
            err(f"C11: unregistered conflict {c!r} in {r['interface_id']}")
computed_c11 = not any('C11' in e for e in errors)

# ── Criterion 12: No production code changed ──────────────────────────────────
changed = subprocess.run(['git','diff','--name-only',PRODUCTION_BASE],
                         capture_output=True, text=True, cwd=REPO_ROOT).stdout.strip()
no_prod = True
for f in changed.split('\n'):
    if not f: continue
    if f.startswith('docs/hal-interface/phase-0/'): continue
    if '.github/prompts/' in f: continue
    err(f"C12: non-Phase-0 file changed: {f}")
    no_prod = False
gc = subprocess.run(['git','diff','--check'], capture_output=True, text=True, cwd=REPO_ROOT)
if gc.returncode != 0:
    err(f"C12: git diff --check: {gc.stdout[:200]}")
    no_prod = False
computed_c12 = no_prod

# ── Criterion 13: No compatibility tests added ───────────────────────────────
# Check that no test specification or test artifact was added in the phase-0 docs
test_files = subprocess.run(
    ['git','diff','--name-only', PRODUCTION_BASE],
    capture_output=True, text=True, cwd=REPO_ROOT
).stdout.strip().split('\n')
test_artifacts_added = any(
    re.search(r'test.*\.py$|test.*\.c$|hil_.*', f, re.I) and 'tools/validate' not in f
    for f in test_files if f
)
if test_artifacts_added: err("C13: test artifacts added")
computed_c13 = not test_artifacts_added

# ── Criterion 14: No contradictions across documents ─────────────────────────
# Check scope consistency
BANNED_SCOPE = [
    ("pokeysHal is a HOMEMOD", "pokeysHal library wrongly described as HOMEMOD"),
    ("pokeysHal implements pokeys_homecomp", "pokeysHal implements homecomp"),
    ("pokeys_homecomp is a pokeysHal subsystem", "homecomp is a pokeysHal subsystem"),
    ("pokeysHal owns joint", "pokeysHal owns joint.N.* pins"),
]
all_docs = {
    "conflict-register.md": conf_txt, "open-decisions.md": od_txt,
    "lifecycle-ownership-matrix.md": lm_txt, "phase-0-completion-report.md": rpt_txt,
    "README.md": rm_txt, "traceability.md": tr_txt,
}
no_banned = True
for phrase, label in BANNED_SCOPE:
    for fn, txt in all_docs.items():
        if phrase.lower() in txt.lower():
            err(f"C14: {label} in {fn}")
            no_banned = False

# Stale phrases
STALE = ["overwritten before first use","reassigned before first use",
         "runtime severity: LOW","LOW (volatile_home","Reviewer Upstream",
         "if the upstream claim is verified","zeroed by hal_malloc",
         "Both YAML files validated","56 patterns","140 rows","139 active"]
no_stale = True
for fn, txt in all_docs.items():
    for ph in STALE:
        if ph.lower() in txt.lower():
            err(f"C14: stale phrase {ph!r} in {fn}")
            no_stale = False

# Count consistency
import yaml as _yaml
live_src = len(_yaml.safe_load(open(os.path.join(PHASE0_DIR,"source-register.yaml")).read())["sources"])
live = {
    'Source register entries':    live_src,
    'Requirement catalogue entries': len(reqs),
    'Conflicts registered':        len(re.findall(r'^Conflict ID:', conf_txt, re.M)),
    'Open decisions required':     len(re.findall(r'^### DEC-', od_txt, re.M)),
    'Traceability chains':         len(re.findall(r'^## \d+\.', tr_txt, re.M)),
}
counts_ok = True
for label, lcount in live.items():
    m = re.search(rf'\| {re.escape(label)} \| (\d+) \|', rpt_txt)
    if m:
        if int(m.group(1)) != lcount:
            err(f"C14: count mismatch '{label}': report={m.group(1)} artifact={lcount}")
            counts_ok = False
    else:
        err(f"C14: count '{label}' not found in report")
        counts_ok = False

# Stale parity counts in source register
sr_txt = open(os.path.join(PHASE0_DIR,"source-register.yaml")).read()
for stale in ['140 rows','139 active','56 patterns']:
    if stale in sr_txt:
        err(f"C14: stale {stale!r} in source-register.yaml")
        no_stale = False

# Source register IDs must be unique
sids = [s['source_id'] for s in sources]
if len(sids) != len(set(sids)):
    err("C14: duplicate source IDs")

computed_c14 = no_banned and no_stale and counts_ok

# ── Immutable files ───────────────────────────────────────────────────────────
for immutable in ["legacy-pev2-parity.yaml","tools/extract_legacy_pev2_exports.py"]:
    fpath = os.path.join(PHASE0_DIR, immutable)
    rv2 = subprocess.run(['git','diff','--exit-code',IMMUTABLE_COMMIT,'--',fpath],
                         capture_output=True, cwd=REPO_ROOT)
    if rv2.returncode != 0: err(f"Immutable changed: {immutable}")

# ── HOMECOMP-* scope ──────────────────────────────────────────────────────────
for r in reqs:
    if r['interface_id'].startswith('HOMECOMP-'):
        owner = r.get('owner','')
        if 'pokeys_homecomp' not in owner:
            err(f"C8: {r['interface_id']} owner not pokeys_homecomp: {owner!r}")
        for sr in r.get('implementation_sources',[]):
            if sr.startswith('F-'):
                err(f"C8: {r['interface_id']} has pokeysHal impl source {sr!r}")

# No active conflict is solely about VOLATILE_HOME
for cid in reg_conflicts:
    bs = conf_txt.find(f"Conflict ID: {cid}")
    ne = conf_txt.find("Conflict ID: CONFLICT-", bs+1)
    blk = conf_txt[bs:ne] if ne > 0 else conf_txt[bs:]
    if all(term in blk for term in ['volatile_home']) and 'AxesCommand' not in blk and 'digout' not in blk:
        err(f"C11/scope: {cid} appears to be solely about volatile_home (out-of-scope)")

# ── Criterion statuses ────────────────────────────────────────────────────────
computed = {
    1: computed_c1, 2: computed_c2, 3: computed_c3, 4: computed_c4,
    5: computed_c5, 6: computed_c6, 7: computed_c7, 8: computed_c8,
    9: computed_c9, 10: computed_c10, 11: computed_c11, 12: computed_c12,
    13: computed_c13, 14: computed_c14,
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
print(f"Sources:{live_src} Reqs:{len(reqs)} Parity:{len(parity)} "
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
