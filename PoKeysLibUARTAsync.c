/*
 * PoKeysLibUARTAsync.c
 *
 * Asynchronous UART communication helpers built on the
 * PoKeysLibAsync request framework. These functions mirror
 * the blocking API in PoKeysLibUART.c but return immediately
 * after queueing the request so that realtime threads are not
 * delayed by network I/O.
 */

#include "PoKeysLibHal.h"
#include "PoKeysLibAsync.h"
#include <string.h>

typedef struct {
    uint8_t *len_ptr;
    uint8_t *data_ptr;
    uint8_t used;
} UARTAsyncContext;

static UARTAsyncContext uart_ctx[256];

static int PK_UART_ReadParse(sPoKeysDevice *dev, const uint8_t *resp)
{
    uint8_t id = resp[6];
    UARTAsyncContext *c = &uart_ctx[id];
    if (c->len_ptr)
        *(c->len_ptr) = resp[3];
    if (c->data_ptr && resp[3])
        memcpy(c->data_ptr, resp + 8, resp[3]);
    c->used = 0;
    return PK_OK;
}

int PK_UARTConfigureAsync(sPoKeysDevice* device, uint32_t baudrate,
                          uint8_t format, uint8_t interfaceID)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    uint8_t params[3] = { 0x10, interfaceID, (uint8_t)(format & 0x7F) };
    int req = CreateRequestAsyncWithPayload(device, PK_CMD_UART_COMMUNICATION,
                                            params, 3, &baudrate,
                                            sizeof(uint32_t), NULL);
    if (req < 0) return req;
    return SendRequestAsync(device, (uint8_t)req);
}

int PK_UARTWriteAsync(sPoKeysDevice* device, uint8_t interfaceID,
                      uint8_t *dataPtr, uint32_t dataWriteLen)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    uint32_t remaining = dataWriteLen;
    uint32_t offset = 0;
    while (remaining > 0) {
        uint8_t chunk = (remaining > 55) ? 55 : (uint8_t)remaining;
        uint8_t params[3] = { 0x20, interfaceID, chunk };
        int req = CreateRequestAsyncWithPayload(device, PK_CMD_UART_COMMUNICATION,
                                                params, 3,
                                                dataPtr + offset, chunk,
                                                NULL);
        if (req < 0) return req;
        int r = SendRequestAsync(device, (uint8_t)req);
        if (r < 0) return r;
        offset += chunk;
        remaining -= chunk;
    }
    return PK_OK;
}

int PK_UARTReadAsync(sPoKeysDevice* device, uint8_t interfaceID,
                     uint8_t *dataPtr, uint8_t *dataReadLen)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    uint8_t params[2] = { 0x30, interfaceID };
    int req = CreateRequestAsync(device, PK_CMD_UART_COMMUNICATION,
                                 params, 2, NULL, 0,
                                 PK_UART_ReadParse);
    if (req < 0) return req;
    uart_ctx[req].len_ptr = dataReadLen;
    uart_ctx[req].data_ptr = dataPtr;
    uart_ctx[req].used = 1;
    return SendRequestAsync(device, (uint8_t)req);
}

