## PoKeysHal AI Coding Agent Instructions

### Big Picture Architecture
- The project implements a LinuxCNC HAL-compatible driver for PoKeys devices, supporting both userspace and realtime (RT) operation.
- Core architecture uses an asynchronous request/response mailbox system for non-blocking communication. See `Pokeys Async Overview.md` and `README.md` for design rationale and workflow diagrams.
- Major components:
  - `experimental/`: RT HAL components and async logic
  - `pokeyslib/`: User-space reference implementation (feature parity required)
  - `hal-canon/`: Canonical HAL helpers for analog/digital/encoder
  - Protocol details in `PoKeys - protocol specification.pdf`

### Critical Developer Workflows
- **Build RT HAL component:**
  - Use VSCode task "4 - Build RT HAL component (Submakefile.rt)" or run:
    `rm -rf experimental/build && mkdir -p experimental/build && cp Makefile.noqmakeRT *.h *.c experimental/pokeys_async.c experimental/Submakefile.rt experimental/build/ && cp hal-canon/*.c hal-canon/*.h experimental/build/ && cd experimental/build && make -f Submakefile.rt all && sudo make -f Submakefile.rt install`
- **Build userspace HAL component:**
  - Use VSCode task "3 - Build HAL userspace component" or run:
    `sudo halcompile --install --userspace --extra-link-args='-L/usr/lib -lPoKeysHal' experimental/pokeys_async.c && sudo halcompile --preprocess experimental/pokeys_async2.comp`
- **Run HAL components:**
  - Use VSCode tasks "5" and "6" for userspace and RT execution via `halrun`.
- **Install dependencies:**
  - Use "1 - Install apt dependencies" task to install required packages from `prerequisites.txt`.
- **Testing:**
  - Manual and automated tests should verify async, non-blocking, and deterministic behavior. See `docs/Todo.md` for compliance and ToDo tracking.

### Project-Specific Conventions & Patterns
- All RT code must use async APIs (e.g., `PK_*Async`), avoid blocking, and only allocate memory before thread start with `hal_malloc`.
- Use `#ifdef RTAPI` for RT-specific code sections.
- Logging in RT code must use `rtapi_print_msg`.
- Protocol implementation must match `PoKeys - protocol specification.pdf`.
- Feature parity between RT and userspace code is required; extend userspace code as needed.
- Use "ToDo" comments and maintain `docs/Todo.md` for ongoing work. Track open/in-work/closed tasks in `docs/open`, `docs/in-work`, and `docs/closed`.

### Integration Points & External Dependencies
- Integrates with LinuxCNC HAL (RTAPI, halcompile, etc.).
- Depends on libusb for device communication (see `PoKeysLib.pro`).
- Submodule `pokeyslib` provides user-space reference and binaries.
- Build and CI workflows are defined in `.github/workflows/build.yml` and `deb-package.yml`.

### Examples
- See `experimental/pokeys_async.c` and `pokeys_async2.c` for async RT code patterns.
- See `Pokeys Async Overview.md` for mailbox, send/receive, and timeout logic.
- See `README.md` for architecture, flowcharts, and key features.

### Task Specification
- Use docstrings and comments to clarify intent, constraints, and completion criteria.
- Example ToDo block:
  ```c
  // ToDo: Implement PK_UARTWriteAsync for RT thread, ensure non-blocking and deterministic behavior.
  ```

### Completion Criteria
- All new code must:
  - Use async APIs for device communication
  - Avoid blocking, nondeterministic, or unsafe operations
  - Document tasks and progress in the appropriate docs folder
  - Pass manual/automated tests for RT compliance
 
    ## 🧭 Core Philosophy: "Slow is Fast" + "No Excuses" + "No Shortcuts" + "Clarify First"

### "Slow is Fast": Deliberate Development

> **If you go deliberately and carefully now, you'll go much faster overall.**

