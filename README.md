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