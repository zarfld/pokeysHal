Yes. My earlier recommendation to move most of this philosophy into ordinary documentation was too broad.

The **rationale and generic examples** can move out of permanent context, but the **operational consequences must remain always loaded**. These principles were introduced to prevent recurring agent failure modes, so reducing them to a vague sentence such as “report assumptions honestly” would remove an important safeguard.

The current text is already detailed, but much of that detail is generic: cache warm-up, circuit breakers, user-interface misuse, authentication, secrets and production on-call responsibilities.    It explains the philosophies, but does not consistently tell an agent what files to inspect, which PoKeys shortcuts are prohibited, what evidence is required, or when it must stop.

## Better allocation

| Content                                        | Destination                      |
| ---------------------------------------------- | -------------------------------- |
| Repository-specific mandatory behavior         | Always-loaded root instructions  |
| C/RT-specific prohibitions                     | `c-realtime.instructions.md`     |
| Detailed conversion/testing procedure          | Skills                           |
| Historical motivation and extended explanation | `docs/engineering-discipline.md` |
| Detectable violations                          | CI scripts and static checks     |

The permanent instructions should act as an **engineering contract**, not as an inspirational manifesto.

## Proposed replacement section

## Repository Engineering Discipline

These rules exist because superficially successful changes—code that compiles, looks plausible, or satisfies only the happy path—have previously caused architectural violations, incomplete implementations, regressions, and poor real-time behavior.

They are mandatory for all repository work. They do not authorize overengineering. They require the agent to perform the essential engineering work for the requested change and to provide evidence for its claims.

### 1. Understand Before Modifying

Before changing code, the agent must establish the applicable contract from the repository.

For a behavior change, inspect the relevant sources in this order:

1. The linked GitHub issue, requirement, acceptance criteria, or explicit user request.
2. Existing architecture and file-responsibility instructions.
3. The current implementation and its callers.
4. The corresponding synchronous `PoKeysLib<Subsystem>.c` reference implementation, where applicable.
5. The PoKeys protocol specification for command codes, byte layouts, masks, limits, and response semantics.
6. Existing async subsystem implementations that demonstrate the repository’s current pattern.
7. Existing tests, build workflows, and known limitations.

Do not infer protocol behavior from function names, comments, nearby commands, or apparently similar subsystems.

Do not silently choose between contradictory sources. Identify the conflict and determine which source is authoritative before implementing affected behavior.

First resolve questions by inspecting available repository sources. Ask the user only when a material ambiguity remains and different interpretations would change external behavior, protocol handling, HAL interfaces, safety, or real-time characteristics.

### 2. Establish a Baseline

Before modifying behavior:

* Run the narrowest relevant existing build or test.
* Record whether the baseline passes or fails.
* Separate pre-existing failures from failures introduced by the change.
* Do not describe an untested repository state as working.
* Do not weaken, delete, skip, or mark a failing check as non-fatal merely to obtain a green result.

When hardware, LinuxCNC, the real-time kernel, or another required environment is unavailable, state exactly which verification could not be executed. Do not substitute compilation for hardware or timing validation.

### 3. Make the Smallest Complete Change

Implement the smallest coherent change that satisfies the requested behavior.

A small change is not an incomplete change. It must still include all required:

* production behavior;
* error handling;
* state transitions;
* protocol parsing;
* HAL exports;
* declarations and build integration;
* tests or reproducible verification;
* documentation updates;
* traceability references.

Do not broaden the task into unrelated cleanup, repository restructuring, generic framework creation, or speculative future support.

Do not perform a large rewrite when a localized correction is sufficient.

Do not create parallel implementations such as `new`, `old`, `fixed`, `v2`, or `refactored` variants when the existing implementation should be corrected.

### 4. No Fake Completion

The following do not constitute a completed implementation:

