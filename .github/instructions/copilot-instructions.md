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