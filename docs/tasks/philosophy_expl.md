# Development Philosophy — PoKeysLibHal

**Document type:** Project Philosophy & Engineering Standards  
**Applies to:** All contributors to `pokeysHal`  
**Related files:** [`.github/copilot-instructions.md`](../../.github/copilot-instructions.md), [`.github/instructions/submodules.instructions.md`](../../.github/instructions/submodules.instructions.md)

---

## 1. Introduction

This document explains the **development philosophy** adopted for the `pokeysHal` project. It answers the question: *Why do we work this way?* Understanding the philosophy helps every contributor make consistent decisions even in situations not explicitly covered by rules or instructions.

The philosophy rests on three pillars:

1. **Empirical engineering** — replace speculation with measurement and tests
2. **Standards-compliant process** — follow IEEE/ISO/IEC standards as a structure, not a bureaucracy
3. **Extreme Programming (XP) values** — keep feedback loops short, design simply, communicate honestly

---

## 2. Why These Practices Matter for a Real-Time Driver

PoKeysLibHal is not a typical application. It is a **hard real-time HAL driver** for industrial CNC use. Failures are not gracefully degraded — they can damage machines, tooling, or people. This context raises the cost of every undiscovered defect and mandates:

- **Temporal correctness**: Deadlines are part of correctness, not a nice-to-have. Missing a 1 ms RT cycle is a functional defect.
- **No speculation**: "I think this will work" is not acceptable. Proof through testing is required.
- **Deterministic execution**: No unbounded loops, no dynamic allocation, no blocking calls in RT paths.
- **Conservative design**: YAGNI (You Aren't Gonna Need It) is a safety principle here — speculative complexity creates RT unpredictability.

---

## 3. Core Values

### 3.1 Courage

- Deliver unpleasant truths immediately (e.g., "the `motmod` issue prevents full LinuxCNC startup").
- Report bad news early rather than hiding it — stakeholders have the right to know.
- Provide **options** (not excuses) when reporting problems.
- Separate **estimates** from **promises**: a timeline is an estimate; truth is the only promise.

### 3.2 Feedback

- Seek feedback in **minutes or hours**, not weeks.
- Working software is the primary measure of progress — "90% done" without working code is not progress.
- The TDD Red-Green-Refactor cycle should complete in under 10 minutes.
- CI must stay green; a broken build is fixed before any other work continues.

### 3.3 Communication

- Make status visible at a glance (checklist-based `Todo.md`, completed/in-work/planned task structure).
- Document decisions with rationale (ADRs); future contributors deserve to understand *why*, not just *what*.
- Use the ubiquitous language of the domain consistently: HAL pins, RT threads, mailbox, subsystem — always spelled the same way.

### 3.4 Simplicity

- Build only what is needed today. Speculative features add code surface area and RT unpredictability.
- Simple design passes all tests, reveals intention clearly, contains no duplication, and uses the minimum number of classes and functions.
- If a design feels like "walking uphill", stop — that instinct signals a design problem worth fixing now.

### 3.5 Respect

- Problems are team problems, not individual failures.
- Psychological safety allows honest status reporting.
- Collective ownership: any contributor can improve any part of the codebase.

---

## 4. Test-Driven Development (TDD)

**Absolute rule**: write a failing test *before* writing production code.

```
Red   → Write a test that fails (proves the feature does not yet exist)
Green → Write the minimum code to pass the test
Refactor → Improve design while keeping all tests green
```

### Why TDD matters here

- RT code is difficult to debug after the fact — tests catch defects before they enter the RT path.
- TDD forces the developer to think about the interface before the implementation, producing cleaner APIs.
- Each test is a permanent specification; they document intent as reliably as comments but are always up to date.

### Current test gaps

Unit tests for the async infrastructure (`CreateRequestAsync`, `SendRequestAsync`, `PK_ReceiveAndDispatch`) are identified as technical debt. These functions are central to correctness — adding TDD coverage for them is a high-priority goal.

---

## 5. Standards as Structure, Not Bureaucracy

Standards are adopted because they encode hard-won lessons from the engineering community, not because compliance is an end in itself.

| Standard | Why it is used |
|----------|----------------|
| **ISO/IEC/IEEE 12207:2017** | Provides lifecycle phases (requirements → architecture → design → implementation → integration → V&V → transition → maintenance) as a navigational structure, not a waterfall |
| **ISO/IEC/IEEE 29148:2018** | Ensures requirements are complete, unambiguous, and traceable to code and tests |
| **ISO/IEC/IEEE 42010:2011** | Architecture description as viewpoints + concerns = clear, communicable structure |
| **IEEE 1016-2009** | Design descriptions at the right level of detail for the implementation team |
| **IEEE 1012-2016** | V&V planning ensures testing is systematic, not ad-hoc |
| **IEC 61508** | Safety-critical implementation patterns (no RT blocking, no dynamic allocation, deterministic execution) |

The lifecycle phases (01 through 09) provide orientation:

```
Phase 01: Stakeholder requirements (what do users need?)
Phase 02: System requirements (what must the system do?)
Phase 03: Architecture (how are the major pieces structured?)
Phase 04: Detailed design (how does each component work?)
Phase 05: Implementation (TDD: Red-Green-Refactor)
Phase 06: Integration (continuous integration, adapter tests)
Phase 07: V&V (test against requirements, acceptance criteria)
Phase 08: Transition (deployment, user documentation)
Phase 09: Operation & Maintenance (monitor, fix, improve)
```

Most active work on this project is in phases 05–07. The lifecycle structure helps ensure nothing is skipped.

---

## 6. Domain-Driven Design (DDD) Principles

### 6.1 Ubiquitous Language

All code, documentation, and communication uses a shared vocabulary:

| Term | Meaning |
|------|---------|
| **HAL pin** | A LinuxCNC Hardware Abstraction Layer signal point exported by the component |
| **RT thread** | The real-time servo thread invoked at a fixed period (e.g., 1 ms) |
| **Mailbox** | Pre-allocated async request/response buffer shared between RT and receive threads |
| **Subsystem** | One functional area of the PoKeys device (digital I/O, encoders, PEv2, etc.) |
| **Dispatch** | Matching an incoming UDP response to a pending mailbox entry |
| **Parse callback** | A function invoked by `PK_ReceiveAndDispatch` to decode a response into HAL-typed fields |
| **Adapter layer** | Code that translates between a submodule's API and the project's domain model |

### 6.2 Bounded Contexts

The project has three bounded contexts with explicit adapter layers between them:

1. **PoKeys device protocol** (`pokeyslib/` submodule) — device-level binary protocol
2. **PoKeysLibHal async layer** (this project) — async request/response mapped to HAL types
3. **LinuxCNC HAL component** (`experimental/pokeys_async.c`) — HAL pin export and RT function

Changes in one context must not leak into another. The `PoKeysLibHal.h` HAL structs are the adapter boundary between contexts 1 and 2; `export_<subsystem>_pins()` functions are the boundary between contexts 2 and 3.

---

## 7. Real-Time Engineering Principles

### 7.1 Temporal Correctness

Requirements must state timing constraints in measurable terms:
- "The RT function must complete in < 1 ms" (not "it must be fast")
- Timing is **proven by measurement**, not asserted (GPIO + oscilloscope, or cycle counter comparison)

### 7.2 Terse ISRs / RT Functions

- Hard RT: complete in < 5 µs
- Soft RT: complete in < 50 µs
- The PoKeys RT function currently measures ~0.35 ms maximum on a 1 ms thread (35% utilization)

### 7.3 RT Safety Rules (non-negotiable)

- **No `malloc`/`free`** in RT paths — use pre-allocated static or HAL-allocated structures
- **No blocking calls** — UDP socket must be `O_NONBLOCK`; no `sleep()`, `mutex_lock()`, or system calls that may block
- **No unbounded iterations** — loops in RT code must have a known maximum iteration count
- **`mlockall(MCL_CURRENT | MCL_FUTURE)`** — prevents page faults in RT context

---

## 8. Submodule and Dependency Governance

External code is brought in as a Git submodule only when:
- A precise snapshot of another repository is needed
- The dependency cannot be published as a package
- A clear adapter layer can isolate the domain from the submodule's internals

Submodule management follows the [Submodule Instructions](./../.github/instructions/submodules.instructions.md):
- Pin to immutable SHAs (not branch tracking)
- Adapter layer always present; no domain leakage
- CI fetches submodules recursively
- Update policy and license documented per module

---

## 9. Traceability

Every work item traces bidirectionally:

```
Stakeholder Need → GitHub Issue (StR)
  → System Requirement (REQ-F / REQ-NF Issue)
    → Architecture Decision (ADR Issue)
      → Implementation (PR referencing issues)
        → Test Case (TEST Issue)
          → Verification Result
```

Even when working informally (outside the full issue template process), each PR description should reference the requirement it implements and the tests that verify it.

---

## 10. Honest Status Reporting

Progress is reported against objective criteria, not feelings:

- ✅ Tests pass and code is deployed — **done**
- ⚠️ Tests pass in isolation but integration issue identified — **partially done with known issue**
- ❌ No passing tests — **not done**

"90% complete" without working tests means "less than 50% complete" from a standards standpoint.

The `docs/Todo.md` and task documents in `docs/tasks/` are the primary status instruments. They are updated after every meaningful unit of work.

---

## 11. Summary

| Principle | Practical effect |
|-----------|-----------------|
| TDD (Red-Green-Refactor) | Tests written before code; no untested changes committed |
| Simple design | YAGNI; no speculative features; minimum viable implementation |
| Short feedback loops | CI must pass before merge; compile tests run immediately |
| Standards as structure | Lifecycle phases guide navigation; standards encode engineering lessons |
| Honest status | Progress measured by working, tested software |
| RT discipline | No blocking, no malloc, no unbounded loops in RT paths |
| Ubiquitous language | Consistent domain vocabulary in code, docs, and communication |
| Traceable work | GitHub issues link requirements ↔ code ↔ tests |

---

*See [`docs/repo_review.md`](../repo_review.md) for an objective assessment of the current repository state against these principles.*  
*See [`.github/copilot-instructions.md`](../../.github/copilot-instructions.md) for the full standards-compliant AI assistant instructions.*
