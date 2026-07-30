---
description: >
  PoKeysHal engineering discipline contract. 8 mandatory behavioral rules for all
  C implementation, protocol, HAL export, and real-time work. Loaded automatically
  for C files, headers, experimental components, and implementation/integration phases.
applyTo: "**/*.c,**/*.h,experimental/**,05-implementation/**,06-integration/**"
---

# Engineering Discipline Contract

These rules are mandatory. They do not authorize over-engineering. They require the essential engineering work for the requested change and explicit evidence for claims.

**1. Inspect before modifying.** Before changing behavior, establish the applicable contract: the linked issue or requirement; existing architecture and file-responsibility rules; the current implementation and its callers; the synchronous reference implementation where applicable; the PoKeys protocol specification for command codes, byte layouts, masks, limits, and response semantics; existing async subsystem patterns; existing tests and known limitations. Do not infer protocol behavior from function names or apparently similar subsystems. Resolve contradictions against an authoritative source before implementing.

**2. Establish a baseline.** Run the narrowest relevant existing build or test before modifying behavior. Record whether it passes or fails. Separate pre-existing failures from new ones. Do not describe an untested state as working. Do not weaken, skip, or mark a check as non-fatal to obtain a green result. When hardware or the RT kernel is unavailable, state exactly which verification could not be executed.

**3. Make the smallest complete change.** Implement the smallest coherent change that satisfies the requested behavior. A small change is not an incomplete change — it must still include all required production behavior, error handling, state transitions, protocol parsing, HAL exports, declarations, build integration, tests, documentation, and traceability. Do not broaden into unrelated cleanup, restructuring, or speculative future support. Do not create parallel variants (`new`, `old`, `v2`) when the existing implementation should be corrected.

**4. No fake completion.** Stubs, placeholder bodies, TODOs standing in for required behavior, hardcoded sample values, simulated device responses in production code, parsers that accept data without validating it, functions that compile but are never called, HAL pins exported but never updated, send functions without response handling, tests that cannot fail, tests that only check compilation, tests changed to match incorrect behavior, commented-out failing code, workflows changed to ignore failures, and documentation claiming unimplemented behavior — none of these constitute a completed implementation.

**5. No weakening checks.** Do not delete, skip, or weaken a failing test or verification step to obtain a passing status. If a check cannot be completed, report the task as **partially verified**, not complete.

**6. Report exact evidence.** Before claiming completion, report: files changed, requirement addressed, behavior implemented, tests added or changed, exact build and test commands executed, results of those commands, hardware or RT checks performed, checks not performed and why, remaining risks. Use precise terms: **Implemented** (production behavior exists); **Compiled** (compilation only); **Tested** (named tests executed and passed); **Hardware-verified** (observed with PoKeys device); **RT-validated** (tested in LinuxCNC RT environment); **Timing-validated** (measured with an identified method and threshold). Do not use "complete," "working," "fixed," "RT-safe," or "verified" without corresponding evidence.

**7. Unavailable verification = partially verified.** When hardware, LinuxCNC, or the RT kernel is unavailable, state exactly which verification could not be executed. Do not substitute compilation for hardware or timing validation. A task with an outstanding unavailable check is **partially verified**, not complete.

**8. Stay in scope.** Do not turn a focused task into an unrequested cleanup campaign. Correct directly related misleading comments; remove code made obsolete by the change; update existing authoritative documentation. Report significant unrelated defects as a separate issue rather than mixing them into the current change.
