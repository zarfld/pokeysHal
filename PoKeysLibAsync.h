#ifndef POKEYSLIB_ASYNC_H
#define POKEYSLIB_ASYNC_H
#include "PoKeysLibHal.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>




typedef enum {
    TRANSACTION_PENDING = 0,
    TRANSACTION_COMPLETED = 1,
    TRANSACTION_TIMEOUT = 2,
    TRANSACTION_FAILED = 3
} transaction_status_t;

typedef enum {
    PK_CMD_DIGITAL_INPUTS_GET   = 0x10,
    PK_CMD_DIGITAL_OUTPUTS_SET  = 0x11,
    PK_CMD_ANALOG_INPUTS_GET    = 0x20,
    PK_CMD_ANALOG_OUTPUTS_SET   = 0x21,
    PK_CMD_ENCODER_VALUES_GET = 0xCD,
    PK_CMD_ENCODER_VALUES_SET = 0xCD,    // Same as GET but param10/11 means set
    PK_CMD_ENCODER_OPTION_SET  = 0xC4,
    PK_CMD_ENCODER_MAPPING_SET = 0xC5,
    PK_CMD_ENCODER_KEYMAP_A_SET = 0xC6,
    PK_CMD_ENCODER_KEYMAP_B_SET = 0xC7,
    PK_CMD_FAST_ENCODERS_SET = 0xCE,
    PK_CMD_ULTRAFAST_ENCODERS_SET = 0x1C,
    PK_CMD_ENCODER_TEST_MODE_GET = 0x85,
    PK_CMD_DEVICE_INFO_GET      = 0x05,
    // usw...
} pokeys_command_t;

typedef int (*pokeys_response_parser_t)(sPoKeysDevice *dev, const uint8_t *response);

typedef struct {
    uint8_t request_buffer[64];
    uint8_t response_buffer[64];
    uint8_t request_id;
    pokeys_command_t command_sent;

    uint64_t timestamp_sent;
    uint8_t retries_left;

    transaction_status_t status;
    bool response_ready;
    // Optional parser
    int (*response_parser)(sPoKeysDevice *dev, const uint8_t *response);

    void *target_ptr;
    size_t target_size;
} async_transaction_t;

typedef struct {
    uint8_t request_id;
    pokeys_command_t command_sent;
    uint64_t timestamp_sent;
    int retries_left;
    bool response_ready;

    void *target_ptr;
    size_t target_size;
    pokeys_response_parser_t response_parser; // << NEW! Optional per-request parser function

    uint8_t request_buffer[64];
    uint8_t response_buffer[64];
} mailbox_entry_t;



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
