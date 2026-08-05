---
description: >
  Mandatory routing and invariants for HAL-interface compatibility work that can
  affect pokeysHal HAL contracts, compatibility claims, or HAL-facing verification.
applyTo: "**/*.c,**/*.h,**/*.comp,**/*.hal,**/*.ini,**/tests/**,**/test_*.c,**/*_test.c"
---

# HAL-Interface Compatibility Routing Contract

Use this instruction when work may affect PoKeys-side HAL exports, HAL-facing
structures, compatibility expectations, or HAL integration behavior.

## Mandatory Routing

Invoke the skill:
- `.github/skills/hal-interface-compatibility/SKILL.md`

before implementation or test design when a task may change or rely on:
- `hal_pin_*` exports;
- `hal_param_*` exports;
- `hal_export_*` helpers;
- `hal_export_funct` registration;
- `.comp` pin, parameter, or function declarations;
- HAL-visible structures or HAL-visible member typing;
- object names or component prefix conventions;
- HAL type or direction/access;
- cardinality or conditional creation rules;
- units, scaling, or default/initial behavior claims;
- enum or bitmap interpretation;
- lifecycle-related HAL behavior;
- PoKeys-side HAL-to-device or device-to-HAL propagation;
- HAL/INI compatibility claims.

## Required Invariants

For HAL-interface-impacting tasks, you must:
- identify affected Phase 0 interface IDs before coding or test design;
- separate authoritative, legacy, current-observed, and target contracts;
- inspect linked conflicts and open decisions before behavior-changing work;
- record unresolved evidence gaps explicitly instead of guessing;
- keep HAL compatibility and async protocol parity as separate streams.

## Prohibitions

You must not:
- silently resolve contract conflicts without a recorded decision;
- derive normative requirements from issue titles alone;
- treat examples as normative sources over higher authority;
- treat maintenance tooling output as normative authority;
- classify external counterpart behavior as pokeysHal implementation scope;
- claim compatibility without naming evidence and verification layer.

## Scope Boundary

In scope:
- pokeysHal library and RT `pokeys.comp` replacement behavior;
- PoKeys-side HAL pins, parameters, functions, naming, typing, cardinality,
  creation conditions, scaling/default semantics, lifecycle, and propagation;
- compatibility with legacy `pokeys.comp` exports.

External evidence only:
- HAL/INI net examples;
- `joint.N.*` counterpart objects;
- `pokeys_homecomp` endpoint observations.

Out of scope:
- `pokeys_homecomp` implementation or lifecycle internals;
- homecomp local state and internal read/write behavior;
- `joint.N.*` objects as pokeysHal exports.

## Authority Handling

Follow the precedence defined in the HAL-interface compatibility skill.
Use Phase 0 artifact paths as indexed evidence references; do not copy catalogue
rows into implementation notes, tests, prompts, or instructions.

## HIL Routing

When hardware-dependent verification is required, route through:
- `.github/skills/hil-tdd/SKILL.md`
- `.github/skills/hil-tdd/references/result-schema.md`

Do not redefine HIL status terminology in this instruction.
