#ifndef POKEYSLIB_ASYNC_H
#define POKEYSLIB_ASYNC_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// Define the mailbox entry structure
typedef struct {
    uint8_t request_id;
    pokeys_command_t command_sent;
    uint64_t timestamp_sent;
    int retries_left;
    bool response_ready;

    void *target_ptr;
    size_t target_size;

    uint8_t request_buffer[64];
    uint8_t response_buffer[64];
} mailbox_entry_t;

// Function declarations
int CreateRequestAsync(pokeys_device_t *dev, pokeys_command_t cmd,
                       const uint8_t *params, size_t params_len,
                       void *target_ptr, size_t target_size);

int SendRequestAsync(pokeys_device_t *dev, uint8_t request_id);

int PK_ReceiveAndDispatch(pokeys_device_t *dev);

void PK_TimeoutAndRetryCheck(pokeys_device_t *dev, uint64_t timeout_us);

#endif // POKEYSLIB_ASYNC_H