# Matrix Keyboard Commands

This document summarises the helper functions implemented in **PoKeysLibMatrixKB.c**.
All operations issue subcommands of `PK_CMD_MATRIX_KEYBOARD_CFG` (`0xCA`).

## Command reference

### PK_MatrixKBConfigurationGet
* **Subcommand**: `10`
* **Response fields**: returns keyboard dimensions, pin assignments and macro mapping options.
* **Additional fetches**: subcommands `12`‑`19` and `32`‑`39` retrieve key codes and optional triggered mappings.

### PK_MatrixKBConfigurationSet
* **Subcommands**: `1`, `50`, `2`‑`9` and `22`‑`29`.
* **Request payload**: configuration values from `sMatrixKeyboard` including pin lists, macro mapping and key codes.
* **Final step**: resend subcommand `1` with the activation flag enabled.

### PK_MatrixKBStatusGet
* **Subcommand**: `20`
* **Response fields**: bytes 8‒23 contain a bit mask for the pressed keys which updates `matrixKBvalues`.

## Asynchronous API

For realtime tasks the following wrappers in `PoKeysLibMatrixKBAsync.c` perform
the same configuration and polling steps without blocking. Each call schedules
the UDP request using `CreateRequestAsync` so only negligible CPU time is spent
in the caller:

- `PK_MatrixKBConfigurationGetAsync`
- `PK_MatrixKBConfigurationSetAsync`
- `PK_MatrixKBStatusGetAsync`

The background dispatcher updates the device structure when packets arrive,
ensuring minimal latency and consistent non-blocking behaviour.