* a stub returning success;
* a placeholder body;
* a TODO standing in for required behavior;
* hardcoded sample values;
* simulated device responses in production code;
* a parser that accepts data without validating it;
* a function that compiles but is never registered or called;
* a HAL pin that is exported but never updated;
* a send function without its corresponding response handling;
* a test that cannot fail;
* a test that only checks compilation;
* a test changed to match incorrect implementation behavior;
* commented-out failing code or tests;
* a workflow changed to ignore failures;
* documentation claiming behavior that has not been implemented and verified.

A TODO is permitted only for explicitly out-of-scope future work. It must not conceal incomplete acceptance criteria and should reference a tracked issue where appropriate.

### 5. Preserve the PoKeysHal Architecture

Maintain the repository’s defined responsibilities.

* `PoKeysLibAsync.c` contains shared asynchronous transport, mailbox, dispatch, timeout, and retry infrastructure only.
* `PoKeysLib**Async.c` files contain subsystem-specific request creation, response parsing, HAL export, and optional scheduler registration.
* `experimental/pokeys_async.c` remains an integration shell and must not absorb subsystem implementations.
* `PoKeysLibHal.h` contains HAL-compatible data structures.
* `PoKeysLibAsync.h` contains shared asynchronous declarations, protocol enums, masks, offsets, and contracts.
* Canonical digital, analog, and encoder channels use the `hal-canon` interfaces.

Do not solve a local problem by violating these boundaries.

When the existing code already violates a boundary, do not copy the violation into new code. Contain the requested change, report the existing violation, and correct it when it is necessary for the requested work.

### 6. No Real-Time Shortcuts

Code reachable from a LinuxCNC real-time function must remain deterministic and bounded.

Do not introduce:

* blocking device communication;
* synchronous PoKeys requests;
* blocking socket operations;
* mutex waits in the RT path;
* dynamic allocation after real-time execution begins;
* file access;
* sleeps;
* unbounded loops;
* uncontrolled retries;
* expensive logging in a high-frequency path;
* hidden work whose runtime depends on device or network response time.

An asynchronous function is not RT-safe merely because its name ends in `Async`. Verify the complete call path.

Keep network reception, response dispatch, parsing, retries, and state changes consistent with the defined concurrency model. Shared state must have explicit ownership and synchronization semantics.

Do not claim a timing guarantee from code inspection. Timing compliance requires measurement in the appropriate runtime environment.

### 7. No Protocol Shortcuts

For every new or modified command:

* Verify the command and subcommand identifiers.
* Verify request parameter and payload positions.
* Verify response offsets, widths, byte order, ranges, and status bits.
* Use named enums, masks, and offset constants.
* Check response identity before applying data.
* Validate lengths before reading payload fields.
* Handle malformed, stale, duplicate, mismatched, and timed-out responses safely.
* Preserve request-ID and mailbox lifecycle rules.
* Define what happens after the final retry fails.

Do not copy a synchronous implementation mechanically. Separate request creation from response parsing and account for the time gap and concurrency between them.

### 8. Tests Must Prove the Requested Behavior

For production behavior changes, first establish one of the following:

* a failing automated test;
* a reproducible failing build or verification command;
* a documented hardware reproduction when automation is not currently possible.

The test or reproduction must fail for the relevant reason before the implementation is changed.

Tests must exercise observable behavior, not merely mirror implementation details.

Where applicable, cover:

* valid responses;
* invalid or truncated responses;
* wrong command or request ID;
* boundary values;
* timeout and retry exhaustion;
* state before and after response dispatch;
* repeated execution;
* disconnected-device behavior;
* relevant RT and userspace differences.

Do not invent artificial tests for documentation-only or metadata-only changes. For refactoring, establish a passing behavioral baseline before changing structure.

### 9. Make It Work, Make It Right, Then Make It Fast

Use this sequence:

1. **Make it work:** establish the smallest end-to-end behavior that satisfies one verified requirement.
2. **Make it right:** remove duplication, restore architecture boundaries, improve naming, handle errors, and keep tests green.
3. **Make it fast:** measure the relevant path and optimize only demonstrated bottlenecks.