**In Development Process**:
- **Design before coding** → Fewer rewrites, less scope creep, easier maintenance
- **Tests & TDD** → Bugs caught early, changes safer/faster, confident shipping
- **Code reviews** → Better APIs, fewer defects, knowledge spread
- **Avoid premature optimization** → Cleaner code, real performance gains where it matters
- **Tooling & automation** → Every future change faster, safer, more repeatable

**In Runtime Behavior**:
- **Backpressure & throttling** → Systems stay stable, higher effective throughput
- **Correct concurrency** → Fewer race conditions, less debugging, safer scaling
- **Cache warm-up & gradual rollouts** → Predictable performance, smoother operation

**What It Does NOT Mean**:
❌ Endless architecture astronautics  
❌ Perfect design before any code  
❌ Never shipping because still "refining"  

**What It DOES Mean**:
✅ Purposeful pacing  
✅ Short feedback loops  
✅ Small, well-thought increments  

**Heuristic**: If "going slow" reduces rework, bugs, or instability later, it's the kind of "slow" that makes you fast.

---

### "No Excuses": Ownership and Robustness

> **If it's your code or your system, you own the outcome – not the tools, not the spec, not "the user", not the deadline.**

**Ownership of Behavior**:
- Library has a bug? → Sandbox it, add retries, or replace it
- API is weird? → Wrap it in a sane adapter
- Users misuse UI? → Improve UX, validation, confirmations
- Legacy code is messy? → Anti-corruption layers, gradual migration
- **Result**: Defensive coding, better abstractions, stable behavior

**Error Handling (Assume Things Go Wrong)**:
- Don't assume files exist → Check, handle failure, log clearly, degrade gracefully
- Don't assume network is fine → Timeouts, retries with backoff, circuit breakers
- Don't assume happy path → Test edge cases, document failure modes
- **Result**: Systems fail under control with good diagnostics

**Quality (No Shortcuts)**:
- "No time for tests" → Cover critical paths at minimum
- "We'll refactor later" → Leave code slightly better than you found it
- "Deadline pressure" → Avoid "just this once" shortcuts that become permanent
- **Result**: Fewer regressions, lower maintenance cost, less firefighting

**Communication (No Surprises)**:
- Dependency late? → Communicate early, propose options
- Scope unrealistic? → Say it explicitly, suggest trade-offs
- Made a mistake? → Admit quickly, focus on mitigation
- **Result**: Clear contracts, fewer shocks, trust in commitments

**What "No Excuses" Does NOT Mean**:
❌ Blaming individuals when things break  
❌ Ignoring systemic problems  
❌ Forcing overtime / heroics  
❌ Suffering silently without raising issues  

**What "No Excuses" DOES Mean**:
✅ Owning your part of the system  
✅ Being proactive instead of reactive  
✅ Turning problems into concrete actions (tests, refactors, monitoring)  
✅ Professionalism: don't argue with reality, don't hide behind tools  

**Heuristic**: Reasons explain problems; excuses avoid responsibility. Acknowledge constraints, then optimize within them.

---

### "No Shortcuts": Refusing to Trade Long-Term Health for Short-Term Gain

> **Don't sacrifice the system's long-term health for a tiny short-term win. Simplify and optimize, but never skip the essentials: correctness, clarity, tests, security.**

**Design & Architecture (No Shortcuts)**:
- One service/class now → "We'll refactor later" → Never happens; every change hurts
- Hardcoded values → "Configs are overkill" → Changes require code deploys
- No interfaces → "We'll define them later" → Tight coupling, risky refactors
- **No shortcuts**: Define minimal but clear boundaries; separate concerns even in small steps; avoid "temporary" hacks that become permanent
- **Result**: Even small designs are deliberate and leave room to evolve

**Tests & Correctness (No Shortcuts)**:
- "This is trivial, no test needed" → Hidden regressions, fear of change
- "Tests after the demo" → Never written; bugs discovered in production
- Copy/paste blocks → "What could go wrong?" → Multiple versions to maintain
- **No shortcuts**: Cover critical paths and edge cases; write tests when fixing bugs; prefer small testable units
- **Result**: Reliability costs time upfront, saves multiples later

