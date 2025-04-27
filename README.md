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