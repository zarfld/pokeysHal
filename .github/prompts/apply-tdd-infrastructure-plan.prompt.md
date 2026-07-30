---
name: apply-tdd-infrastructure-plan
description: >
  Implement one approved PoKeysHal TDD/Copilot infrastructure issue while excluding
  hardware operation and HIL test implementation.
argument-hint: 'issue=<approved GitHub issue number>'
agent: 'agent'
tools: ['read', 'search', 'edit', 'execute', 'github/*']
---

# Task

Implement only the approved infrastructure issue:

${input:issue:Enter the approved GitHub issue number}

## Required preparation

1. Read the issue and all acceptance criteria.
2. Read:
   - `docs/HIL_Setup_4_TDD/HIL_TDD_synthesis.md`;
   - the approved infrastructure audit/plan;
   - current repository instructions;
   - every customization file affected by the issue.
3. Establish the current baseline.
4. Confirm the requested phase does not include hardware operation or HIL test execution.
5. Create or use the issue-specific branch; never modify `main` directly.

## Scope rules

- Make the smallest complete infrastructure change.
- Do not add HIL HAL files, fixture scripts, workflows, or hardware tests unless the
  issue explicitly authorizes that later phase.
- Do not copy raw thought-document content into active instructions.
- Preserve one authoritative location per rule.
- Preserve existing line endings unless normalization is explicitly in scope.
- Avoid unrelated prompt cleanup.
- Do not replace unresolved hardware questions with guessed values.

## Customization rules

- Prompt files use current `.prompt.md` frontmatter (`agent`, not `mode`).
- Prompt files do not use `applyTo`.
- Path-specific instructions use scalar `applyTo`.
- Agents contain role/tool/routing guidance, not complete procedures.
- Skills contain detailed repeatable procedures.
- Documentation records rationale and decisions.
- Mandatory safety or evidence invariants remain in appropriately scoped instructions.

## Verification

Run and report:

1. frontmatter parsing for all changed customization files;
2. search for obsolete file names and routing references;
3. search for generic web/auth/database examples in rewritten PoKeys TDD files;
4. line counts before and after;
5. `git diff --check`;
6. `git diff --ignore-space-at-eol`;
7. exact changed-file list;
8. any available Copilot customization discovery check.

Do not run production build, hardware, RT, or timing tests unless executable code changed.

## Completion report

Report:

- issue implemented;
- files changed;
- ownership changes between instruction/prompt/skill/agent/documentation;
- validation commands and exact results;
- checks not performed;
- deferred HIL phases;
- remaining unresolved hardware facts.

Use `Implemented` and `configuration-validated` precisely. Do not claim
`HIL-tested`, `hardware-verified`, `RT-validated`, or `timing-validated`.