**Error Handling & Resilience (No Shortcuts)**:
- Ignoring return codes → "It won't fail" → No idea what happened in production
- No timeouts/retries → Single point of failure cascades
- Missing or noisy logging → Cannot diagnose failures
- **No shortcuts**: Handle failures as normal cases; useful error messages with context; timeouts, retries, backoff, fallbacks
- **Result**: Small glitch self-heals vs. full-scale outage

**Security & Validation (No Shortcuts)**:
- "Internal only, no auth needed" → Internal services can be abused
- "Trust the client" → Injection attacks, data corruption
- Secrets in code → "Just for convenience" → Security incident
- **No shortcuts**: Validate and sanitize external input; treat internal services as potentially hostile; proper secret management, least privilege
- **Result**: Security shortcuts are cheap today, catastrophic tomorrow

**Performance & Optimization (No Shortcuts)**:
- Premature optimization → "We must be fast now" → Complex, unmaintainable code
- No measurement → "I think this is slow" → Optimizing wrong thing
- **No shortcuts**: First write simple, clear code; measure with profiler; optimize true hot spots; document why optimizations exist
- **Result**: Don't shortcut the measure → analyze → optimize cycle

**Documentation & Naming (No Shortcuts)**:
- No README/docs → "We know what it does" → Slow onboarding, forgotten context
- Cryptic names → Future you doesn't understand it
- No change log → Breaking changes surprise users
- **No shortcuts**: Name things clearly; document non-obvious invariants/assumptions; maintain minimal but current README/architecture notes
- **Result**: Leave future you and others a usable map

**Code Review & Collaboration (No Shortcuts)**:
- Merge without review → "It's urgent" → Quality erosion
- Giant PRs → Mixed concerns, impossible to review properly
- Ignore feedback → "It works, move on" → Trust erosion
- **No shortcuts**: Small changes reviewable properly; address feedback or explain reasoning; use reviews to improve code and shared understanding
- **Result**: Invest minutes now to avoid hours of confusion later

**Refactoring & Technical Debt (No Shortcuts)**:
- "Add another if" → "Clean later" → Snowballing complexity
- Duplicate logic → "Faster than extracting" → Multiple versions drift
- Leave broken abstractions → "Not my problem" → Every feature takes longer
- **No shortcuts**: Boy Scout rule (leave code better than found); pay back technical debt regularly; when touching fragile area, stabilize it (tests + refactor)
- **Result**: Prevent compound interest on technical debt

**What "No Shortcuts" Does NOT Mean**:
❌ Overengineering everything  
❌ Adding layers "just in case"  
❌ Blocking delivery until everything is "perfect"  
❌ Gold-plating features  

**What "No Shortcuts" DOES Mean**:
✅ Do the **essential** engineering work for the problem at hand  
✅ Don't knowingly skip things that will hurt you soon (tests, error handling, minimal design, basic docs)  
✅ Simplify by **reducing complexity**, not by ignoring necessary work  
✅ Distinguish between YAGNI (speculation) and needed work (correctness, maintainability)  

**Heuristic**: Am I avoiding work that makes the system safer, clearer, easier to change in the **near future**? If yes, that's probably a shortcut I shouldn't take.

