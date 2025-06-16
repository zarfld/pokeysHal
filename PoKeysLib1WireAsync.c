/*
 * Asynchronous 1-Wire helpers using PoKeysLibAsync framework.
 *
 * These mirror the blocking functions found in PoKeysLib1Wire.c
 * but avoid any blocking socket operations.
 *
 * Realtime-compatible design with minimal CPU usage.
 * Asynchronous non-blocking communication.
 */

#include "PoKeysLibHal.h"
#include "PoKeysLibAsync.h"
#include <string.h>

typedef struct {
    uint8_t *status_ptr;      /* generic status or operation status */
    uint8_t *count_ptr;       /* read count or scan result */
    uint8_t *buffer_ptr;      /* read data or ROM code */
    uint8_t max_len;          /* maximum buffer length */
    uint8_t used;
} OneWireAsyncContext;

static OneWireAsyncContext ow_ctx[256];

static int PK_1Wire_StatusParse(sPoKeysDevice *dev, const uint8_t *resp)
{
    uint8_t id = resp[6];
    OneWireAsyncContext *c = &ow_ctx[id];
    if (c->status_ptr)
        *(c->status_ptr) = resp[3];
    c->used = 0;
    return PK_OK;
}

static int PK_1Wire_ReadStatusParse(sPoKeysDevice *dev, const uint8_t *resp)
{
    uint8_t id = resp[6];
    OneWireAsyncContext *c = &ow_ctx[id];
    if (c->status_ptr)
        *(c->status_ptr) = resp[8];
    if (c->count_ptr)
        *(c->count_ptr) = 0;
    if (resp[8] == 1 && c->buffer_ptr) {
        uint8_t count = resp[9];
        if (count > 16)
            count = 16;
        if (c->count_ptr)
            *(c->count_ptr) = count;
        for (uint8_t i = 0; i < c->max_len && i < count; i++)
            c->buffer_ptr[i] = resp[10 + i];
    }
    c->used = 0;
    return PK_OK;
}

static int PK_1Wire_BusScanParse(sPoKeysDevice *dev, const uint8_t *resp)
{
    uint8_t id = resp[6];
    OneWireAsyncContext *c = &ow_ctx[id];
    if (c->status_ptr)
        *(c->status_ptr) = resp[8];
    if (c->count_ptr)
        *(c->count_ptr) = resp[9];
    if (c->buffer_ptr)
        memcpy(c->buffer_ptr, resp + 10, 8);
    c->used = 0;
    return PK_OK;
}

int PK_1WireStatusSetAsync(sPoKeysDevice* device, uint8_t activated)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    uint8_t params[1] = { activated };
    int req = CreateRequestAsync(device, PK_CMD_ONEWIRE_COMMUNICATION,
                                 params, 1, NULL, 0, NULL);
    if (req < 0) return req;
    return SendRequestAsync(device, req);
}

int PK_1WireStatusGetAsync(sPoKeysDevice* device, uint8_t* activated)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    uint8_t params[1] = { 0x11 };
    int req = CreateRequestAsync(device, PK_CMD_ONEWIRE_COMMUNICATION,
                                 params, 1, NULL, 0, PK_1Wire_StatusParse);
    if (req < 0) return req;
    ow_ctx[req].status_ptr = activated;
    ow_ctx[req].used = 1;
    return SendRequestAsync(device, req);
}

int PK_1WireWriteReadStartAsync(sPoKeysDevice* device, uint8_t WriteCount,
                                uint8_t ReadCount, uint8_t *data)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    if (WriteCount > 16) WriteCount = 16;
    if (ReadCount > 16) ReadCount = 16;
    uint8_t params[4] = { 0x10, WriteCount, ReadCount, 0 };
    int req = CreateRequestAsyncWithPayload(device, PK_CMD_ONEWIRE_COMMUNICATION,
                                            params, 4, data, WriteCount, NULL);
    if (req < 0) return req;
    return SendRequestAsync(device, req);
}

int PK_1WireWriteReadStartExAsync(sPoKeysDevice* device, uint8_t pinID,
                                  uint8_t WriteCount, uint8_t ReadCount,
                                  uint8_t *data)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    if (WriteCount > 16) WriteCount = 16;
    if (ReadCount > 16) ReadCount = 16;
    uint8_t params[4] = { 0x10, WriteCount, ReadCount, pinID };
    int req = CreateRequestAsyncWithPayload(device, PK_CMD_ONEWIRE_COMMUNICATION,
                                            params, 4, data, WriteCount, NULL);
    if (req < 0) return req;
    return SendRequestAsync(device, req);
}

int PK_1WireReadStatusGetAsync(sPoKeysDevice* device, uint8_t *readStatus,
                               uint8_t *ReadCount, uint8_t *data)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    uint8_t params[1] = { 0x11 };
    int req = CreateRequestAsync(device, PK_CMD_ONEWIRE_COMMUNICATION,
                                 params, 1, NULL, 0, PK_1Wire_ReadStatusParse);
    if (req < 0) return req;
    ow_ctx[req].status_ptr = readStatus;
    ow_ctx[req].count_ptr = ReadCount;
    ow_ctx[req].buffer_ptr = data;
    ow_ctx[req].max_len = 16;
    ow_ctx[req].used = 1;
    return SendRequestAsync(device, req);
}

int PK_1WireBusScanStartAsync(sPoKeysDevice* device, uint8_t pinID)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    uint8_t params[2] = { 0x20, pinID };
    int req = CreateRequestAsync(device, PK_CMD_ONEWIRE_COMMUNICATION,
                                 params, 2, NULL, 0, NULL);
    if (req < 0) return req;
    return SendRequestAsync(device, req);
}

int PK_1WireBusScanGetResultsAsync(sPoKeysDevice* device, uint8_t *operationStatus,
                                   uint8_t *scanResult, uint8_t *deviceROM)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    uint8_t params[1] = { 0x21 };
    int req = CreateRequestAsync(device, PK_CMD_ONEWIRE_COMMUNICATION,
                                 params, 1, NULL, 0, PK_1Wire_BusScanParse);
    if (req < 0) return req;
    ow_ctx[req].status_ptr = operationStatus;
    ow_ctx[req].count_ptr = scanResult;
    ow_ctx[req].buffer_ptr = deviceROM;
    ow_ctx[req].max_len = 8;
    ow_ctx[req].used = 1;
    return SendRequestAsync(device, req);
}

int PK_1WireBusScanContinueAsync(sPoKeysDevice* device)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    uint8_t params[1] = { 0x22 };
    int req = CreateRequestAsync(device, PK_CMD_ONEWIRE_COMMUNICATION,
                                 params, 1, NULL, 0, NULL);
    if (req < 0) return req;
    return SendRequestAsync(device, req);
}

int PK_1WireBusScanStopAsync(sPoKeysDevice* device)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    uint8_t params[1] = { 0x23 };
    int req = CreateRequestAsync(device, PK_CMD_ONEWIRE_COMMUNICATION,
                                 params, 1, NULL, 0, NULL);
    if (req < 0) return req;
    return SendRequestAsync(device, req);
}

