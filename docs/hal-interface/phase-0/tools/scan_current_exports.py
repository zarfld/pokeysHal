#!/usr/bin/env python3
"""Extract current PoKeys-side HAL export objects from the implementation tree.

The scanner is source-driven and emits one row per concrete HAL object created
by the current helper implementations and direct export calls that map to the
Phase 0 requirement catalogue.
"""
import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[4]
RELEVANT_FILES = [ROOT / "experimental" / "pokeys_async.c", ROOT / "experimental" / "pokeys_async.comp"]
for path in sorted(ROOT.glob("PoKeysLibIOAsync.c")):
    RELEVANT_FILES.append(path)
for path in sorted(ROOT.glob("PoKeysLibPulseEngine_v2Async.c")):
    RELEVANT_FILES.append(path)
for path in sorted(ROOT.glob("hal-canon/*.c")):
    RELEVANT_FILES.append(path)

inventory = []
seen = set()


def emit(row):
    key = (row.get("kind"), row.get("definition_file"), row.get("definition_line"), row.get("normalized_name"))
    if key in seen:
        return
    seen.add(key)
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


def infer_cardinality(normalized_name, helper_name=None):
    name = normalized_name.lower()
    if "<i>.axescommand" in name or "<i>.index-enable" in name:
        return "always 8"
    if helper_name in {"hal_export_digin", "hal_export_digout", "hal_export_adcin", "hal_export_adcout"}:
        return "conditional"
    if "<i>." in name or "<j>." in name:
        return "conditional"
    return "1 per component instance"


def pattern_to_normalized_name(name_pattern):
    name_pattern = str(name_pattern).strip().strip('"')
    if not name_pattern:
        return ""
    name_pattern = name_pattern.replace("%s", "<prefix>")
    name_pattern = re.sub(r'%\d*[du]', '<j>', name_pattern)
    return name_pattern


def classify_catalogue_ids(normalized_name, call_name, helper_name=None):
    text = normalized_name.lower()
    if text.endswith("digin.<j>.in"):
        return ["DIGIN-001"]
    if text.endswith("digin.<j>.in-not"):
        return ["DIGIN-002"]
    if text.endswith("digout.<j>.out"):
        return ["DIGOUT-001"]
    if text.endswith("digout.<j>.invert"):
        return ["DIGOUT-002"]
    if text.endswith("adcin.<j>.in.hw"):
        return ["ADCIN-001"]
    if text.endswith("adcin.<j>.in.raw"):
        return ["ADCIN-002"]
    if text.endswith("adcin.<j>.value"):
        return ["ADCIN-003"]
    if text.endswith("adcin.<j>.scale"):
        return ["ADCIN-004"]
    if text.endswith("adcin.<j>.offset"):
        return ["ADCIN-006"]
    if text.endswith("adcin.<j>.bit-weight"):
        return ["ADCIN-007"]
    if text.endswith("adcin.<j>.hw-offset"):
        return ["ADCIN-008"]
    if text.endswith("adcin.<j>.referencevoltage"):
        return ["ADCIN-005"]
    if text.endswith("adcout.<j>.pwmduty"):
        return ["ADCOUT-001"]
    if text.endswith("adcout.<j>.max_voltage"):
        return ["ADCOUT-002"]
    if text.endswith("adcout.<j>.pwm.period"):
        return ["ADCOUT-003"]
    if text.endswith("adcout.<j>.value"):
        return ["ADCOUT-004"]
    if text.endswith("adcout.<j>.enable"):
        return ["ADCOUT-006"]
    if text.endswith("adcout.<j>.offset"):
        return ["ADCOUT-007"]
    if text.endswith("adcout.<j>.scale"):
        return ["ADCOUT-008"]
    if text.endswith("adcout.<j>.high-limit"):
        return ["ADCOUT-009"]
    if text.endswith("adcout.<j>.low-limit"):
        return ["ADCOUT-010"]
    if text.endswith("adcout.<j>.bit-weight"):
        return ["ADCOUT-011"]
    if text.endswith("adcout.<j>.hw-offset"):
        return ["ADCOUT-012"]
    if "axescommand" in text:
        return ["PEV2A-006"]
    if "index-enable" in text:
        return ["PEV2A-007"]
    if helper_name == "hal_export_funct" or call_name == "hal_export_funct":
        return ["LIFE-003"]
    return []


