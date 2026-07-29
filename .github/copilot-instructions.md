# Root Copilot Instructions - Standards-Compliant Software Development

## 🗂️ Repository Context: pokeysHal

This repository implements the **PoKeysLibHal** — an optimized HAL (Hardware Abstraction Layer) driver for PoKeys USB/Ethernet devices intended for **hard real-time use with LinuxCNC**.

### Tech Stack
- **Language**: C (C99), with some `.comp` files for LinuxCNC HAL components
- **Build system**: `make -f Makefile.noqmake` (primary), CMake (`cmake .. && make`)
- **Target platform**: Linux (Ubuntu/Debian), LinuxCNC (RTAI or PREEMPT_RT kernel)
- **Key dependencies**: libusb-1.0, LinuxCNC HAL (`liblinuxcnchal`, `halcompile`), pthreads

### Build Commands
```bash
# Build the HAL library
sudo make -f Makefile.noqmake install

# Build with CMake
mkdir -p build && cd build && cmake .. && make

# Build and install the LinuxCNC HAL component (userspace)
sudo halcompile --install --userspace --extra-link-args="-L/usr/lib -lPoKeysHal" experimental/pokeys_async.c

# Build RT component
cd experimental/build && make -f Submakefile.rt all && sudo make -f Submakefile.rt install
```

### Testing
```bash
# Run userspace HAL component test
halrun <<EOF
loadusr -W pokeys_async
show pin && show funct && start && show pin && show param && exit
EOF

# Run real-time HAL component test
halrun <<EOF
loadrt threads name1=test-thread period1=1000000
loadrt pokeys_async
addf pokeys-async.0 test-thread
start && show pin && show param && exit
EOF

# Compile test
bash test_compile.sh
```

### Repository Structure
- `PoKeysLib*.c / PoKeysLib*.h` — PoKeys communication library (C source)
- `PoKeysLibHal.h` — HAL-conform structs (uses `hal_u32_t`, `hal_s32_t`, `hal_float_t`, `hal_bit_t`; includes canonical interfaces `hal_digin_t`, `hal_digout_t`, `hal_adcin_t`, `hal_adcout_t`, `hal_encoder_t`)
- `PoKeysLibAsync.h` — Async infrastructure declarations (enums, structs, `PK_*Async()` declarations)
- `PoKeysLibAsync.c` — **Async infrastructure ONLY**: `CreateRequestAsync()`, `SendRequestAsync()`, `PK_ReceiveAndDispatch()`, `PK_TimeoutAndRetryCheck()`, mailbox management. No subsystem logic, no HAL exports.
- `PoKeysLib**Async.c` — **Per-subsystem async** (fundamentally different from `PoKeysLibAsync.c`): async implementation of the corresponding `PoKeysLib**.c` subsystem, with (1) `export_<subsystem>_pins()` HAL export, (2) Send functions (`PK_**GetAsync()`) called from RT thread, (3) Parse callbacks (`PK_**Parse()`) invoked by `PK_ReceiveAndDispatch`, (4) optional `register_<subsystem>_tasks()` for scheduler
- `experimental/pokeys_async.c` — LinuxCNC HAL component integration shell (calls `export_**_pins()`, `EXTRA_SETUP`, RT function, `user_mainloop`)
- `experimental/async_scheduler.h/.c` — Async scheduler (to be migrated to `PoKeysLibAsync`)
- `hal-canon/` — Canonical HAL pin implementations (`hal_digin_t`, `hal_digout_t`, `hal_adcin_t`, `hal_adcout_t`, `hal_encoder_t`)
- `hid*.c / hidapi.h` — HID/USB interface
- `docs/` — Project documentation
- `.github/instructions/` — Phase-specific and architecture Copilot instructions

> ⚠️ **Architecture rules are enforced via `.github/instructions/pokeyshal-architecture.instructions.md`**. All agents MUST follow these rules when reading, writing, or reviewing C/H files.

### Coding Conventions (C)
- C99 standard; avoid C++-isms
- No dynamic memory allocation in real-time threads (use `mlockall()`, pre-allocate)
- No blocking calls in real-time paths; use non-blocking sockets and async patterns
- Use `hal_u32_t`, `hal_s32_t`, `hal_bit_t`, `hal_float_t` for all HAL-exposed struct members in `PoKeysLibHal.h`
- ISRs/RT functions must complete in <50µs (soft RT) or <5µs (hard RT)
- Prefix all public API functions with `PK_` (e.g., `PK_DigitalIOSetAsync`)
- Header guards: `#ifndef POKEYSLIB_XXX_H` pattern
- HAL pin exports for digital/analog/encoder channels use canonical functions: `hal_export_digin()`, `hal_export_digout()`, `hal_export_adcin()`, `hal_export_adcout()`, `hal_export_encoder()`

### Key Real-Time Constraints
- No `malloc`/`free` in RT paths
- No blocking I/O (UDP socket must be `O_NONBLOCK`)
- HAL pins exported via `export_<subsystem>_pins()` in `PoKeysLib**Async.c` (NOT directly in `experimental/pokeys_async.c`)
- `mlockall(MCL_CURRENT | MCL_FUTURE)` required for RT memory locking

---

## Engineering Discipline

The full engineering contract (13 rules, completion gate, evidence requirements) is in:
- `AGENTS.md` — compressed cross-tool invariants (rules 1–4, 6, 10, 13)
- `.github/instructions/engineering-discipline.instructions.md` — full contract, loaded automatically for C/implementation work
- `docs/engineering-discipline.md` — extended rationale and philosophy

For C/header work, `c-realtime.instructions.md` and `pokeyshal-architecture.instructions.md` apply automatically.

## Traceability

All work must start with a GitHub issue. Link artifacts using `#N` syntax:
- PRs: `Fixes #N` or `Implements #N` in the PR description
- Source: `/* Implements: #N (REQ-F-xxx) */`
- Tests: `/* Verifies: #N */`

Issue types: StR, REQ-F, REQ-NF, ADR, ARC-C, QA-SC, TEST.
Child issues link upward; parent issues link downward. PRs without an issue link will fail review.

## Lifecycle Phases

Phase-specific instructions auto-apply by directory:
`01-stakeholder-requirements/`, `02-requirements/`, `03-architecture/`, `04-design/`,
`05-implementation/`, `06-integration/`, `07-verification-validation/`, `08-transition/`, `09-operation-maintenance/`

## Routing

| Task | Resource |
|---|---|
| C/RT async subsystem conversion | `convert-to-hal-rtapi` skill |
| C/header architecture enforcement | `pokeyshal-architecture.instructions.md` |
| C/RT quick-reference + build commands | `c-realtime.instructions.md` |
| QtPyVCP UI generation | `qtpyvcp-ui-generator` skill |
| Submodule management | `submodules.instructions.md` |
| Requirements elicitation/analysis | `RequirementsAnalyst` agent |
| Architecture decisions (ADRs) | `ArchitectureStrategist` agent |
| TDD implementation | `TDDDriver` agent |
| Testing/verification | `TestingSpecialist` agent |
| Documentation updates | `DocumentationExpert` agent |

## Documentation Rules

- Update existing documentation rather than creating new files.
- Keep documentation consistent with implemented features; never document speculative behavior.
- Consolidate: integrate new content into the authoritative document, then remove the temporary artifact.

