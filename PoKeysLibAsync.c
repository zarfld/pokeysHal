#include "PoKeysLibAsync.h"
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

extern uint64_t get_current_time_us(void); // Your system's high-res timer
extern uint8_t next_request_id(void); // Request ID generator
extern async_transaction_t transactions[MAX_TRANSACTIONS];

/**
 * @brief Allocates a new free transaction.
 *
 * @return Pointer to an empty async_transaction_t, or NULL if none available.
 */
async_transaction_t* transaction_alloc(void)
{
    for (int i = 0; i < MAX_TRANSACTIONS; i++) {
        if (transactions[i].status == TRANSACTION_COMPLETED ||
            transactions[i].status == TRANSACTION_TIMEOUT ||
            transactions[i].status == TRANSACTION_FAILED ||
            transactions[i].request_id == 0) {
            // Reset transaction
            memset(&transactions[i], 0, sizeof(async_transaction_t));
            transactions[i].status = TRANSACTION_PENDING;
            return &transactions[i];
        }
    }
    return NULL; // No available slot
}

uint64_t get_current_time_us(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return ((uint64_t)tv.tv_sec * 1000000ULL) + tv.tv_usec;
}

static uint8_t current_request_id = 0;

uint8_t next_request_id(void)
{
    current_request_id = (current_request_id + 1) & 0xFF;
    if (current_request_id == 0)
        current_request_id = 1; // Optionally skip 0
    return current_request_id;
}

/**
 * @brief Finds an open transaction by its Request ID.
 *
 * @param request_id The Request ID to search for.
 * @return Pointer to matching async_transaction_t or NULL if not found.
 */
async_transaction_t* transaction_find(uint8_t request_id)
{
    for (int i = 0; i < MAX_TRANSACTIONS; i++) {
        if (transactions[i].request_id == request_id &&
            transactions[i].status == TRANSACTION_PENDING) {
            return &transactions[i];
        }
    }
    return NULL; // Not found
}

/**
 * @brief Prepares an asynchronous request (non-sending).
 *
 * @param dev Pointer to the PoKeys device (not used directly for building but might be logged or expanded later).
 * @param cmd Command to be sent (Operation ID).
 * @param params Optional pointer to up to 4 bytes of parameters.
 * @param params_len Length of parameters (should be 0-4).
 * @param target_ptr Pointer where decoded response data should be written.
 * @param target_size Size of the target buffer.
 * @return Request ID on success, negative error code on failure.
 */
int CreateRequestAsync(sPoKeysDevice *dev, pokeys_command_t cmd,
    const uint8_t *params, size_t params_len,
    void *target_ptr, size_t target_size)
{
async_transaction_t *t = transaction_alloc();
if (!t)
return -1; // No free slot available

uint8_t req_id = next_request_id();

// Build basic request packet
memset(t->request_buffer, 0, sizeof(t->request_buffer));
t->request_buffer[0] = 0xBB;                 // Start byte (Host to Device)
t->request_buffer[1] = (uint8_t)cmd;          // Operation/Command ID

// Insert optional parameters (max 4 bytes into bytes 2–5)
if (params && params_len > 0 && params_len <= 4) {
memcpy(&t->request_buffer[2], params, params_len);
}

t->request_buffer[6] = req_id;                // Request ID (7th byte)

// Calculate checksum (sum of first 7 bytes mod 256)
uint8_t checksum = 0;
for (int i = 0; i <= 6; i++) {
checksum += t->request_buffer[i];
}
t->request_buffer[7] = checksum;              // Checksum byte

// Fill transaction metadata
t->request_id = req_id;
t->command_sent = cmd;
t->status = TRANSACTION_PENDING;
t->retries_left = 2;                          // Example: allow 2 retries (total 3 attempts)
t->timestamp_sent = 0;                        // Will be set on first send

t->target_ptr = target_ptr;
t->target_size = target_size;

// (response_buffer will be filled when receiving, no need to touch here)

return req_id;
}

