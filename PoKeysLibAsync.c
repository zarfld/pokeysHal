#include "PoKeysLibAsync.h"
#include <string.h>
#ifndef RTAPI
#include <time.h>
#else
#include "rtapi.h"
#endif
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>

extern uint64_t get_current_time_us(void); // Your system's high-res timer
extern uint8_t next_request_id(void); // Request ID generator
static async_transaction_t pk_transactions[MAX_TRANSACTIONS];

/**
 * @brief Allocates a new free transaction.
 *
 * @return Pointer to an empty async_transaction_t, or NULL if none available.
 */
async_transaction_t* transaction_alloc(void)
{
    for (int i = 0; i < MAX_TRANSACTIONS; i++) {
        if (pk_transactions[i].status == TRANSACTION_COMPLETED ||
            pk_transactions[i].status == TRANSACTION_TIMEOUT ||
            pk_transactions[i].status == TRANSACTION_FAILED ||
            pk_transactions[i].request_id == 0) {
            // Reset transaction
            memset(&pk_transactions[i], 0, sizeof(async_transaction_t));
            pk_transactions[i].status = TRANSACTION_PENDING;
            return &pk_transactions[i];
        }
    }
    return NULL; // No available slot
}

uint64_t get_current_time_us(void)
{
    #ifndef RTAPI
    struct timeval tv;
    gettimeofday(&tv, NULL);
     return ((uint64_t)tv.tv_sec * 1000000ULL) + tv.tv_usec;
    #else
    return rtapi_get_time() / 1000;  // convert ns → µs
   
   #endif
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
        if (pk_transactions[i].request_id == request_id &&
            pk_transactions[i].status == TRANSACTION_PENDING) {
            return &pk_transactions[i];
        }
        else if(pk_transactions[i].request_id == request_id){
            // Transaction found but not pending (e.g., completed, failed, or timed out)
            rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: Transaction found but not pending (status: %d)\n",
                __FILE__, __FUNCTION__, pk_transactions[i].status);
            return &pk_transactions[i];
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
    void *target_ptr, size_t target_size,
    int (*parser_func)(sPoKeysDevice *, const uint8_t *))
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
t->response_parser = parser_func; // <=== New! Optional parser function

// (response_buffer will be filled when receiving, no need to touch here)

return req_id;
}

