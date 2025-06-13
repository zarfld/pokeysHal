#include "PoKeysLibHal.h"
#include "PoKeysLibAsync.h"
#include <string.h>

/*
 * Asynchronous PoCAN functions built on the generic PoKeysLib async framework.
 * These mirror the blocking CAN helpers from PoKeysLibCAN.c but allow non-blocking
 * operation using CreateRequestAsync/SendRequestAsync.
 *
 * Basic PoCAN command layout:
 *   Command 0x86 with subcommands:
 *     0x01 - configure bitrate (payload: uint32_t bitrate)
 *     0x10 - register filter (param2 = format, payload: uint32_t CAN ID)
 *     0x20 - transmit message  (payload: sPoKeysCANmsg)
 *     0x31 - read message     (response: status + sPoKeysCANmsg)
 *     0x32 - flush receive buffer
 */

typedef struct {
    uint8_t *status_ptr;
    sPoKeysCANmsg *msg_ptr;
    uint8_t used;
} CANAsyncContext;

static CANAsyncContext can_ctx[256];

static int PK_CANRead_Parse(sPoKeysDevice *dev, const uint8_t *resp)
{
    uint8_t id = resp[6];
    CANAsyncContext *c = &can_ctx[id];
    if (c->status_ptr)
        *(c->status_ptr) = resp[3];
    if (c->msg_ptr && resp[3])
        memcpy(c->msg_ptr, resp + 8, sizeof(sPoKeysCANmsg));
    c->used = 0;
    return PK_OK;
}

int PK_CANConfigureAsync(sPoKeysDevice* device, uint32_t bitrate)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    uint8_t params[1] = { 0x01 };
    int req = CreateRequestAsyncWithPayload(device, 0x86, params, 1,
                                            &bitrate, sizeof(uint32_t), NULL);
    if (req < 0) return req;
    return SendRequestAsync(device, req);
}

int PK_CANRegisterFilterAsync(sPoKeysDevice* device, uint8_t format, uint32_t CANid)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    uint8_t params[2] = { 0x10, format };
    int req = CreateRequestAsyncWithPayload(device, 0x86, params, 2,
                                            &CANid, sizeof(uint32_t), NULL);
    if (req < 0) return req;
    return SendRequestAsync(device, req);
}

int PK_CANWriteAsync(sPoKeysDevice* device, sPoKeysCANmsg *msg)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    uint8_t params[1] = { 0x20 };
    int req = CreateRequestAsyncWithPayload(device, 0x86, params, 1,
                                            msg, sizeof(sPoKeysCANmsg), NULL);
    if (req < 0) return req;
    return SendRequestAsync(device, req);
}

int PK_CANReadAsync(sPoKeysDevice* device, sPoKeysCANmsg *msg, uint8_t *status)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    uint8_t params[1] = { 0x31 };
    int req = CreateRequestAsync(device, 0x86, params, 1,
                                 NULL, 0, PK_CANRead_Parse);
    if (req < 0) return req;
    can_ctx[req].status_ptr = status;
    can_ctx[req].msg_ptr = msg;
    can_ctx[req].used = 1;
    return SendRequestAsync(device, req);
}

int PK_CANFlushAsync(sPoKeysDevice* device)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    uint8_t params[1] = { 0x32 };
    int req = CreateRequestAsync(device, 0x86, params, 1, NULL, 0, NULL);
    if (req < 0) return req;
    return SendRequestAsync(device, req);
}

