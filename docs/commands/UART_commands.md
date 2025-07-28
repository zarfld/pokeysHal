# UART Commands

This document summarises the helper functions implemented in **PoKeysLibUART.c**. All operations use command `PK_CMD_UART_COMMUNICATION` (`0xDE`).

## Command reference

### PK_UARTConfigure
* **Subcommand**: `0x10`
* **Request payload**: baud rate as a 32-bit value while `interfaceID` and `format` select the port and frame format.

### PK_UARTWrite
* **Subcommand**: `0x20`
* **Request payload**: up to 55 bytes from `dataPtr` are transmitted in each call.
* **Response fields**: the device returns the number of bytes actually written.

### PK_UARTRead
* **Subcommand**: `0x30`
* **Response fields**: byte count followed by the received data which is copied to `dataPtr`.


## Asynchronous API

For realtime applications the following wrappers perform the same UART
operations without blocking the caller. Each function schedules a request
using `CreateRequestAsync` and returns immediately while the transfer
is completed by the background send/receive threads.

- `PK_UARTConfigureAsync`
- `PK_UARTWriteAsync`
- `PK_UARTReadAsync`

These are fully compatible with hard realtime threads because no network
waits occur during the call.
