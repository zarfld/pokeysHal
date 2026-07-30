---
name: update-submodule
description: >
  Procedure for safely bumping a PoKeysHal submodule pointer (pokeyslib or hal-canon).
  Use when updating a pinned submodule SHA to a newer upstream commit.
---

# Update Submodule Skill

## When to use this skill

Use when intentionally moving a submodule pointer to a newer upstream commit. Do not run `git submodule update --remote` and commit the result without following this procedure.

## Procedure

### 1. Inspect the currently pinned commit

```bash
git submodule status
# Note the current SHA for the submodule being updated
```

### 2. Fetch candidate upstream changes

```bash
cd <submodule-dir>
git fetch origin
git log HEAD..origin/main --oneline   # or relevant upstream branch
```

### 3. Select an immutable commit

- Choose a specific commit SHA — not a branch tip.
- Prefer a tagged release if the upstream uses tags.
- Read the upstream changelog for the range being advanced.

### 4. Review the upstream delta

- Check for API changes in headers that affect `PoKeysLibAsync.h` or `PoKeysLibHal.h`.
- Check for protocol changes that affect command codes, response layouts, or behavior.
- Note any breaking changes for the bump commit message.

### 5. Update the pointer

```bash
cd /path/to/parent-repo
git -C <submodule-dir> checkout <chosen-sha>
git add <submodule-dir>
```

### 6. Build and test the parent repository

```bash
# Build library
sudo make -f Makefile.noqmake install

# Userspace smoke test
halrun <<EOF
loadusr -W pokeys_async
show pin && show funct && start && show pin && show param && exit
EOF

# Compile test
bash test_compile.sh
```

Record the exact build/test results.

### 7. Document compatibility

Commit message must include:
- Old SHA → new SHA
- Summary of upstream changes reviewed
- Build/test result (Compiled / Tested / Hardware-verified)

### 8. Create a dedicated bump PR

Open a PR with the sole change being the submodule pointer update. Do not mix submodule bumps with production code changes.

### 9. Never branch-follow

Do not set `branch = <name>` in `.gitmodules` as a substitute for explicit SHA selection. The committed submodule reference must always be a specific, immutable SHA.
