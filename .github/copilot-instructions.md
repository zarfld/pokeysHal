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
- `PoKeysLibAsync.c` — async transport/mailbox infrastructure only (no subsystem logic)
- `PoKeysLib**Async.c` — per-subsystem async: HAL export, send, parse, optional scheduler registration
- `experimental/pokeys_async.c` — integration shell only
- `PoKeysLibHal.h` — HAL-compatible structs; `PoKeysLibAsync.h` — shared async contracts
- `hal-canon/` — canonical HAL channel interfaces
- `docs/` — project documentation; `.github/instructions/` — Copilot instructions

> File responsibilities, RT constraints, HAL types, and protocol rules: `c-architecture-realtime.instructions.md`

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

