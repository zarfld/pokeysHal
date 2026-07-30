---
description: >
  PoKeysHal C/RT architecture invariants and real-time constraints. Applies to all C
  source files, headers, and experimental HAL components. Defines file responsibilities,
  RT prohibitions, HAL type rules, protocol constants, and logging conventions.
applyTo: "**/*.c,**/*.h,experimental/**"
---

# PoKeysHal C/RT Architecture and Real-Time Constraints

## File Responsibilities

Each file has a strict role. Violations must be reported and tracked.

| File / Pattern | Role | Must NOT contain |
|---|---|---|
| `PoKeysLibHal.h` | HAL-compatible data structures | Raw C types in HAL-visible members; function implementations; async declarations |
| `PoKeysLibAsync.h` | Shared async contracts: enums, masks, offsets, `PK_*Async()` declarations | HAL export logic; subsystem parsers; implementations |
| `PoKeysLibAsync.c` | Shared async transport: mailbox, `PK_ReceiveAndDispatch`, `PK_TimeoutAndRetryCheck`, retry | Subsystem protocol logic; HAL exports; subsystem parsers; direct `hal_pin_*_new()` calls |
| `PoKeysLib**Async.c` | Per-subsystem async: `export_<subsystem>_pins()`, Send (`PK_**GetAsync()`), Parse callback (`PK_**Parse()`), optional `register_<subsystem>_tasks()` | Shared transport logic; code that belongs in `PoKeysLibAsync.c` |
| `experimental/pokeys_async.c` | Integration shell: `export()`, `EXTRA_SETUP()`, `user_mainloop()`, `FUNCTION(_)` | Struct/enum definitions; `#define` constants; direct `hal_pin_*_new()` calls; `export_*_pins()` definitions |
| `hal-canon/hal_canon.h` + `hal-canon/*.c` | Implements canonical HAL channel export helpers; direct `hal_pin_*_newf()` calls are permitted inside these helpers | PoKeys subsystem logic; protocol request/response handling |

Do not solve a local problem by violating these boundaries. When existing code already violates a boundary, do not copy the violation — contain the change, report it, and correct it when necessary.

Subsystem invariant: `PoKeysLib**Async.c` files must use `hal_export_digin()`, `hal_export_digout()`, `hal_export_adcin()`, `hal_export_adcout()`, or `hal_export_encoder()` for canonical channels. Do not call `hal_pin_bit_newf()` or other `hal_pin_*_newf()` variants directly for channels that have a canonical helper.

## HAL-Compatible Types

All HAL-visible struct members in `PoKeysLibHal.h` must use:
- `hal_u32_t`, `hal_s32_t` for unsigned/signed 32-bit integers
- `hal_float_t` for floating-point
- `hal_bit_t` for boolean/digital
- Expand bitfields to individual `hal_bit_t` members

Never use raw `int`, `float`, `bool`, or `uint32_t` for HAL-exposed fields.

## Real-Time Prohibitions

Code reachable from a LinuxCNC real-time function must remain deterministic and bounded.

Do not introduce in RT paths:
- `malloc`, `free`, or any dynamic allocation after RT execution begins
- Blocking device communication or synchronous PoKeys requests
- Blocking socket operations (UDP socket must use `O_NONBLOCK`)
- Mutex waits, condition variables, or blocking synchronization
- File access, `sleep`, or any blocking system call
- Unbounded loops or uncontrolled retry logic
- Expensive logging in a high-frequency path

For userspace real-time processes, call `mlockall(MCL_CURRENT | MCL_FUTURE)` during initialization before entering the real-time execution loop. This requirement does not apply to kernel-module code.

An async function is not RT-safe merely because its name ends in `Async`. Verify the complete call path.

## Protocol Constants

- All PoKeys command codes must use the `pokeys_command_t` enum — no numeric literals.
- All bit masks must be named constants defined in `PoKeysLibAsync.h`.
- All response byte offsets must be named constants — no magic offset numbers.
- Verify command codes, payload positions, response offsets, and status bits against the PoKeys protocol specification before implementing or modifying any command.
- Validate response lengths before reading payload fields.

## RT Logging and Conditional Compilation

- Use `rtapi_print_msg` with the appropriate severity level. Avoid per-cycle logging in high-frequency RT paths. Never use `printf` or `fprintf` from an RT path.
- Wrap RT-specific code in `#ifdef RTAPI` / `#endif`.
- Keep ISR/RT functions under 50 µs (soft RT) or 5 µs (hard RT).

## Timing

- Do not claim timing compliance from code inspection alone.
- Timing compliance requires measurement in the applicable runtime environment with an identified method and acceptance threshold.

## Skill Reference

For the full async subsystem conversion procedure (request creation, response parsing, HAL export, protocol verification), use the `convert-to-hal-rtapi` skill (`.github/skills/convert-to-hal-rtapi/SKILL.md`).
