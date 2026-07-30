---
description: "README documentation invariants for PoKeysHal: document implemented behavior only, update existing content, verify commands."
applyTo: "**/README.md,**/readme.md,**/Readme.md"
---

# README Documentation Rules

- Document only implemented and verified behavior. Do not describe planned or speculative features.
- Update existing authoritative sections rather than creating parallel files or duplicate content.
- Verify every command and code block against the actual build/run environment before adding or editing.
- Use the README as an entry point with navigation to deeper documentation; do not make it the complete design document.
- Preserve links to related documentation; validate that referenced files exist before committing.

For a full README rewrite or structural overhaul, use the `update-readme` skill (`.github/skills/update-readme/SKILL.md`).
