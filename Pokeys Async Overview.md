# PoKeysLib Asynchronous Request/Response Extension

## Goal

Refactor the previous synchronous communication logic in PoKeysLib to a **realtime-safe asynchronous** request/response architecture.

## Motivation

- Avoid blocking network wait times in the realtime thread.
- Clean separation of sending, receiving, and processing.
- Timeout and retry management per packet.
- Direct filling of HAL pins or target variables immediately upon receiving.
- Clear and traceable architecture for realtime systems like LinuxCNC.

---

## New Main Functions

### CreateRequestAsync

Prepares a request packet and creates a mailbox entry.

```c
int CreateRequestAsync(pokeys_device_t *dev, pokeys_command_t cmd,
                       const uint8_t *params, size_t params_len,
                       void *target_ptr, size_t target_size);
```

**Responsibilities:**

- Build the UDP packet (header, command, request ID, checksum).
- Create an entry in the mailbox buffer.
- Store optional target pointer and size for automatic payload filling.
- No network activity yet.

---

### SendRequestAsync

Sends a prepared request and sets the timestamp.

```c
int SendRequestAsync(pokeys_device_t *dev, uint8_t request_id);
```

**Responsibilities:**

- Trigger UDP `sendto()` for the prepared request.
- Set `timestamp_sent`.
- Update retry counter.

---

### PK\_ReceiveAndDispatch

Receives incoming UDP packets and matches them to requests.

```c
int PK_ReceiveAndDispatch(pokeys_device_t *dev);
```

**Responsibilities:**

- Read UDP packet in non-blocking mode.
- Extract request ID.
- Find matching mailbox entry.
- Write response data to `target_ptr` automatically if set.
- Set `response_ready = true`.

**Clarification:**

- **Automatic response data filling:** If `target_ptr` is defined in the mailbox entry, the payload from the response (starting at byte 8) is directly copied to `target_ptr`. No manual processing function is needed afterward.

---

### PK\_TimeoutAndRetryCheck

Periodically executes timeout and retry logic for open requests.

```c
void PK_TimeoutAndRetryCheck(pokeys_device_t *dev, uint64_t timeout_us);
```

**Responsibilities:**

- Check time since `timestamp_sent`.
- If timeout: resend or mark as error.

---

## New Data Structure: Mailbox Entry

```c
typedef struct {
    uint8_t request_id;
    pokeys_command_t command_sent;
    uint64_t timestamp_sent;
    int retries_left;
    bool response_ready;

    void *target_ptr; // Pointer where payload will be auto-copied if response received
    size_t target_size;

    uint8_t request_buffer[64];
    uint8_t response_buffer[64];
} mailbox_entry_t;
```

---

## Workflow Diagram

```plaintext
1. CreateRequestAsync()
   - Build packet
   - Create mailbox entry (set target_ptr if needed)

2. SendRequestAsync()
   - Send packet (UDP sendto)
   - Set timestamp

3. PK_ReceiveAndDispatch()
   - Read response packet
   - Extract request ID
   - Find matching mailbox entry
   - If target_ptr set, copy response payload into memory automatically
   - Set response_ready

4. PK_TimeoutAndRetryCheck()
   - Check all open requests
   - If timeout: retry or mark error
```

---

## Advantages of the New Structure

- Complete decoupling of sending and receiving.
- Realtime-safe behavior in the servo thread.
- Flexible handling of timeouts and communication errors.
- **Automatic filling of HAL variables or target memory without manual parsing.**
- Simplified extension for new commands.
- Direct, efficient filling of HAL pins without extra copy steps.

---

## Notes

- Operate UDP socket in **non-blocking** mode.
- Protect mailbox memory if needed (e.g., with `mlockall()`).
- Manage request IDs cyclically over 0–255.
- Keep receive processing as lightweight as possible.

---

## Status

With this refactoring, PoKeysLib's Ethernet communication will be fully compatible with realtime requirements of LinuxCNC and similar hard realtime systems.

