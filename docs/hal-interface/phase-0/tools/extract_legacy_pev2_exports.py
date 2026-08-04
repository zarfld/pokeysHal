#!/usr/bin/env python3
"""
Deterministic extractor for hal_pin_*_newf and hal_param_*_newf calls.
Input : zarfld/LinuxCnc_PokeysLibComp @ 0c058e6c, pokeys_uspace/PokeysCompPulsEngine_base.c
Output: source fingerprints; exact-tuple validation against legacy-pev2-parity.yaml

Expected pinned-source result:
    162 active  +  1 commented  =  163 total
    exact source fingerprint set == exact parity fingerprint set

Exit codes:
    0  all checks passed
    1  count mismatch, missing call, extra call, duplicate fingerprint,
       function mismatch, direction mismatch, target-field mismatch,
       format-string mismatch, active/commented mismatch, placeholder found
"""
import argparse, re, sys, json, subprocess, base64, hashlib

REPO = "zarfld/LinuxCnc_PokeysLibComp"
REV  = "0c058e6c7136ddb28a6b9f463a8af3d973496246"
FILE = "pokeys_uspace/PokeysCompPulsEngine_base.c"

EXPECTED_TOTAL    = 163
EXPECTED_ACTIVE   = 162
EXPECTED_COMMENTED = 1

PLACEHOLDER_MARKERS = ("no-fmt", "multiline-format-not-found", "(no-fmt")


def fetch_source():
    r = subprocess.run(
        ["gh", "api", f"repos/{REPO}/contents/{FILE}?ref={REV}"],
        capture_output=True, text=True, check=True,
    )
    data = json.loads(r.stdout)
    return base64.b64decode(data["content"]).decode()


def scan(src):
    """
    Line-by-line block-comment state machine.
    For each hal_*_newf call line extract the 6-tuple fingerprint fields.
    """
    lines = src.split("\n")
    in_block = False
    results = []

    for lineno, line in enumerate(lines, 1):
        j = 0
        was_in_block = in_block
        while j < len(line):
            if not in_block:
                if line[j:j+2] == "/*": in_block = True;  j += 2; continue
                if line[j:j+2] == "//": break
            else:
                if line[j:j+2] == "*/": in_block = False; j += 2; continue
            j += 1

        s = line.strip()
        if not (("hal_pin_" in s or "hal_param_" in s) and "newf" in s):
            continue

        # Extract function name
        m_fn = re.search(r"hal_(pin|param)_(\w+)_newf", s)
        if not m_fn:
            continue
        function_name = f"hal_{m_fn.group(1)}_{m_fn.group(2)}_newf"

        # Extract direction and target field from the call opening
        m_call = re.search(
            r"hal_(?:pin|param)_\w+_newf\s*\(\s*([^,]+),\s*(&[^,]+),", s
        )
        direction = m_call.group(1).strip() if m_call else "?"
        target    = m_call.group(2).strip() if m_call else "?"

        # Format string: search this line and up to 4 continuation lines
        fmt = None
        for look in range(lineno - 1, min(lineno + 4, len(lines))):
            fm = re.search(r'"([^"]*%s[^"]*)"', lines[look])
            if fm:
                fmt = fm.group(0)
                break
        if fmt is None:
            fmt = f'"(no-fmt-L{lineno})"'

        active_state = "commented" if was_in_block else "active"

        results.append({
            "source_line":            lineno,
            "active_or_commented":    active_state,
            "function_name":          function_name,
            "direction_or_access":    direction,
            "target_field_expression": target,
            "format_string":          fmt,
        })

    return results


def fingerprint(rec):
    """Stable SHA-256-based fingerprint over normalized 6-tuple."""
    raw = "|".join([
        rec["active_or_commented"],
        rec["function_name"],
        rec["direction_or_access"],
        rec["target_field_expression"],
        rec["format_string"],
        str(rec["source_line"]),
    ])
    return hashlib.sha256(raw.encode()).hexdigest()[:20]