/**
 * @brief Sends an asynchronous request that was prepared earlier.
 *
 * @param dev Pointer to the PoKeys device structure.
 * @param request_id Request ID of the transaction to send.
 * @return 0 on success, negative error code on failure.
 */
int SendRequestAsync(sPoKeysDevice *dev, uint8_t request_id)
{
    async_transaction_t *t = transaction_find(request_id);
    if (!t) {
        return -1; // No matching pending transaction found
    }

    // Send the packet
    ssize_t sent = sendto(dev->sockfd,
                          t->request_buffer, sizeof(t->request_buffer), 0,
                          (struct sockaddr *)&dev->addr, sizeof(dev->addr));
    if (sent < 0) {
        return -2; // Send error
    }

    // Update timestamp after successful send
    t->timestamp_sent = get_current_time_us(); // Microseconds timer (extern function you should provide)

    return 0; // Success
}

/**
 * @brief Receives UDP packets and dispatches them to the correct async transaction.
 *
 * @param dev Pointer to the PoKeys device structure.
 * @return Number of responses processed, or 0 if none.
 */
int PK_ReceiveAndDispatch(sPoKeysDevice *dev)
{
    uint8_t rx_buffer[64];
    ssize_t len;
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);

    // Non-blocking UDP receive
    len = recvfrom(dev->sockfd, rx_buffer, sizeof(rx_buffer),
                   MSG_DONTWAIT, (struct sockaddr *)&addr, &addrlen);

    if (len <= 0)
        return 0; // No packet available or recv error (EAGAIN/EWOULDBLOCK)

    // Check valid PoKeys response
    if (rx_buffer[0] != 0xAA) {
        return -1; // Invalid start byte (should be 0xAA from Device → Host)
    }

    uint8_t cmd = rx_buffer[1];  // Command echoed back
    uint8_t req_id = rx_buffer[6]; // Request ID echoed back

    // Find the corresponding async transaction
    async_transaction_t *t = transaction_find(req_id);
    if (!t) {
        return -2; // No matching open request
    }

    // Store full response buffer (optional)
    memcpy(t->response_buffer, rx_buffer, sizeof(t->response_buffer));

    // Write result directly into target_ptr if set
    if (t->target_ptr && t->target_size > 0) {
        memcpy(t->target_ptr, &rx_buffer[8], t->target_size);
        // Note: Response payload starts at byte 9 (index 8) according to PoKeys protocol
    }

    t->status = TRANSACTION_COMPLETED;
    t->response_received = true;

    // (Optional) You could immediately free/recycle the transaction here if desired
    // transaction_free(t);

    return 1; // One response processed
}

/**
 * @brief Periodically checks for request timeouts and retries if necessary.
 *
 * @param dev Pointer to the PoKeys device structure.
 * @param timeout_us Timeout threshold in microseconds per attempt.
 */
void PK_TimeoutAndRetryCheck(sPoKeysDevice *dev, uint64_t timeout_us)
{
    uint64_t now = get_current_time_us();

    for (int i = 0; i < MAX_TRANSACTIONS; i++) {
        async_transaction_t *t = &transactions[i];

        if (t->status == TRANSACTION_PENDING) {
            if ((now - t->timestamp_sent) > timeout_us) {
                if (t->retries_left > 0) {
                    // Retry sending
                    ssize_t sent = sendto(dev->sockfd,
                                          t->request_buffer, sizeof(t->request_buffer), 0,
                                          (struct sockaddr *)&dev->addr, sizeof(dev->addr));
                    if (sent >= 0) {
                        t->timestamp_sent = now; // Update timestamp after successful resend
                        t->retries_left--;
                        // Optionally you could log retries here
                    } else {
                        // (Optional) If sendto failed, maybe log or increment error counter
                    }
                } else {
                    // No retries left: mark timeout
                    t->status = TRANSACTION_TIMEOUT;
                    // Optionally clear or free transaction here
                    // transaction_free(t); // if immediate cleanup desired
                }
            }
        }
    }
}