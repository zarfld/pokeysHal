#!/usr/bin/env python3
"""Document-only scanner for current PoKeys-side HAL export sites.

Scans the current implementation tree for:
- hal_pin_*_newf / hal_param_*_newf calls
- hal_export_digin / hal_export_digout / hal_export_adcin / hal_export_adcout
- generated consumer pin declarations in experimental/pokeys_async.comp

Output: JSON inventory for validator use.
"""
import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]
PHASE0 = ROOT / "docs" / "hal-interface" / "phase-0"
RELEVANT_FILES = [
    ROOT / "experimental" / "pokeys_async.c",
    ROOT / "experimental" / "pokeys_async.comp",
]

for path in sorted(ROOT.glob("PoKeysLib*.c")):
    RELEVANT_FILES.append(path)
for path in sorted(ROOT.glob("PoKeysLib*.h")):
    RELEVANT_FILES.append(path)
for path in sorted(ROOT.glob("hal-canon/*.c")):
    RELEVANT_FILES.append(path)

inventory = []
seen = set()

def emit(kind, file_rel, line_no, detail, extra=None):
    key = (kind, str(file_rel), line_no, detail)
    if key in seen:
        return
    seen.add(key)
    row = {
        "kind": kind,
        "file": str(file_rel),
        "line": line_no,
        "detail": detail,
    }
    if extra:
        row.update(extra)
    inventory.append(row)

for path in RELEVANT_FILES:
    if not path.exists():
        continue
    rel = path.relative_to(ROOT)
    try:
        text = path.read_text(encoding="utf-8")
    except Exception:
        continue
    lines = text.splitlines()
    for lineno, line in enumerate(lines, 1):
        stripped = line.strip()
        if not stripped:
            continue
        if "hal_pin_" in stripped and "_newf" in stripped:
            m = re.search(r"hal_(pin|param)_(\w+)_newf", stripped)
            if m:
                kind = "hal_pin_or_param_newf"
                detail = f"{m.group(0)}"
                emit(kind, rel, lineno, detail, {"function_name": m.group(0)})
        if "hal_export_digin" in stripped or "hal_export_digout" in stripped or "hal_export_adcin" in stripped or "hal_export_adcout" in stripped:
            emit("hal_export_helper", rel, lineno, stripped)
        if "hal_export_funct" in stripped:
            emit("hal_export_funct", rel, lineno, stripped)

# Parse generated consumer pins from experimental/pokeys_async.comp.
comp_path = ROOT / "experimental" / "pokeys_async.comp"
if comp_path.exists():
    text = comp_path.read_text(encoding="utf-8")
    for lineno, line in enumerate(text.splitlines(), 1):
        if re.match(r"\s*(pin|param|function)\b", line):
            emit("comp_decl", comp_path.relative_to(ROOT), lineno, line.strip())

print(json.dumps({"inventory": inventory, "count": len(inventory)}, indent=2))
