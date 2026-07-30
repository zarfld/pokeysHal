---
name: update-readme
description: >
  Full README rewrite or structural overhaul for PoKeysHal. Use when significantly
  restructuring a README, adding new major sections, or bringing an outdated README
  up to date with the current implementation.
---

# Update README Skill

## When to use this skill

Use this skill when:
- The README needs a substantial restructure (not a single-section edit)
- A new subsystem or major feature requires its own documented section
- The README has become significantly out of date with the implementation
- Navigation or table of contents needs to be rebuilt

For small corrections (one section, one command), edit directly without invoking this skill.

## Procedure

### 1. Inspect before writing

- Read the current README fully before modifying anything.
- Read `docs/INDEX.md` (if present) for the authoritative documentation map.
- Identify which sections describe implemented vs planned behavior.

### 2. Verify all claims

Before writing or retaining any content:
- Build the library (`sudo make -f Makefile.noqmake install`) and confirm it succeeds.
- Run the userspace smoke test (`halrun` with `loadusr -W pokeys_async`) if hardware is available.
- Confirm every code block and command matches the actual current behavior.
- Mark hardware-dependent steps explicitly: "Requires connected PoKeys device."

### 3. Structure

A PoKeysHal README must contain, in this order:

1. **What it is** — one paragraph: PoKeysLibHal, LinuxCNC HAL driver, USB/Ethernet, C99.
2. **Prerequisites** — OS, LinuxCNC version, kernel (RTAI / PREEMPT_RT), libusb.
3. **Build** — exact commands from `Makefile.noqmake` and `halcompile`.
4. **Quick start** — minimal `halrun` session for userspace and RT.
5. **Repository structure** — table of key files and their roles.
6. **Further documentation** — links to `docs/`, architecture instructions, skills.

Do not include speculative roadmap, unimplemented features, or example output from sessions that were not actually executed.

### 4. Update rules

- Update existing sections in place; do not create `README_new.md` or similar parallel files.
- Retain existing anchors and section names where other documents link to them.
- Remove sections that describe behavior no longer present in the codebase.

### 5. Validate

- Confirm every statement is true of the current working tree. When the branch contains unmerged functionality, clearly distinguish branch behavior from the last released version.
