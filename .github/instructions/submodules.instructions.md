---
description: "PoKeysHal submodule constraints: pin immutable commits, no branch tracking, inspect compatibility before pointer changes."
applyTo: ".gitmodules,pokeyslib/**,hal-canon/**"
---

# Submodule Constraints

- Do not edit files inside `pokeyslib/` or `hal-canon/` as though they belong to this repository. Changes to submodule content must go through the upstream repository.
- Pin submodules to immutable commit SHAs. Do not use branch-following state (`branch = main` in `.gitmodules`) as the committed reference.
- Inspect the upstream delta before changing a submodule pointer. Review the changelog, API changes, and any breaking behavior relative to the currently pinned commit.
- Test the parent repository build and userspace/RT smoke tests after bumping a submodule pointer and before merging.

For the full update/bump procedure, use the `update-submodule` skill (`.github/skills/update-submodule/SKILL.md`).

## Known Gap

The build CI workflow runs `git submodule update --remote` which mutates the pinned SHA. This is accepted as an early-warning mechanism for upstream compatibility issues. Do not remove it without providing a PR-based bump workflow as a replacement.
