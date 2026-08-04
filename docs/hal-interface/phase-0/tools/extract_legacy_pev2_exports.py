#!/usr/bin/env python3
"""
Deterministic extractor for hal_pin_*_newf and hal_param_*_newf calls.
Input : zarfld/LinuxCnc_PokeysLibComp @ 0c058e6c, pokeys_uspace/PokeysCompPulsEngine_base.c
Output: source fingerprints; validation against legacy-pev2-parity.yaml

Expected counts: 162 active, 1 commented, 163 total.
"""
import re, sys, json, subprocess, base64, hashlib

REPO = "zarfld/LinuxCnc_PokeysLibComp"
REV  = "0c058e6c7136ddb28a6b9f463a8af3d973496246"
FILE = "pokeys_uspace/PokeysCompPulsEngine_base.c"

def fetch():
    r = subprocess.run(
        ["gh", "api", f"repos/{REPO}/contents/{FILE}?ref={REV}"],
        capture_output=True, text=True, check=True)
    return base64.b64decode(json.loads(r.stdout)["content"]).decode()

def scan(src):
    """
    Two-pass scanner:
    1. Track block-comment state per line.
    2. Join multi-line calls (look ahead up to 4 lines for format string).
    """
    lines = src.split("\n")

    # Pass 1: determine comment state at start of each line
    in_block = False
    line_in_block = []
    for line in lines:
        line_in_block.append(in_block)
        j = 0
        while j < len(line):
            if not in_block:
                if line[j:j+2] == "/*": in_block = True; j += 2; continue
                if line[j:j+2] == "//": break
            else:
                if line[j:j+2] == "*/": in_block = False; j += 2; continue
            j += 1

    results = []
    for i, line in enumerate(lines):
        s = line.strip()
        # Detect call start
        if not (("hal_pin_" in s or "hal_param_" in s) and "newf" in s):
            continue
        m = re.search(r'hal_(pin|param)_(\w+)_newf', s)
        if not m:
            continue
        fn  = f"hal_{m.group(1)}_{m.group(2)}_newf"
        act = "commented" if line_in_block[i] else "active"
        lineno = i + 1

        # Search for format string on this or next few lines
        fmts = None
        for j in range(i, min(i + 5, len(lines))):
            fm = re.search(r'"([^"]*%s[^"]*)"', lines[j])
            if fm:
                fmts = fm.group(0)
                break
        if fmts is None:
            fmts = '"(multiline-format-not-found)"'

        raw = f"{act}|{fn}|{fmts}|{lineno}"
        results.append({
            "active_state": act,
            "function_name": fn,
            "format_string": fmts,
            "source_line": lineno,
            "fingerprint": hashlib.sha256(raw.encode()).hexdigest()[:16],
        })
    return results

def main():
    src = fetch()
    calls = scan(src)
    active    = [c for c in calls if c["active_state"] == "active"]
    commented = [c for c in calls if c["active_state"] == "commented"]
    print(f"Total:{len(calls)}  Active:{len(active)}  Commented:{len(commented)}")
    if len(calls) != 163:
        print(f"WARNING: Expected 163 calls, found {len(calls)}")
    if len(active) != 162:
        print(f"WARNING: Expected 162 active, found {len(active)}")
    if len(commented) != 1:
        print(f"WARNING: Expected 1 commented, found {len(commented)}")
        for c in commented:
            print(f"  Commented: L{c['source_line']} {c['format_string']}")

    if "--dump" in sys.argv:
        for c in calls:
            print(f"L{c['source_line']:4d} [{c['active_state'][0]}] {c['format_string']}")

    if "--check-parity" in sys.argv:
        import yaml
        idx = sys.argv.index("--check-parity")
        parity = yaml.safe_load(open(sys.argv[idx+1]).read())["parity_rows"]
        src_lines = {c["source_line"] for c in calls}
        par_lines = {r["source_line"] for r in parity}
        missing = src_lines - par_lines
        extra   = par_lines - src_lines
        if missing: print(f"Lines missing from parity ({len(missing)}): {sorted(missing)}")
        if extra:   print(f"Extra lines in parity ({len(extra)}): {sorted(extra)}")
        if not missing and not extra:
            print("Source-line sets IDENTICAL — parity complete (163 lines)")
        # Check active/commented agreement
        act_src = {c["source_line"] for c in calls if c["active_state"]=="active"}
        act_par = {r["source_line"] for r in parity if r["active_or_commented"]=="active"}
        com_src = {c["source_line"] for c in calls if c["active_state"]=="commented"}
        com_par = {r["source_line"] for r in parity if r["active_or_commented"]=="commented"}
        if act_src != act_par: print(f"active_state mismatch: {act_src.symmetric_difference(act_par)}")
        if com_src != com_par: print(f"commented mismatch: src={com_src} par={com_par}")
        if act_src == act_par and com_src == com_par:
            print("Active/commented classification AGREES")

if __name__ == "__main__":
    main()