/**
 * @brief Prepares an asynchronous PoKeys request with optional payload data after header.
 *
 * @param device Pointer to PoKeys device
 * @param cmd PoKeys command ID (e.g., 0xC4, 0xC5, 0xCE, etc.)
 * @param params Optional pointer to up to 4 parameter bytes (NULL if none)
 * @param params_len Length of params (max 4 bytes)
 * @param payload Optional pointer to payload (e.g., encoder options) to copy starting at byte 8
 * @param payload_size Size of payload (must fit into remaining packet space)
 * @param parser_func Optional parser function for response (NULL for write-only operations)
 * @return Request ID on success, negative error code on failure
 */
 int CreateRequestAsyncWithPayload(
    sPoKeysDevice *device,
    pokeys_command_t cmd,
    const uint8_t *params,
    size_t params_len,
    const void *payload,
    size_t payload_size,
    pokeys_response_parser_t parser_func
)
{
    if (device == NULL)
        return -1; // Error: No device

    async_transaction_t *t = transaction_alloc();
    if (!t)
        return -2; // No free slot available

    uint8_t req_id = next_request_id();

    // Initialize request buffer
    memset(t->request_buffer, 0, sizeof(t->request_buffer));
    t->request_buffer[0] = 0xBB;            // Start byte
    t->request_buffer[1] = (uint8_t)cmd;     // Command ID

    // Insert parameters
    if (params && params_len > 0 && params_len <= 4) {
        memcpy(&t->request_buffer[2], params, params_len); // params at bytes 2–5
    }

    t->request_buffer[6] = req_id;           // Request ID at byte 6

    // Calculate checksum (sum of first 7 bytes)
    uint8_t checksum = 0;
    for (int i = 0; i <= 6; i++) {
        checksum += t->request_buffer[i];
    }
    t->request_buffer[7] = checksum;         // Checksum at byte 7

    // If payload is present, insert into request_buffer starting at byte 8
    if (payload && payload_size > 0) {
        if (payload_size > (sizeof(t->request_buffer) - 8)) {
            return -3; // Error: Payload too big
        }
        memcpy(&t->request_buffer[8], payload, payload_size);
    }

    // Fill transaction metadata
    t->request_id = req_id;
    t->command_sent = cmd;
    t->status = TRANSACTION_PENDING;
    t->retries_left = 2; // 2 retries allowed
    t->timestamp_sent = 0; // Will be set when sent

    t->target_ptr = NULL; // No response buffer for write commands
    t->target_size = 0;
    t->response_parser = parser_func; // Could be NULL if no response parsing needed

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
    if (!dev) return -1;
    async_transaction_t *t = transaction_find(request_id);
    if (!t) {
        rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: No matching transaction found for request ID %d\n", __FILE__, __FUNCTION__, request_id);
        return -1; // No matching pending transaction found
    }

    // Guard against NULL devHandle (e.g. USB-only device without UDP socket)
    rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: devHandle=%p for request ID %d\n", __FILE__, __FUNCTION__, dev->devHandle, request_id);
    if (!dev->devHandle) {
        rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: devHandle is NULL for request ID %d - skipping send\n", __FILE__, __FUNCTION__, request_id);
        return -1;
    }

    // Send the packet
 //   ssize_t sent = sendto(*(int*)dev->devHandle, t->request_buffer, sizeof(t->request_buffer), 0,(struct sockaddr *)&dev->devHandle2, sizeof(struct sockaddr_in));
 ssize_t sent = sendto(*(int*)dev->devHandle, t->request_buffer, sizeof(t->request_buffer), 0,(struct sockaddr *)dev->devHandle2, sizeof(struct sockaddr_in));
    if (sent < 0) {
        rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: sendto failed for request ID %d, errno=%d (%s)\n", __FILE__, __FUNCTION__, request_id, errno, strerror(errno));
        rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: devHandle2=%d\n", __FILE__, __FUNCTION__,dev->devHandle2);
        struct sockaddr_in *a = (struct sockaddr_in *)&dev->devHandle2;
        rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: → sendto: sin_family=%d, ip=%08x, port=%d\n", __FILE__, __FUNCTION__,a->sin_family, ntohl(a->sin_addr.s_addr), ntohs(a->sin_port));

        return -2; // Send error
    }

    // Update timestamp after successful send
    t->timestamp_sent = get_current_time_us(); // Microseconds timer (extern function you should provide)

    return 0; // Success
}

int CreateAndSendRequestAsync(sPoKeysDevice *dev, pokeys_command_t cmd,
    const uint8_t *params, size_t params_len,
    void *target_ptr, size_t target_size,
    int (*parser_func)(sPoKeysDevice *dev, const uint8_t *response))
{
    int req_id = CreateRequestAsync(dev, cmd, params, params_len,
                                    target_ptr, target_size, parser_func);
    if (req_id < 0)
        return req_id;
    return SendRequestAsync(dev, (uint8_t)req_id);
}

int CreateAndSendRequestAsyncWithPayload(sPoKeysDevice *dev, pokeys_command_t cmd,
    const uint8_t *params, size_t params_len,
    const void *payload, size_t payload_size,
    pokeys_response_parser_t parser_func)
{
    int req_id = CreateRequestAsyncWithPayload(dev, cmd, params, params_len,
                                               payload, payload_size, parser_func);
    if (req_id < 0)
        return req_id;
    return SendRequestAsync(dev, (uint8_t)req_id);
}

/**
 * @brief Receives UDP packets and dispatches them to the correct async transaction.
 *
 * @param dev Pointer to the PoKeys device structure.
 * @return Number of responses processed, or 0 if none.
 */