**Key Distinction**:
- **YAGNI** (You Aren't Gonna Need It) → Don't build speculative features
- **No Shortcuts** → Don't skip essential engineering for current features

---

### "Clarify First – Never Code on Guesses"

> **Requirements are part of your job, not "somebody else's problem". Misinterpretations are defects, not excuses.**

**Essence**: Vague requirements = bugs waiting to happen. Clarifying requirements is engineering work, not overhead.

**In Requirements Engineering**:
- Vague requirement? → Surface it, don't ignore it
- Contradictory specs? → Don't pick one silently; escalate
- Unsure what user needs? → Don't guess; ask with concrete examples
- **Result**: Spend 30 minutes clarifying now vs. days reworking later

**Clarification Test** (Before Implementing Anything):
> If you cannot explain the requirement back in 2–3 sentences with clear examples, you don't understand it well enough to implement it.

**Treat Misinterpretations as Process Bugs**:
- Every mismatch between implemented vs. needed → Fix code + improve capture process
- Root causes:
  - Vague wording → Require concrete examples + acceptance criteria
  - Hidden assumptions → Make explicit, reference previous patterns
  - Spoken-only decisions → Must be logged before implementation
  - Ambiguous domain terms → Maintain glossary (ubiquitous language)
- **Result**: Don't blame people; harden system against misinterpretation

**What "Clarify First" Does NOT Mean**:
❌ Analysis paralysis / endless refinement  
❌ Refusing to start until 100% perfect spec  
❌ Treating stakeholders as adversaries  
❌ Ignoring emergent understanding  

**What "Clarify First" DOES Mean**:
✅ No implementation without written acceptance criteria  
✅ Restate requirements in your own words ("So what you want is...")  
✅ Look for edge cases upfront, not after implementation  
✅ Capture clarifications somewhere persistent (ticket, spec, comment)  
✅ Follow requirements – AND challenge them when they don't make sense  
✅ Respect the spec, but question inconsistencies, safety issues, incompleteness  

**Concrete Practices**:
- **Specification by Example**: Every requirement has Given-When-Then examples
- **Executable Specs**: Tests are living requirements; if tests fail, either code or spec is wrong
- **Frequent Demos**: Thin slices shown early reveal mental model mismatches
- **Traceability**: Every PR/commit references requirement; each requirement links to design/tests/code

**Heuristic**: If you're implementing based on assumptions rather than confirmed understanding, you're introducing technical debt disguised as progress.

**Reinforces**:
- "Slow is fast" (30 minutes clarifying now saves days of rework)
- "No excuses" (vague spec explains problem but doesn't absolve team from fixing process)
- "No shortcuts" (don't implement half-baked features just because text exists)

---

## Complementary Engineering Philosophies

These principles build upon and reinforce the three core philosophies:

### 1. "Make it work, make it right, make it fast"

**Sequence**: Correctness → Design → Optimization

**Application**:
- **Make it work**: First get a minimal vertical slice working end-to-end; prove feasibility
- **Make it right**: Clean up design, extract abstractions, add tests; remove duplication
- **Make it fast**: Only then profile and optimize hotspots; avoid premature optimizations

**Reinforces**: "Slow is fast" (deliberate sequence), "No shortcuts" (don't skip "make it right")

---

### 2. "Simplicity over cleverness"

**Essence**: Simple and boring beats clever and fragile

**Application**:
- Prefer straightforward algorithms and designs over "smart" tricks
- Choose standard patterns and libraries unless there is a clear reason not to
- If a solution is hard to explain in 2-3 minutes, it might be too clever

**Reinforces**: "No shortcuts" (maintainability), "Slow is fast" (simpler = fewer bugs)

---

### 3. "Small, incremental change"

**Essence**: Big-bang changes are fragile; small steps are safe and fast

**Application**:
- Small PRs/commits with a single clear purpose
- Refactor in slices instead of massive rewrites
- Deploy frequently with small deltas → simpler rollbacks and debugging

**Reinforces**: "Slow is fast" (fast feedback loops), XP Continuous Integration

---

### 4. "Explicit is better than implicit"

**Essence**: Make behavior and dependencies visible

**Application**:
- Clear function signatures instead of hidden globals
- Explicit configuration instead of "magic" defaults
- Clear types and contracts instead of relying on convention

**Reinforces**: "No excuses" (debuggability), "No shortcuts" (clarity over cleverness)

---

### 5. "Feedback is a first-class asset"

**Essence**: Treat every form of feedback as a primary tool, not a nuisance

**Application**:
- Tests, linters, logs, metrics, profilers = feedback loops
- User bug reports and complaints are input to improve robustness and UX
- Code reviews are a feedback mechanism, not a formality

**Reinforces**: "Slow is fast" (rapid feedback), "No shortcuts" (don't disable alarms)

---

### 6. "You build it, you run it"

**Essence**: Responsibility for code extends into production

**Application**:
- Developers involved in monitoring, alerting, and on-call (where appropriate)
- Design decisions consider operability: logs, metrics, traceability
- Don't throw code "over the fence" to ops/support

**Reinforces**: "No excuses" (ownership), "No shortcuts" (operational concerns upfront)

---

### 7. "If it hurts, do it more often (and automate)"

**Essence**: Painful tasks signal missing automation or process design

**Application**:
- If releases are painful, release more frequently and automate
- If merging is painful, integrate more often and refine branching strategy
- If testing is painful, improve test tools and testability

**Reinforces**: "Slow is fast" (invest in tooling), XP Continuous Integration

---

### 8. "Prefer boring technology for critical paths"

**Essence**: Stability and predictability beat novelty for core systems

**Application**:
- For critical infrastructure (drivers, timing, finance, production systems), prefer:
  - Well-known languages
  - Mature frameworks
  - Battle-tested libraries
- Use experimental or cutting-edge tech at the edges, not in the core

**Reinforces**: "No shortcuts" (don't buy "fast" development with unknown risks)

---

### 9. "Strong opinions, weakly held"

**Essence**: Be decisive, but change your mind when confronted with better evidence

**Application**:
- Have a default way to do things (coding style, architecture preferences)
- When data or convincing arguments show a better way, adapt quickly
- Avoid dogmatism ("we *always* do X") in favor of reasoned standards

**Reinforces**: XP values (courage to change, feedback-driven), "Slow is fast" (learning)

---

### 10. "Leave the campsite cleaner than you found it" (Boy Scout Rule)

**Essence**: Always make the codebase slightly better as you touch it

**Application**:
- When you work in a file:
  - Fix a small smell
  - Update a misleading comment
  - Add or improve a test
- Don't wait for a "big refactor" that may never be scheduled

**Reinforces**: "No shortcuts" (incremental improvement), prevents technical debt compound interest

---

### 11. "Reuse before reinvent" + "One source of truth" + "Curate, don't accumulate"

**Essence**: Prefer existing solutions, eliminate duplication, keep codebase clean

**11a. Reuse Before Reinvent**:
- Before writing new code, check if codebase or mature libraries already have it
- Wrap or extend existing components instead of forking casually
- Align with standard implementations and idioms
- **Result**: Less code, fewer bugs, less maintenance; more time for domain problems

**11b. One Source of Truth** (No Redundant Implementations):
- Each concept has single authoritative implementation or definition
- Shared constants/types instead of duplicated literals (enums, error codes, register definitions)
- Centralized business rules (one module, used everywhere)
- When duplicated logic found → Extract to shared function/module, replace all copies
- **Result**: Rule changes in one place; compiler/tests propagate

**11c. Curate, Don't Accumulate** (Keep Repo Clean):
- Repository is curated product, not dumping ground
- Dead code is removed, not commented out
- Obsolete paths deprecated then deleted
- Experiments either graduate to structured location (`experiments/`) or are archived
- Regular cleanup: Remove unused functions/files, consolidate helpers, delete old experiments
- **Result**: Clear structure, no `old/`, `backup/`, `tmp/` folders; no `xyz_old.c`, `xyz_new.c`, `xyz_refactor.c`

**Trade-offs and Guardrails**:
- Third-party dependencies: Check licensing, maintenance, quality, API stability
- Don't pull huge framework for tiny feature
- Internal reuse: Don't force-fit where requirements truly differ
- If generalizing, keep API simple; avoid over-generalizing

**Reinforces**: 
- "No shortcuts" (eliminate duplication properly, don't leave tech debt)
- "Boy Scout rule" (cleanup as you go)
- "Simplicity over cleverness" (one clear solution beats many fragmented ones)

---
