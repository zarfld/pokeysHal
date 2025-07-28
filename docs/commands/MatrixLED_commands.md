# Matrix LED Commands

This document summarises the helper functions implemented in **PoKeysLibMatrixLED.c**.
All operations use `PK_CMD_MATRIX_LED_CONFIGURATION` (`0xD5`) or `PK_CMD_MATRIX_LED_UPDATE` (`0xD6`).

## Command reference

### PK_MatrixLEDConfigurationSet
* **Command ID**: `PK_CMD_MATRIX_LED_CONFIGURATION (0xD5)` with subcommand `0`.
* **Request payload**: enable flags, row counts and column counts taken from `sPoKeysMatrixLED`.

### PK_MatrixLEDConfigurationGet
* **Command ID**: `PK_CMD_MATRIX_LED_CONFIGURATION (0xD5)` with subcommand `1`.
* **Response fields**: returns display enable bits and geometry for each matrix display.

### PK_MatrixLEDUpdate
* **Command ID**: `PK_CMD_MATRIX_LED_UPDATE (0xD6)`.
* **Request payload**: eight bytes of pixel data from `sPoKeysMatrixLED.data` for each display with a set refresh flag.

## Asynchronous API

`PoKeysLibMatrixLEDAsync.c` provides non-blocking wrappers for realtime use. Calls return immediately after queueing the UDP packet so the caller never waits for network activity.

- `PK_MatrixLEDConfigurationGetAsync`
- `PK_MatrixLEDConfigurationSetAsync`
- `PK_MatrixLEDUpdateAsync`

The dispatcher updates `sPoKeysMatrixLED` when responses arrive, ensuring deterministic realtime behaviour without blocking the thread that requests an update.
