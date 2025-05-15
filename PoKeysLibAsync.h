#ifndef POKEYSLIB_ASYNC_H
#define POKEYSLIB_ASYNC_H
#include "PoKeysLibHal.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>








// Function declarations
int CreateRequestAsync(sPoKeysDevice *dev, pokeys_command_t cmd,
    const uint8_t *params, size_t params_len,
    void *target_ptr, size_t target_size,
    int (*parser_func)(sPoKeysDevice *dev, const uint8_t *response));

int CreateRequestAsyncWithPayload(
        sPoKeysDevice *device,
        pokeys_command_t cmd,
        const uint8_t *params,
        size_t params_len,
        const void *payload,
        size_t payload_size,
        pokeys_response_parser_t parser_func
    );

int SendRequestAsync(sPoKeysDevice *dev, uint8_t request_id);

int PK_ReceiveAndDispatch(sPoKeysDevice *dev);

void PK_TimeoutAndRetryCheck(sPoKeysDevice *dev, uint64_t timeout_us);

#endif // POKEYSLIB_ASYNC_H
