# Review result

I reviewed `main` at commit `5a7251c8c5cbbe413bdad2de65e59baff9b25be7`, produced by merged PR #134. The PR changed 51 files with 1,878 additions and 745 deletions.

**Verdict: the optimization was only partially completed.**

The discovery and naming repairs were mostly successful, and the repository-specific engineering contract is materially better. However, several configuration defects remain, the C-task context is still far larger than intended, and much of the generic content was moved rather than actually optimized.

## Objective-by-objective assessment

| Objective                             | Result | Assessment                                                            |
| ------------------------------------- | -----: | --------------------------------------------------------------------- |
| Reduce root instructions              |      ✅ | Reduced from 621 to approximately 127 lines                           |
| Narrow submodule instructions         |      ✅ | Correctly scoped to `.gitmodules`, `pokeyslib/**`, and `hal-canon/**` |
| Repair prompt directory               |      ✅ | Files moved from `.github/ptompts/` to `.github/prompts/`             |
| Repair skills                         |      ✅ | Lowercase directories, exact `SKILL.md`, required frontmatter         |
| Repair custom-agent filenames         |      ✅ | Renamed to `*.agent.md`                                               |
| Add cross-tool `AGENTS.md`            |      ✅ | Added with repository-specific operational rules                      |
| Operationalize engineering philosophy |      ✅ | Strong repository-specific contract added                             |
| Reduce context for C work             |      ❌ | Still roughly 1,120 lines of overlapping instructions                 |
| Repair test instructions              |      ❌ | Frontmatter is syntactically invalid                                  |
| Make agents operational               |      ❌ | Several declared tool names are unrecognized                          |
| Clean prompt content                  |     ⚠️ | Directory fixed, but malformed and generic prompts remain             |
| Replace generic test examples         |      ❌ | Web/login/API/SQL examples remain                                     |
| Add mechanical enforcement            |      ❌ | No instruction/configuration validation was added                     |
| Follow the new process in PR #134     |      ❌ | PR scope, traceability and evidence contradicted the new rules        |

# Blocking findings

## 1. `tests.instructions.md` has invalid YAML

The frontmatter now contains:

```yaml
applyTo: "**/tests/**,...,test_compile.sh"
  - "**/test_*.py"
```

The second line is a dangling YAML list item with no key. The instruction file may therefore fail to parse and may not load at all.

This problem was also reported by Copilot review after the PR had already been merged, and the review thread remains unresolved.

It should be:

```yaml
---
description: "PoKeysHal C and RT test requirements"
applyTo: "**/tests/**,**/test_*.c,**/*_test.c,test_compile.sh"
---
```

Python, JavaScript and TypeScript patterns should be removed unless those technologies are genuinely part of this repository.

## 2. The custom agents contain invalid or ineffective tool names

For example, `TDDDriver` declares:

```yaml
tools: ["read", "edit", "githubRepo", "runCommands"]
```

Current GitHub aliases include `read`, `edit`, `search`, `execute`, `agent`, `web`, `todo`, and namespaced MCP tools such as `github/*`. Unrecognized tool names are ignored. Consequently:

* `runCommands` does not grant command execution; use `execute`.
* `githubRepo` does not grant GitHub access; use `github/*` or appropriate `github/<tool>`.
* Most agents also omit `search`.
* TDD and testing agents may be unable to execute the tests they are instructed to run. ([GitHub Docs][1])

A suitable TDD configuration would be:

```yaml
tools: ["read", "search", "edit", "execute", "github/*"]
```

All six agent profiles should be reviewed, because `githubRepo` occurs across the agent set.

# Main optimization failure: C context remains excessive

For an ordinary C file, the following can all apply:

1. `.github/copilot-instructions.md` — approximately 127 lines.
2. `AGENTS.md` — approximately 153 lines.
3. `engineering-discipline.instructions.md` — approximately 185 lines.
4. `c-realtime.instructions.md` — approximately 103 lines.
5. `pokeyshal-architecture.instructions.md` — approximately 555 lines.

That is approximately **1,120 lines before the source file, issue, user request, tool descriptions or conversation context**.

The root repeats architecture, RT constraints, engineering discipline and routing.  `AGENTS.md` repeats the same architecture, baseline, anti-shortcut, RT, evidence and completion rules.  The full engineering instruction repeats them again and applies to all C/H work.   `c-realtime.instructions.md` adds another architecture summary and convention list.  Finally, the architecture instruction still contains hundreds of lines of detailed architecture reference material.

The previous C-task estimate was approximately 1,416 lines. The new arrangement therefore reduces C context by only about 20%, not to the intended 150–250-line range.

## Recommended context architecture

### `.github/copilot-instructions.md` — 70–100 lines

Keep only:

* repository identity;
* authoritative build commands;
* five or six critical invariants;
* resource routing;
* concise completion/evidence rule.

Remove detailed architecture tables, traceability tutorials and duplicated RT rules.

### `AGENTS.md` — 70–100 lines

Make this the authoritative cross-tool operational contract:

* understand before modifying;
* baseline first;
* smallest complete change;
* no fake completion;
* evidence terminology;
* completion gate.

Do not repeat repository structure and build instructions already present in the Copilot root file.

### One C/RT instruction — 120–180 lines

Merge:

* `c-realtime.instructions.md`;
* the essential enforcement portion of `pokeyshal-architecture.instructions.md`;
* protocol and RT prohibitions from `engineering-discipline.instructions.md`.

Move diagrams, extended file descriptions, examples and migration procedures into documentation or the conversion skill.

### Detailed implementation procedures

Keep these in `convert-to-hal-rtapi/SKILL.md`, which is now structurally valid.  Skills are the correct mechanism for detailed task-specific procedures that should be loaded only when relevant. ([GitHub Docs][2])