def parse_helper_body(text, start_idx, helper_name, definition_file, definition_line, call_site_file=None):
    brace_open = text.find("{", start_idx)
    if brace_open < 0:
        return
    depth = 0
    body_text = None
    for idx in range(brace_open, len(text)):
        if text[idx] == "{":
            depth += 1
        elif text[idx] == "}":
            depth -= 1
            if depth == 0:
                body_text = text[brace_open + 1:idx]
                break
    if body_text is None:
        body_text = text[brace_open + 1:]

    lines = body_text.splitlines()
    for i, line in enumerate(lines):
        if "rtapi_snprintf" not in line:
            continue
        name_match = re.search(r'"([^"]+)"', line)
        if not name_match:
            continue
        name_pattern = name_match.group(1)
        for j in range(i + 1, min(i + 4, len(lines))):
            next_line = lines[j]
            call_match = re.search(r"\b(hal_(?:pin|param)_[a-z0-9_]+_newf)\s*\(", next_line)
            if not call_match:
                continue
            call_name = call_match.group(1)
            call_text = "\n".join(lines[i:j + 1])
            normalized_name = pattern_to_normalized_name(name_pattern)
            catalogue_ids = classify_catalogue_ids(normalized_name, call_name, helper_name)
            if not catalogue_ids:
                break
            emit({
                "kind": "actual_export_object",
                "object_kind": "parameter" if call_name.startswith("hal_param_") else "pin",
                "hal_type": infer_hal_type(call_name),
                "direction_or_access": infer_direction(call_text, call_name),
                "cardinality": infer_cardinality(normalized_name, helper_name),
                "normalized_name": normalized_name,
                "catalogue_ids": catalogue_ids,
                "definition_file": str(definition_file.relative_to(ROOT)).replace('\\', '/'),
                "definition_line": definition_line,
                "call_site_file": str(call_site_file.relative_to(ROOT)).replace('\\', '/') if call_site_file else None,
                "helper_name": helper_name,
                "actual_or_normative": "actual",
                "unresolved": False,
            })
            break


def scan_helper_definitions(path):
    text = path.read_text(encoding="utf-8")
    for helper_name in ["hal_export_digin", "hal_export_digout", "hal_export_adcin", "hal_export_adcout"]:
        pattern = re.compile(rf"int\s+{re.escape(helper_name)}\s*\(")
        for match in pattern.finditer(text):
            start = match.start()
            line_no = text.count("\n", 0, start) + 1
            parse_helper_body(text, start, helper_name, path, line_no)


def scan_direct_calls(path):
    text = path.read_text(encoding="utf-8")
    for match in re.finditer(r"\b(hal_(?:pin|param)_[a-z0-9_]+_newf)\s*\(", text):
        call_name = match.group(1)
        line_no = text.count("\n", 0, match.start()) + 1
        call_text = text[match.start():match.end() + 300]
        name_match = re.search(r'"([^"]+)"', call_text)
        normalized_name = pattern_to_normalized_name(name_match.group(1)) if name_match else ""
        catalogue_ids = classify_catalogue_ids(normalized_name, call_name)
        if not catalogue_ids:
            continue
        emit({
            "kind": "actual_export_object",
            "object_kind": "parameter" if call_name.startswith("hal_param_") else "pin",
            "hal_type": infer_hal_type(call_name),
            "direction_or_access": infer_direction(call_text, call_name),
            "cardinality": infer_cardinality(normalized_name),
            "normalized_name": normalized_name,
            "catalogue_ids": catalogue_ids,
            "definition_file": str(path.relative_to(ROOT)).replace('\\', '/'),
            "definition_line": line_no,
            "call_site_file": str(path.relative_to(ROOT)).replace('\\', '/'),
            "actual_or_normative": "actual",
            "unresolved": False,
        })


def scan_function_exports(path):
    text = path.read_text(encoding="utf-8")
    for match in re.finditer(r"\bhal_export_funct\s*\(", text):
        line_no = text.count("\n", 0, match.start()) + 1
        emit({
            "kind": "actual_export_object",
            "object_kind": "function",
            "hal_type": "n/a",
            "direction_or_access": "n/a",
            "cardinality": "1 per component instance",
            "normalized_name": "<prefix>",
            "catalogue_ids": ["LIFE-003"],
            "definition_file": str(path.relative_to(ROOT)).replace('\\', '/'),
            "definition_line": line_no,
            "call_site_file": str(path.relative_to(ROOT)).replace('\\', '/'),
            "helper_name": "hal_export_funct",
            "actual_or_normative": "actual",
            "unresolved": False,
        })


for path in RELEVANT_FILES:
    if not path.exists():
        continue
    rel = path.relative_to(ROOT)
    if path.name == "pokeys_async.comp":
        continue
    if path.parent.name == "hal-canon":
        scan_helper_definitions(path)
        continue
    if path.name == "pokeys_async.c":
        scan_function_exports(path)
    scan_direct_calls(path)

print(json.dumps({"inventory": inventory, "count": len(inventory)}, indent=2))
