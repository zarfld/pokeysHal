#!/usr/bin/env python3
"""Document-only scanner for current PoKeys-side HAL export sites.

Scans the current implementation tree for:
- hal_pin_*_newf / hal_param_*_newf calls
- hal_export_digin / hal_export_digout / hal_export_adcin / hal_export_adcout
- generated consumer pin declarations in experimental/pokeys_async.comp
- hal_export_funct

Output: JSON inventory for validator use.
"""
import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[4]
RELEVANT_FILES = [ROOT / "experimental" / "pokeys_async.c", ROOT / "experimental" / "pokeys_async.comp"]
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
    row = {"kind": kind, "file": str(file_rel), "line": line_no, "detail": detail}
    if extra:
        row.update(extra)
    inventory.append(row)


def infer_hal_type(call_name):
    if call_name.startswith("hal_pin_bit") or call_name.startswith("hal_param_bit"):
        return "hal_bit_t"
    if call_name.startswith("hal_pin_u32") or call_name.startswith("hal_param_u32"):
        return "hal_u32_t"
    if call_name.startswith("hal_pin_s32") or call_name.startswith("hal_param_s32"):
        return "hal_s32_t"
    if call_name.startswith("hal_pin_float") or call_name.startswith("hal_param_float"):
        return "hal_float_t"
    return "unknown"


def infer_direction(call_text, call_name):
    m = re.search(r"\b(HAL_(?:IN|OUT|IO|RW|RO))\b", call_text)
    if m:
        return m.group(1)
    if call_name.startswith("hal_param_"):
        return "HAL_RW"
    return "unknown"


def infer_cardinality(call_text):
    if "for (int i = 0; i < 8; i++)" in call_text or "for (i = 0; i < 8; i++)" in call_text:
        return "always 8"
    if "for (int i = 0; i < 8" in call_text:
        return "always 8"
    if "for (int i = 0; i <" in call_text:
        return "loop-bound"
    return "1 per component instance"


def classify_catalogue_ids(name_expr, call_name, detail):
    text = f"{name_expr} {detail}".lower()
    if "devserial" in text:
        return ["COMP-001"]
    if "alive" in text:
        return ["COMP-002"]
    if "err" in text and "comp" not in text:
        return ["COMP-003"]
    if "pinfunction" in text:
        return ["PINFN-001"]
    if "invert" in text:
        return ["PINFN-002"]
    if "axescommand" in text:
        return ["PEV2A-006"]
    if "index-enable" in text or "index_enable" in text:
        return ["PEV2A-007"]
    if "axesstate" in text:
        return ["PEV2A-005"]
    if "nrofaxes" in text or "nrOfAxes" in text:
        return ["PEV2A-004"]
    if "pulseengineactivated" in text or "pulseenginestate" in text:
        return ["PEV2A-003"]
    if "referencevoltage" in text:
        return ["ADCIN-005"]
    if "in.raw" in text:
        return ["ADCIN-003"]
    if "in.hw" in text:
        return ["ADCIN-004"]
    if "max_voltage" in text:
        return ["ADCOUT-003"]
    if "pwmduty" in text:
        return ["ADCOUT-004"]
    if "pwm.period" in text:
        return ["ADCOUT-002"]
    if call_name.startswith("hal_pin") or call_name.startswith("hal_param"):
        if "adcin" in text:
            return ["ADCIN-001"]
        if "adcout" in text:
            return ["ADCOUT-001"]
        if "digin" in text:
            return ["DIGIN-001"]
        if "digout" in text:
            return ["DIGOUT-001"]
    if "hal_export_funct" in detail:
        return ["LIFE-003"]
    return []