# Other significant findings

## 3. Test instructions remain generic and factually inaccurate

The file still contains user-login, browser, REST API, database, SQL-injection, TypeScript and Python examples that have no meaningful relationship to a C/LinuxCNC/PoKeys driver.

It also asserts that GitHub Actions currently checks:

* requirement-to-test links;
* valid issue references;
* coverage above 80%;
* orphaned tests.

Those claims should not remain unless corresponding workflows actually exist. The previous audit explicitly identified these as textual rules rather than CI enforcement.

This file should instead contain PoKeys-specific examples such as:

* malformed 64-byte protocol response;
* command-ID mismatch;
* request-ID mismatch;
* timeout and retry exhaustion;
* mailbox reuse;
* HAL pin update after dispatch;
* disconnected-device behavior;
* RT versus userspace builds;
* timing measurements.

## 4. `tdd-compile.prompt.md` is malformed

The file begins with a four-backtick `prompt` fence before the YAML block:

`````markdown
````prompt
---
mode: agent
...
`````

Therefore, the YAML is not at the start of the file and will likely be interpreted as content rather than prompt metadata.

The opening fence and its corresponding closing fence should be removed.

## 5. Prompt content was relocated, not optimized

The directory typo was fixed, but many prompts were renamed without adapting their content. For example, the repository-audit prompt still assumes:

* `src/`;

* `tests/`;

* `package.json`;

* generic package managers;

* a reusable lifecycle template rather than the actual PoKeysHal structure.

Prompt files are manually invoked, so their size does not pollute every interaction. Nevertheless, a manually invoked prompt should still be accurate for the repository. ([GitHub Docs][3])

## 6. README instruction portability was not fully repaired

The filename is now correct, but the frontmatter still uses a YAML array:

```yaml
applyTo:
  - "**/README.md"
  - "**/readme.md"
  - "**/Readme.md"
```

GitHub’s documented portable form is a comma-separated scalar string. ([GitHub Docs][4])

Use:

```yaml
applyTo: "**/README.md,**/readme.md,**/Readme.md"
```

## 7. Documentation artifacts need cleanup

`docs/engineering-discipline.md` begins with embedded terminal escape/control sequences before its title.

`docs/tasks/philosophy_expl.md` begins as a response to a conversation—“Yes. My earlier recommendation…”—rather than as a standalone repository document. It also duplicates the engineering contract that now exists in `AGENTS.md` and `engineering-discipline.instructions.md`.

A cleaner arrangement would be:

* `docs/engineering-discipline.md`: standalone historical rationale.
* `AGENTS.md`: concise operational contract.
* Remove `docs/tasks/philosophy_expl.md`, or rewrite it as a short design decision explaining why the instructions were reorganized.
* Keep `docs/repo_review.md` as an explicitly labelled historical audit snapshot, because it describes commit `cf3902...`, not the current repository.

## 8. PR #134 violated the rules it introduced

The PR description says it adds two documentation files and updates the index, but the PR actually changed 51 files across instructions, prompts, skills and agents. The body also marks “No unrelated files changed” and “All tests pass,” while its testing section contains no executed test evidence.

The root instructions now state that all work must start with an issue and PRs without an issue link fail review.  PR #134 nevertheless declared requirements and traceability as `N/A`.

This is more than cosmetic: the repository’s first application of “No Shortcuts” did not comply with its own scope, traceability and evidence rules.

# Required corrective backlog

## P0 — configuration defects

1. Fix `tests.instructions.md` YAML.
2. Replace invalid custom-agent tool names.
3. Remove the erroneous fence from `tdd-compile.prompt.md`.
4. Remove the control characters from `docs/engineering-discipline.md`.

## P1 — complete the actual optimization

5. Consolidate the three C/RT instruction files into one compact path-specific instruction.
6. Eliminate duplication between root instructions and `AGENTS.md`.
7. Rewrite `tests.instructions.md` for C, HAL, protocol and RT verification.
8. Convert remaining list-form `applyTo` frontmatter to documented scalar strings.
9. Rewrite or remove the duplicated `philosophy_expl.md`.

## P2 — make the rules enforceable

Add a small CI validation script that checks:

* valid YAML frontmatter;
* documented `applyTo` form;
* recognized agent tool aliases;
* exact skill directory and `SKILL.md` naming;
* prompt files beginning with valid frontmatter rather than code fences;
* absence of `.github/ptompts`;
* duplicate or globally overlapping instruction scopes;
* claims about CI checks that have no corresponding workflow.

## Acceptance conclusion

The migration successfully fixed the **repository layout and discovery layer**, and the new engineering-discipline text captures the intended anti-shortcut behavior well.

It did **not** complete the more important second half of the optimization:

* reliable parsing;
* working agent capabilities;
* repository-specific test guidance;
* reduced C-task context;
* elimination of duplicated rules;
* mechanical enforcement.

Therefore, the current state should be classified as **implemented but only partially verified**, not complete.

[1]: https://docs.github.com/en/copilot/reference/custom-agents-configuration?utm_source=chatgpt.com "Custom agents configuration - GitHub Docs"
[2]: https://docs.github.com/en/copilot/reference/customization-cheat-sheet?utm_source=chatgpt.com "Copilot customization cheat sheet - GitHub Docs"
[3]: https://docs.github.com/en/copilot/concepts/prompting/response-customization?utm_source=chatgpt.com "About customizing GitHub Copilot responses - GitHub Docs"
[4]: https://docs.github.com/en/copilot/how-tos/copilot-cli/customize-copilot/add-custom-instructions?utm_source=chatgpt.com "Adding custom instructions for GitHub Copilot CLI - GitHub Docs"