Compilation is not equivalent to “works.”

Passing a happy-path test is not equivalent to “right.”

An optimization without before-and-after measurements is not demonstrated optimization.

Do not retain a knowingly poor design under the promise that it will be corrected later when the defect is within the current change’s scope.

### 10. Explicit Evidence Is Required

Before claiming completion, report:

* files changed;
* requirement or issue addressed;
* behavior implemented;
* architecture implications;
* tests added or changed;
* exact build and test commands executed;
* results of those commands;
* hardware or RT checks performed;
* checks not performed and why;
* remaining risks, assumptions, and out-of-scope work.

Use precise status terms:

* **Implemented** means the production behavior exists.
* **Compiled** means only that compilation succeeded.
* **Tested** means named tests were executed and passed.
* **Hardware-verified** means the stated behavior was observed with the relevant PoKeys device.
* **RT-validated** means it was tested in the applicable LinuxCNC real-time environment.
* **Timing-validated** means timing was measured with an identified method and acceptance threshold.

Do not use “complete,” “working,” “fixed,” “RT-safe,” or “verified” without the corresponding evidence.

### 11. Handle Problems Without Excuses

A tool, dependency, legacy defect, missing device, incomplete test environment, or unclear specification may constrain the work, but it does not justify inventing results or silently bypassing required steps.

When blocked:

1. State the exact blocker.
2. Show what was inspected or executed.
3. Separate confirmed facts from hypotheses.
4. Complete all unaffected work that can be done safely.
5. Propose the smallest concrete action needed to remove the blocker.
6. Do not claim the blocked verification as completed.

When a chosen approach fails, preserve useful evidence, revert unsafe partial work, and select the next justified approach. Do not repeatedly patch symptoms without identifying the failing assumption.

### 12. Leave the Repository Healthier, but Stay in Scope

When touching a file:

* correct directly related misleading comments;
* remove obsolete code made redundant by the change;
* improve tests needed to make the change safe;
* update the existing authoritative documentation;
* avoid introducing further duplication or architectural debt.

Do not turn a focused task into an unrequested cleanup campaign.

If significant unrelated defects are discovered, report or create a separate issue rather than mixing them into the current implementation.

### 13. Completion Gate

A task is complete only when all applicable items are satisfied:

* The requested behavior and acceptance criteria are implemented.
* Architecture boundaries are preserved.
* Protocol behavior has been checked against an authoritative source.
* RT paths remain non-blocking, bounded, and allocation-free.
* Relevant tests or reproducible checks exist and pass.
* Required builds succeed.
* No required behavior is represented by a stub, TODO, simulation, or hardcoded result.
* Documentation reflects actual behavior.
* Issue and requirement traceability is present.
* Failed, skipped, unavailable, or hardware-dependent checks are reported honestly.

If an item is not applicable, it need not be performed. If it is applicable but cannot be completed, the task must be reported as partially verified rather than complete.

## Why this is stronger

This version retains the original intent but converts abstract values into:

* **required preparation**;
* **explicit prohibited shortcuts**;
* **PoKeysHal architecture rules**;
* **RT and protocol-specific checks**;
* **evidence terminology**;
* **blocker behavior**;
* **a completion gate**.

The original principles can still be named—“Slow is Fast,” “No Excuses,” “No Shortcuts,” and “Clarify First”—but they should serve as headings or rationale. The actual control mechanism is the concrete contract above.

## Recommended permanent-context arrangement

I would retain a compressed version of sections 1–4, 6, 10 and 13 in `.github/copilot-instructions.md`. The full contract belongs in a path-scoped `engineering-discipline.instructions.md` or root `AGENTS.md`, depending on which agent platforms you want to support.

The detailed protocol and conversion sequences should also appear in the relevant skills, but repetition there is acceptable when it is intentional: root instructions establish the non-negotiable invariant, while the skill explains how to satisfy it.