int PK_ReceiveAndDispatch(sPoKeysDevice *dev)
{
    if (!dev) return 0;

    // Checkpoint 1: entry — log devHandle so the CI trace shows where we are
    // even if the very next instruction segfaults.
    rtapi_print_msg(RTAPI_MSG_ERR,
        "PoKeys: %s:%s: [1] entering, devHandle=%p\n",
        __FILE__, __FUNCTION__, dev->devHandle);

    if (!dev->devHandle) {
        rtapi_print_msg(RTAPI_MSG_ERR,
            "PoKeys: %s:%s: [1a] devHandle is NULL - skipping\n",
            __FILE__, __FUNCTION__);
        return 0;
    }

    int fd = *(int*)dev->devHandle;
    rtapi_print_msg(RTAPI_MSG_ERR,
        "PoKeys: %s:%s: [2] fd=%d - about to recvfrom\n",
        __FILE__, __FUNCTION__, fd);

    uint8_t rx_buffer[64];
    ssize_t len;
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);

    // Non-blocking UDP receive
    len = recvfrom(fd, rx_buffer, sizeof(rx_buffer),
                   MSG_DONTWAIT, (struct sockaddr *)&addr, &addrlen);

    rtapi_print_msg(RTAPI_MSG_ERR,
        "PoKeys: %s:%s: [3] recvfrom returned len=%d (errno=%d)\n",
        __FILE__, __FUNCTION__, (int)len, errno);

    if (len <= 0)
        return 0; // No packet available or recv error (EAGAIN/EWOULDBLOCK)

    // Checkpoint 4: got a packet — log first bytes for protocol check
    rtapi_print_msg(RTAPI_MSG_ERR,
        "PoKeys: %s:%s: [4] rx[0]=0x%02X rx[1]=0x%02X rx[6]=0x%02X\n",
        __FILE__, __FUNCTION__, rx_buffer[0], rx_buffer[1], rx_buffer[6]);

    // Check valid PoKeys response
    if (rx_buffer[0] != 0xAA) {
        rtapi_print_msg(RTAPI_MSG_ERR,
            "PoKeys: %s:%s: [4a] bad start byte 0x%02X - discarding\n",
            __FILE__, __FUNCTION__, rx_buffer[0]);
        return -1; // Invalid start byte (should be 0xAA from Device → Host)
    }

    uint8_t cmd    = rx_buffer[1]; // Command echoed back
    uint8_t req_id = rx_buffer[6]; // Request ID echoed back

    rtapi_print_msg(RTAPI_MSG_ERR,
        "PoKeys: %s:%s: [5] cmd=0x%02X req_id=%u - looking up transaction\n",
        __FILE__, __FUNCTION__, cmd, (unsigned)req_id);

    // Find the corresponding async transaction
    async_transaction_t *t = transaction_find(req_id);

    rtapi_print_msg(RTAPI_MSG_ERR,
        "PoKeys: %s:%s: [6] transaction_find=%p status=%d parser=%p\n",
        __FILE__, __FUNCTION__,
        (void*)t,
        t ? (int)t->status : -1,
        t ? (void*)t->response_parser : NULL);

    if (!t) {
        return -2; // No matching open request
    }

    // Checkpoint 7: copy raw response into transaction slot
    rtapi_print_msg(RTAPI_MSG_ERR,
        "PoKeys: %s:%s: [7] memcpy response_buffer\n",
        __FILE__, __FUNCTION__);
    memcpy(t->response_buffer, rx_buffer, sizeof(t->response_buffer));

    // Write result directly into target_ptr if set
    if (t->target_ptr && t->target_size > 0) {
        rtapi_print_msg(RTAPI_MSG_ERR,
            "PoKeys: %s:%s: [8] memcpy target_ptr=%p size=%zu\n",
            __FILE__, __FUNCTION__, t->target_ptr, t->target_size);
        memcpy(t->target_ptr, &rx_buffer[8], t->target_size);
    }

    // Call optional parser — this is the most likely crash site
    if (t->response_parser) {
        rtapi_print_msg(RTAPI_MSG_ERR,
            "PoKeys: %s:%s: [9] calling parser %p\n",
            __FILE__, __FUNCTION__, (void*)t->response_parser);
        int parse_ret = t->response_parser(dev, rx_buffer);
        rtapi_print_msg(RTAPI_MSG_ERR,
            "PoKeys: %s:%s: [10] parser returned %d\n",
            __FILE__, __FUNCTION__, parse_ret);
    }

    t->status = TRANSACTION_COMPLETED;
    t->response_ready = true;

    rtapi_print_msg(RTAPI_MSG_ERR,
        "PoKeys: %s:%s: [11] done, returning 1\n",
        __FILE__, __FUNCTION__);

    return 1; // One response processed
}

