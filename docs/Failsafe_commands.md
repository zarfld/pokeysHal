# Failsafe Settings Commands

This document summarises the helper functions implemented in **PoKeysLibFailsafe.c**. All operations use command `0x81`.

## Command reference

### PK_FailsafeSettingsGet
* **Subcommand**: `0`
* **Response fields**: returns timeout, peripheral enable bits and output values stored in `sPoKeysFailsafeSettings`.

### PK_FailsafeSettingsSet
* **Subcommand**: `1`
* **Request payload**: settings from `sPoKeysFailsafeSettings` are written back to the device.

## Asynchronous API

For realtime tasks the following wrappers in `PoKeysLibFailsafeAsync.c` perform
the same operations without waiting for network I/O. Each call schedules the
request with `CreateRequestAsync` and returns immediately.

- `PK_FailsafeSettingsGetAsync`
- `PK_FailsafeSettingsSetAsync`