def validate(src_calls, parity_rows, strict=True):
    """
    Compare source fingerprints to parity fingerprints.
    Returns list of error strings; empty list = PASS.
    """
    errors = []

    # Check for placeholder format strings in parity
    for row in parity_rows:
        fmt = str(row.get("format_string", ""))
        if any(p in fmt for p in PLACEHOLDER_MARKERS):
            errors.append(f"PLACEHOLDER in parity row {row.get('legacy_id','?')}: {fmt!r}")

    # Build fingerprint maps
    src_fps  = {fingerprint(c): c for c in src_calls}
    par_fps  = {}
    for row in parity_rows:
        fp = fingerprint(row)
        if fp in par_fps:
            errors.append(f"DUPLICATE fingerprint in parity: {fp} ({row.get('legacy_id','?')})")
        par_fps[fp] = row

    # Count checks
    if len(src_calls) != EXPECTED_TOTAL:
        errors.append(f"COUNT: expected {EXPECTED_TOTAL} total, got {len(src_calls)}")
    n_active    = sum(1 for c in src_calls if c["active_or_commented"] == "active")
    n_commented = sum(1 for c in src_calls if c["active_or_commented"] == "commented")
    if n_active != EXPECTED_ACTIVE:
        errors.append(f"COUNT: expected {EXPECTED_ACTIVE} active, got {n_active}")
    if n_commented != EXPECTED_COMMENTED:
        errors.append(f"COUNT: expected {EXPECTED_COMMENTED} commented, got {n_commented}")

    # Set equality
    missing_from_parity = set(src_fps) - set(par_fps)
    extra_in_parity     = set(par_fps) - set(src_fps)
    for fp in sorted(missing_from_parity):
        c = src_fps[fp]
        errors.append(
            f"MISSING from parity: L{c['source_line']} [{c['active_or_commented']}]"
            f" {c['function_name']} {c['format_string']}"
        )
    for fp in sorted(extra_in_parity):
        r = par_fps[fp]
        errors.append(
            f"EXTRA in parity: {r.get('legacy_id','?')} L{r.get('source_line','?')}"
            f" {r.get('format_string','?')}"
        )

    return errors


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--dump", action="store_true", help="Print all extracted calls")
    parser.add_argument("--check-parity", metavar="YAML", help="Validate against parity YAML")
    args = parser.parse_args()

    src = fetch_source()
    calls = scan(src)

    active    = sum(1 for c in calls if c["active_or_commented"] == "active")
    commented = sum(1 for c in calls if c["active_or_commented"] == "commented")
    print(f"Total:{len(calls)}  Active:{active}  Commented:{commented}")

    if args.dump:
        for c in calls:
            fp = fingerprint(c)
            print(f"L{c['source_line']:4d} [{c['active_or_commented'][0]}]"
                  f" {c['function_name']} {c['direction_or_access']} {c['format_string']}")

    if args.check_parity:
        import yaml as _yaml
        parity = _yaml.safe_load(open(args.check_parity).read())["parity_rows"]
        errors = validate(calls, parity)
        if errors:
            print(f"\nVALIDATION FAILED ({len(errors)} error(s)):")
            for e in errors:
                print(f"  {e}")
            sys.exit(1)
        else:
            print(f"VALIDATION PASSED — exact tuple equality confirmed ({len(calls)} fingerprints)")

    # Always validate counts as a basic sanity check
    count_errors = []
    if len(calls) != EXPECTED_TOTAL:
        count_errors.append(f"Expected {EXPECTED_TOTAL} calls, got {len(calls)}")
    if active != EXPECTED_ACTIVE:
        count_errors.append(f"Expected {EXPECTED_ACTIVE} active, got {active}")
    if counted := commented != EXPECTED_COMMENTED:
        count_errors.append(f"Expected {EXPECTED_COMMENTED} commented, got {commented}")
    if count_errors and not args.check_parity:
        for e in count_errors:
            print(f"ERROR: {e}")
        sys.exit(1)


if __name__ == "__main__":
    main()