/**
 * @brief Enhanced timeout and retry check with exponential backoff and error recovery.
 *
 * This implementation provides:
 * - Exponential backoff for retries
 * - Better error tracking and recovery
 * - RT-safe operation with minimal overhead
 *
 * @param dev Pointer to the PoKeys device structure.
 * @param timeout_us Base timeout threshold in microseconds per attempt.
 */
void PK_TimeoutAndRetryCheck(sPoKeysDevice *dev, uint64_t timeout_us)
{
    if (!dev) return;

    // Guard against NULL devHandle (e.g. USB-only device without UDP socket).
    // Mirrors the identical guard already present in PK_ReceiveAndDispatch and
    // SendRequestAsync.  Without this check, the sendto() inside the retry loop
    // would dereference a NULL pointer and produce SIGSEGV — the same crash that
    // was observed in the RT component before the devHandle guards were added.
    if (!dev->devHandle) {
        rtapi_print_msg(RTAPI_MSG_ERR,
            "PoKeys: %s:%s: devHandle is NULL - clearing pending transactions\n",
            __FILE__, __FUNCTION__);
        for (int i = 0; i < MAX_TRANSACTIONS; i++) {
            if (pk_transactions[i].status == TRANSACTION_PENDING) {
                pk_transactions[i].status = TRANSACTION_FAILED;
                pk_transactions[i].retries_left = 0;
            }
        }
        return;
    }

    uint64_t now = get_current_time_us();
    static uint32_t consecutive_errors = 0;
    static uint64_t last_error_time = 0;
    
    // Implement circuit breaker pattern - if too many consecutive errors,
    // temporarily back off to avoid overwhelming the device
    const uint32_t MAX_CONSECUTIVE_ERRORS = 10;
    const uint64_t ERROR_BACKOFF_TIME_US = 1000000; // 1 second backoff
    
    if (consecutive_errors >= MAX_CONSECUTIVE_ERRORS) {
        if ((now - last_error_time) < ERROR_BACKOFF_TIME_US) {
            return; // Still in backoff period
        } else {
            consecutive_errors = 0; // Reset after backoff period
        }
    }

    for (int i = 0; i < MAX_TRANSACTIONS; i++) {
        async_transaction_t *t = &pk_transactions[i];

        if (t->status == TRANSACTION_PENDING) {
            // Calculate dynamic timeout with exponential backoff
            uint32_t retry_multiplier = (3 - t->retries_left); // 0, 1, 2 for retries
            uint64_t effective_timeout = timeout_us * (1 << retry_multiplier); // 1x, 2x, 4x
            
            if ((now - t->timestamp_sent) > effective_timeout) {
                if (t->retries_left > 0) {
                    // Attempt retry with improved error handling
                    ssize_t sent = sendto(*(int*)dev->devHandle,
                                          t->request_buffer, sizeof(t->request_buffer), 0,
                                          (struct sockaddr *)dev->devHandle2, sizeof(struct sockaddr_in));
                    if (sent >= 0) {
                        t->timestamp_sent = now;
                        t->retries_left--;
                        // Reset consecutive error counter on successful send
                        if (consecutive_errors > 0) consecutive_errors--;
                        
                        #ifdef DEBUG_ASYNC_RETRIES
                        rtapi_print_msg(RTAPI_MSG_DBG, "PoKeys: Retry %d for request ID %d, cmd=0x%02X\n", 
                                       (3 - t->retries_left), t->request_id, t->command_sent);
                        #endif
                    } else {
                        // Send failed - increment error counter and log
                        consecutive_errors++;
                        last_error_time = now;
                        
                        rtapi_print_msg(RTAPI_MSG_WARN, "PoKeys: Retry send failed for request ID %d, errno=%d\n", 
                                       t->request_id, errno);
                        
                        // Mark as failed if this was the last retry attempt
                        if (t->retries_left == 1) {
                            t->status = TRANSACTION_FAILED;
                            t->retries_left = 0;
                        }
                    }
                } else {
                    // No retries left: mark timeout
                    t->status = TRANSACTION_TIMEOUT;
                    consecutive_errors++;
                    last_error_time = now;
                    
                    rtapi_print_msg(RTAPI_MSG_WARN, "PoKeys: Request ID %d timed out after all retries, cmd=0x%02X\n", 
                                   t->request_id, t->command_sent);
                }
            }
        }
    }
}


