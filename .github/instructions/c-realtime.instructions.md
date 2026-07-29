---
description: >
  PoKeysHal C/RT quick-reference: file responsibilities, build commands, and
  project-specific conventions for C, header, and RT HAL component files.
applyTo: "**/*.c,**/*.h,experimental/**"
---

## PoKeysHal AI Coding Agent Instructions

### Big Picture Architecture
- The project implements a LinuxCNC HAL-compatible driver for PoKeys devices, supporting both userspace and realtime (RT) operation.
- Core architecture uses an asynchronous request/response mailbox system for non-blocking communication. See `Pokeys Async Overview.md` and `README.md` for design rationale and workflow diagrams.
- Major components:
  - `experimental/`: RT HAL components and async logic
  - `pokeyslib/`: User-space reference implementation (feature parity required)
  - `hal-canon/`: Canonical HAL helpers for analog/digital/encoder
  - Protocol details in `PoKeys - protocol specification.pdf`

> ⚠️ **Architecture Enforcement**: See `.github/instructions/pokeyshal-architecture.instructions.md` for the authoritative, detailed architecture rules that ALL agents MUST follow. The rules below are a summary.

### Layered Architecture — File Responsibilities (Summary)

Each file has a strict role. Violations must be corrected and tracked as GitHub issues:

| File / Pattern | Role | Key Constraint |
|---|---|---|
| `PoKeysLibHal.h` | HAL-conform structs using `hal_*` types + canonical interfaces | Use `hal_u32_t`, `hal_s32_t`, `hal_float_t`, `hal_bit_t`; expand bitfields to `hal_bit_t`; include `hal_digin_t`, `hal_digout_t`, etc. |
| `PoKeysLibAsync.h` | Async infrastructure declarations (enums, structs, function decls) | Contains `pokeys_command_t`, `mailbox_entry_t`, all `PK_*Async()` declarations |
| `PoKeysLibAsync.c` | **Async infrastructure ONLY** — mailbox, `PK_ReceiveAndDispatch`, `PK_TimeoutAndRetryCheck`. No subsystem logic, no HAL exports, no command parsers. | `CreateRequestAsync()`, `SendRequestAsync()`, `PK_ReceiveAndDispatch()`, `PK_TimeoutAndRetryCheck()` |
| `PoKeysLib**Async.c` | **Subsystem async** (≠ `PoKeysLibAsync.c`): async impl of `PoKeysLib**.c` + Send/Parse split + HAL export + RT registration | MUST have: `export_<subsystem>_pins()`, Send (`PK_**GetAsync()`), Parse callback (`PK_**Parse()` invoked by `PK_ReceiveAndDispatch`), optional `register_<subsystem>_tasks()` |
| `experimental/pokeys_async.c` | LinuxCNC HAL component integration shell ONLY | MUST contain: `export()`, `EXTRA_SETUP()`, `user_mainloop()`, `FUNCTION(_)`. MUST NOT contain: struct/enum defs, `#define` constants, direct `hal_pin_*_new()` calls, `export_*_pins()` definitions |
| `experimental/async_scheduler.h/.c` | Async scheduling (migrate to `PoKeysLibAsync` when ready) | Content should be migrated to `PoKeysLibAsync.h/.c` |
| `hal-canon/hal_canon.h` + `hal-canon/*.c` | Canonical HAL device interface definitions | Always use `hal_export_digin/digout/adcin/adcout/encoder()` — never call `hal_pin_*_new()` directly for these |

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
- Protocol implementation must match `PoKeys - protocol specification.pdf`. Use Markitdown MCP server to read and verify protocol details.
- **No magic numbers**: All command codes must use `pokeys_command_t` enum, all bit masks must be named constants in `PoKeysLibAsync.h`.
- **HAL pin exports**: Use standardized error handling pattern (see `.github/skills/convert-to-hal-rtapi/`).
- **Async conversions**: Follow the `convert-to-hal-rtapi` skill (`.github/skills/convert-to-hal-rtapi/SKILL.md`) for converting synchronous subsystems to async RT-capable implementations.
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

### Skills and Guides

#### convert-to-hal-rtapi Skill
For converting synchronous PoKeysLib subsystems to asynchronous RT-capable HAL implementations:

- **Skill Definition**: `.github/skills/convert-to-hal-rtapi/SKILL.md`
  - Structured step-by-step conversion process
  - Protocol verification workflow using Markitdown MCP server
  - HAL pin export patterns with error handling
  - Real-time safety constraints and quality checklist

**Key Principles from Skill**:
1. ✅ **Protocol Verified** - Always cross-check with `PoKeys - protocol specification.pdf` using Markitdown MCP server
2. ✅ **No Magic Numbers** - All command codes, bit masks, and byte offsets must be named constants in `PoKeysLibAsync.h`
3. ✅ **HAL Pin Pattern** - Every pin/parameter export follows standardized error handling pattern
4. ✅ **RT Safe** - No blocking, no allocation, bounded execution
5. ✅ **Self-Documenting** - Named constants from `pokeys_command_t` enum, clear structure

### Task Specification
- Use comments to clarify intent, constraints, and completion criteria.
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
