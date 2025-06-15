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