def scan_calls(text, rel_path, pattern, kind):
    for match in pattern.finditer(text):
        call_text = match.group(1).strip()
        line_no = text.count("\n", 0, match.start(1)) + 1
        detail = text[max(0, match.start(1) - 220): match.end(1) + 400]
        if kind == "hal_export_helper":
            name = call_text.split("(", 1)[0]
            if name.endswith("digin"):
                emit(kind, rel_path, line_no, detail, {
                    "object_kind": "pin",
                    "catalogue_ids": ["DIGIN-001", "DIGIN-002"],
                    "hal_type": "hal_bit_t",
                    "direction_or_access": "HAL_OUT",
                    "cardinality": "conditional",
                    "normalized_name": "digin.<J>.in / digin.in-not",
                })
            elif name.endswith("digout"):
                emit(kind, rel_path, line_no, detail, {
                    "object_kind": "pin",
                    "catalogue_ids": ["DIGOUT-001", "DIGOUT-002"],
                    "hal_type": "hal_bit_t",
                    "direction_or_access": "HAL_IN",
                    "cardinality": "conditional",
                    "normalized_name": "digout.<J>.out / invert",
                })
            elif name.endswith("adcin"):
                emit(kind, rel_path, line_no, detail, {
                    "object_kind": "pin",
                    "catalogue_ids": ["ADCIN-001", "ADCIN-002", "ADCIN-003", "ADCIN-004", "ADCIN-005"],
                    "hal_type": "hal_float_t",
                    "direction_or_access": "HAL_OUT",
                    "cardinality": "conditional",
                    "normalized_name": "adcin.<J>.*",
                })
            elif name.endswith("adcout"):
                emit(kind, rel_path, line_no, detail, {
                    "object_kind": "pin",
                    "catalogue_ids": ["ADCOUT-001", "ADCOUT-002", "ADCOUT-003", "ADCOUT-004"],
                    "hal_type": "hal_float_t",
                    "direction_or_access": "HAL_IN",
                    "cardinality": "conditional",
                    "normalized_name": "adcout.<J>.*",
                })
            elif name.endswith("funct"):
                emit("hal_export_funct", rel_path, line_no, detail, {
                    "object_kind": "function",
                    "catalogue_ids": ["LIFE-003"],
                    "hal_type": "n/a",
                    "direction_or_access": "n/a",
                    "cardinality": "1 per component instance",
                    "normalized_name": "<prefix>",
                })
            else:
                emit(kind, rel_path, line_no, detail, {"object_kind": "pin", "catalogue_ids": [], "unresolved": True})
        elif kind == "hal_newf":
            call_name = call_text.split("(", 1)[0]
            name_expr = ""
            quoted = re.search(r'("[^"]+")', call_text)
            if quoted:
                name_expr = quoted.group(1).strip('"')
            catalogue_ids = classify_catalogue_ids(name_expr, call_name, detail)
            dynamic_name = bool(re.search(r"[%]s|%d|%u|prefix|\{", name_expr))
            emit(kind, rel_path, line_no, detail, {
                "object_kind": "parameter" if call_name.startswith("hal_param_") else "pin",
                "catalogue_ids": catalogue_ids,
                "hal_type": infer_hal_type(call_name),
                "direction_or_access": infer_direction(call_text, call_name),
                "cardinality": infer_cardinality(call_text),
                "normalized_name": name_expr or call_name,
                "dynamic_name": dynamic_name,
                "unresolved": not catalogue_ids and dynamic_name,
            })


for path in RELEVANT_FILES:
    if not path.exists():
        continue
    rel = path.relative_to(ROOT)
    try:
        text = path.read_text(encoding="utf-8")
    except Exception:
        continue

    if path.name == "pokeys_async.comp":
        for lineno, line in enumerate(text.splitlines(), 1):
            if re.match(r"\s*(pin|param|function)\b", line):
                emit("comp_decl", rel, lineno, line.strip())
        continue

    scan_calls(text, rel, re.compile(r"(hal_export_(?:digin|digout|adcin|adcout|funct)\s*\([^;]*?\)\s*;)", re.S), "hal_export_helper")
    scan_calls(text, rel, re.compile(r"(hal_(?:pin|param)_[a-z0-9_]+_newf\s*\([^;]*?\)\s*;)", re.S), "hal_newf")

print(json.dumps({"inventory": inventory, "count": len(inventory)}, indent=2))
