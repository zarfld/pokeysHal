# PoKeysLib Realtime Refactoring Design

## Objective

Optimization of the PoKeysLib Ethernet/UDP communication for hard realtime use cases in LinuxCNC.

- Separation of Send and Receive.
- No blocking operations inside Realtime Threads.
- Matching of Responses to Requests via Command Matching.
- Direct filling of HAL Pins using response handlers.
- Timeout detection and automatic retries.

## Architecture Approach

### 1. Mailbox / Request Buffer

Each sent Request is stored in a mailbox buffer:

```c
typedef struct {
    pokeys_command_t command_sent;    // Sent Command
    uint8_t subindex;                  // Optional: e.g., Encoder number
    uint64_t timestamp_sent;           // Timestamp at sending
    int retries_left;                  // Maximum retries left
    bool response_ready;               // Response already received?

    void *target_ptr;                  // Pointer to HAL Pin or memory
    size_t target_size;                // Expected size of response data
} mailbox_entry_t;
```

### 2. Sending Flow

- `CreateRequest()` creates the packet.
- `SendRequest()` sends the packet via UDP **non-blocking**.
- `mailbox_insert()` registers the Request in the open mailbox.

### 3. Receiving Flow

Separate receiving process, e.g.:

- Cyclic polling (every 1-5 ms)
- Or `select()`-based triggering on UDP events

After reception:

- Extract `command` from the received packet.
- Find matching open request in the mailbox buffer.
- Write response data directly into `target_ptr`.
- Mark `response_ready = true`.

**Clarification:** If `target_ptr` is set, the payload (starting from byte 8) is automatically copied into the destination structure.

### 4. Timeout Management

Periodically (e.g., every cycle):

- Check all open requests:
  - If `(current time - timestamp_sent) > timeout`,
    - and `retries_left > 0`,
      - Resend request.
      - Update `timestamp_sent` and decrement `retries_left`.
    - Otherwise, set error status.

### 5. Command Dispatching (optional)

Specific response processing via function pointers per Command:

```c
typedef int (*pokeys_response_handler_t)(mailbox_entry_t *entry, const uint8_t *rx_buffer);

pokeys_response_handler_t response_handlers[] = {
    [PK_CMD_ENCODER_VALUES_GET] = handle_encoder_response,
    [PK_CMD_DIGITAL_INPUTS_GET] = handle_inputs_response,
    // more Commands
};
```

Upon reception of a packet, the appropriate handler function is automatically called.

### 6. Advantages of the New Structure

- No blocking in Realtime.
- Asynchronous reception and processing of responses.
- HAL pins are directly updated – minimal latency.
- Clean timeout and retry handling.
- Easily extendable by adding new Commands and Handlers.

---

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

---

## Notes

- Configure UDP socket **non-blocking**.
- Reserve memory for HAL Pins and Mailbox safely via `mlockall()` or HAL-safe methods.
- Strict separation between send and receive flows.
- Minimal protection (e.g., atomic flags) when accessing shared mailbox structures.