/* -------------------------------------------------------------------------
 * Async Scheduler implementation
 * (Migrated from experimental/async_scheduler.c per architecture rules.)
 * ------------------------------------------------------------------------- */
#include <string.h>

static periodic_async_task_t async_tasks[MAX_ASYNC_TASKS];
static size_t async_task_count_internal = 0;

int register_async_task(async_func_t func, sPoKeysDevice *dev, double freq_hz, const char *name)
{
    if (async_task_count_internal >= MAX_ASYNC_TASKS || freq_hz <= 0.0) return -1;

    int64_t now      = rtapi_get_time();
    int64_t interval = (int64_t)(1e9 / freq_hz);

    async_tasks[async_task_count_internal++] = (periodic_async_task_t){
        .func            = func,
        .dev             = dev,
        .interval_ns     = interval,
        .next_call_time  = now + interval,
        .name            = name,
        .active          = 1
    };
    return 0;
}

int async_dispatcher(void)
{
    int64_t now            = rtapi_get_time();
    size_t  selected_index = SIZE_MAX;
    int64_t earliest_due   = INT64_MAX;

    for (size_t i = 0; i < async_task_count_internal; ++i) {
        if (!async_tasks[i].active) continue;
        if (async_tasks[i].next_call_time <= now &&
            async_tasks[i].next_call_time  < earliest_due) {
            selected_index = i;
            earliest_due   = async_tasks[i].next_call_time;
        }
    }

    if (selected_index == SIZE_MAX)
        return 0; /* nothing due */

    periodic_async_task_t *t = &async_tasks[selected_index];
    rtapi_print_msg(RTAPI_MSG_ERR,
        "PoKeys: async_dispatcher: firing task '%s' (func=%p dev=%p)\n",
        t->name, (void*)t->func, (void*)t->dev);
    int ret = t->func(t->dev);
    t->next_call_time = now + t->interval_ns;
    rtapi_print_msg(RTAPI_MSG_ERR,
        "PoKeys: async_dispatcher: task '%s' returned %d\n",
        t->name, ret);

    if (ret < 0)
        rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys async_dispatcher: %s FAILED (ret=%d)\n", t->name, ret);

    return 1;
}

void async_task_set_active(const char *name, int active)
{
    for (size_t i = 0; i < async_task_count_internal; ++i) {
        if (strcmp(async_tasks[i].name, name) == 0) {
            async_tasks[i].active = (active != 0);
            return;
        }
    }
}

size_t async_task_count(void)
{
    return async_task_count_internal;
}
