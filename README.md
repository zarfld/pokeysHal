# 📄 PoKeysLibHal

## Purpose

This project provides an optimized PoKeysLib UDP/Ethernet communication layer for **hard realtime use** (LinuxCNC).

- Asynchronous non-blocking communication
- Automatic mapping of device responses into HAL-safe memory
- Clean timeout and retry handling
- Realtime-compatible design with minimal CPU usage

## Architecture

- **Mailbox System**: Each outgoing request is stored with a `target_ptr`.
- **Send Thread**: Sends prepared packets without blocking.
- **Receive Thread**: Matches responses automatically to pending requests and fills `target_ptr` directly.
- **Timeout Checker**: Retries or flags errors on missing responses.
- **No Manual Parsing**: Response payloads are copied automatically into preassigned structures.

## Key Features

- No blocking operations in realtime threads
- Fully fits LinuxCNC HAL types (`hal_s32_t`, `hal_bit_t`, `hal_float_t`)
- Easily extendable for additional device features
- Fast, safe, and predictable response handling
- Clear separation of send and receive flow

## Security Commands

The library now supports PoKeys security management:

- **PK_SecurityStatusGet** (command `0xE1`)
  - Request: no parameters.
  - Response: byte 9 contains the current security level, bytes 10–41 contain a 32-byte seed used when hashing the password.
- **PK_UserAuthorise** (command `0xE2`)
  - Request: param1 is desired security level; payload is 20-byte SHA‑1 password hash.
  - Response: byte 9 is the unlock status (`0xAA` if successful).
- **PK_UserPasswordSet** (command `0xE3`)
  - Request: param1 is default security level; payload is the password in plain text (32 bytes).
  - Response: header only (no additional data).

## Flowchart

```plaintext
[RT-Thread]                   [Background Receive]
    |                                  |
    | --> Create & Send Request        |
    | --> Insert Mailbox Entry         |
    |                                  |<-- UDP Packet arrives
    |                                  |--> Find matching Mailbox Entry
    |                                  |--> Write Response to target_ptr
    |                                  |--> Mark response_ready = true
    |
    | --> Poll Mailbox Entries
    | --> If response_ready, process data
    | --> If timeout, retry or set error
```

## Notes

- UDP Socket must operate **non-blocking**.
- Memory for mailbox and HAL data should be locked (`mlockall()` recommended).
- No dynamic memory allocation inside realtime threads.

---
## Async Core Routines

- **PK_EnumerateUSBDevicesAsync**
  - Non-blocking enumeration of USB devices.
  - Call repeatedly with a persistent `PKUSBEnumerator` until the return value
    is non-negative. `-2` indicates enumeration is still in progress; `-1`
    reports an error.
  - When finished the function returns the number of detected devices.

- **PK_EnumerateUSBDevices**
  - Blocking enumeration variant that scans all USB interfaces in one call.
  - Returns the number of PoKeys devices found.

- **PK_GetCurrentDeviceConnectionType**
  - `device` pointer describing the PoKeys instance.
  - Returns the currently active connection type constant
    (USB, FastUSB or Network).

- **CreateRequest**
  - Formats a 64‑byte request buffer with the command ID and four optional
    parameters. No I/O is performed.
  - Parameters `type`, `param1..param4` occupy bytes 1‑5 of the buffer.

- **PK_CustomRequest**
  - Convenience helper which fills `device->request` using `CreateRequest` and
    immediately invokes `SendRequest`.
  - Parameters mirror those of `CreateRequest`.

- **SendRequest_multiPart**
  - Transmits a prepared `PK_CMD_MULTIPART_PACKET` using either UDP or FastUSB
    depending on the device connection type.
  - Expects the multipart payload to be prepared in `device` beforehand.

- **SendRequest**
  - Finalises the current request buffer (header, request ID, checksum), sends it
    and waits for the matching reply. The call blocks until a response is
    received or a timeout occurs.

- **SendRequest_NoResponse**
  - Sends the formatted packet and returns immediately without waiting for any
    reply. Useful for commands that acknowledge by other means.

## Device Information Routines

- **CompareName**
  - Utility to test whether a device name begins with a given prefix.
- **PK_DeviceDataGet**
  - Reads firmware and hardware identifiers, populating `device->DeviceData` and `device->info`.
- **PK_FillPWMPinNumbers**
  - Populates `device->PWM.PWMpinIDs` with the PWM-capable pins for the current model.
- **PK_DeviceNameSet**
  - Sends the name stored in `device->DeviceData.DeviceName` to the device.
- **PK_NetworkConfigurationSet**
  - Writes IP address, subnet and gateway fields from `device->netDeviceData`.
- **PK_DeviceActivation**
  - Uploads the activation code and stores the resulting option mask.
- **PK_DeviceActivationClear**
  - Clears any stored activation information on the device.
- **PK_SaveConfiguration**
  - Persists the current configuration in device flash.
- **PK_ClearConfiguration**
  - Resets configuration to defaults without clearing the host structure.
- **PK_CheckPinCapabilityByDevice**
  - Checks whether a pin supports a capability for a given device type mask.
- **PK_CheckPinCapabilityByTypeID**
  - Same as above but uses an exact device type ID.
- **PK_CheckPinCapability**
  - Convenience wrapper for the connected device instance.
- **PK_CheckPinEnabledCapability**
  - Placeholder to query if a capability is actually enabled (currently returns `0`).
- **PK_GetDebugValues**
  - Retrieves a table of diagnostic counters into the provided buffer.
- **PK_SetFastUSBEnableStatus**
  - Enables or disables the FastUSB interface.
- **PK_GetFastUSBEnableStatus**
  - Reads back the FastUSB enable flag.
- **PK_ReadDeviceLog**
  - Fetches the device's internal log entries.

## Real-Time Clock Functions

- **PK_RTCGet**
  - Reads the current time from the device and stores it in the `device->RTC` structure fields.
- **PK_RTCSet**
  - Sends the values from `device->RTC` to update the device's internal clock.

## Matrix Keyboard Functions

- **PK_MatrixKBConfigurationGet**
  - Reads matrix keyboard configuration and key mapping into `device->matrixKB`.
- **PK_MatrixKBConfigurationSet**
  - Writes configuration and key mapping from `device->matrixKB` back to the device.
- **PK_MatrixKBStatusGet**
  - Updates `device->matrixKB.matrixKBvalues` with the currently pressed keys.

## Matrix LED Functions

- **PK_MatrixLEDConfigurationSet**
  - Sends the enable flags and geometry from `device->MatrixLED` to the device.
- **PK_MatrixLEDConfigurationGet**
  - Reads back the display configuration into `device->MatrixLED`.
- **PK_MatrixLEDUpdate**
  - Transfers pixel data for displays where `RefreshFlag` is set.

## Asynchronous IO Wrappers

- **PK_PinConfigurationGetAsync**
  - Asynchronous counterpart to `PK_PinConfigurationGet`.
  - Queues the same requests using `CreateRequestAsync` and returns immediately.

- **PK_PinConfigurationSetAsync**
  - Asynchronous version of `PK_PinConfigurationSet`.
  - Builds the configuration packets and schedules them without waiting.

- **PK_DigitalIOSetAsync**
  - Non-blocking wrapper around `PK_DigitalIOSet`.
  - Prepares the digital output payload and enqueues it via `CreateRequestAsync`.
- **PK_LCDConfigurationGetAsync / SetAsync / UpdateAsync**
  - New helpers for the character LCD.
  - Use the same async request queue to avoid blocking the realtime thread.
