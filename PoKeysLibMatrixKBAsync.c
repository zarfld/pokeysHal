/*
 * Asynchronous matrix keyboard support built on the PoKeysLibAsync
 * infrastructure. These helpers mirror the blocking calls in
 * PoKeysLibMatrixKB.c but queue requests so that realtime threads
 * remain non-blocking and CPU overhead is minimal.
 */

#include "PoKeysLibHal.h"
#include "PoKeysLibAsync.h"
#include <string.h>

typedef struct {
    uint8_t index;
    uint8_t used;
} MatrixKBAsyncCtx;

static MatrixKBAsyncCtx kb_ctx[256];

static int PK_MKB_ConfigParse(sPoKeysDevice *dev, const uint8_t *resp)
{
    if (!dev || !resp) return PK_ERR_GENERIC;
    dev->matrixKB.matrixKBconfiguration = resp[8];
    dev->matrixKB.matrixKBheight = 1 + (resp[9] & 0x0F);
    dev->matrixKB.matrixKBwidth  = 1 + ((resp[9] >> 4) & 0x0F);

    for (uint8_t n = 0; n < 8; n++) {
        dev->matrixKB.matrixKBrowsPins[n]     = resp[10 + n];
        dev->matrixKB.matrixKBrowsPins[8 + n] = resp[42 + n];
        dev->matrixKB.matrixKBcolumnsPins[n]  = resp[18 + n];
    }

    for (uint8_t n = 0; n < 128; n++) {
        dev->matrixKB.macroMappingOptions[n] =
            ((resp[26 + n/8] & (1 << (n%8))) > 0) ? 1 : 0;
    }

    dev->matrixKB.matrixKBScanningDecimation = resp[51];
    return PK_OK;
}

static int PK_MKB_KeyCodeParse(sPoKeysDevice *dev, const uint8_t *resp)
{
    uint8_t id = resp[6];
    MatrixKBAsyncCtx *c = &kb_ctx[id];
    uint8_t blk = c->index;
    for (uint8_t k = 0; k < 16; k++) {
        dev->matrixKB.keyMappingKeyCode[blk*16 + k]     = resp[8 + k];
        dev->matrixKB.keyMappingKeyModifier[blk*16 + k] = resp[24 + k];
    }
    if (dev->info.iMatrixKeyboardTriggeredMapping) {
        for (uint8_t x = 0; x < 8; x++) {
            dev->matrixKB.keyMappingTriggeredKey[blk*16 + x] =
                (resp[40] & (1 << x)) > 0;
            dev->matrixKB.keyMappingTriggeredKey[blk*16 + 8 + x] =
                (resp[41] & (1 << x)) > 0;
        }
    }
    c->used = 0;
    return PK_OK;
}

static int PK_MKB_KeyCodeUpParse(sPoKeysDevice *dev, const uint8_t *resp)
{
    uint8_t id = resp[6];
    MatrixKBAsyncCtx *c = &kb_ctx[id];
    uint8_t blk = c->index;
    for (uint8_t k = 0; k < 16; k++) {
        dev->matrixKB.keyMappingKeyCodeUp[blk*16 + k]     = resp[8 + k];
        dev->matrixKB.keyMappingKeyModifierUp[blk*16 + k] = resp[24 + k];
    }
    c->used = 0;
    return PK_OK;
}

static int PK_MKB_StatusParse(sPoKeysDevice *dev, const uint8_t *resp)
{
    if (!dev || !resp) return PK_ERR_GENERIC;
    for (uint8_t n = 0; n < 128; n++) {
        dev->matrixKB.matrixKBvalues[n] =
            (resp[8 + n/8] & (1 << (n%8))) > 0;
    }
    return PK_OK;
}

int PK_MatrixKBConfigurationGetAsync(sPoKeysDevice* device)
{
    if (!device) return PK_ERR_NOT_CONNECTED;

    if (device->info.iMatrixKeyboard) {
        uint8_t p[1] = { 10 };
        int req = CreateRequestAsync(device, PK_CMD_MATRIX_KEYBOARD_CFG,
                                     p, 1, NULL, 0, PK_MKB_ConfigParse);
        if (req < 0) return req;
        int r = SendRequestAsync(device, (uint8_t)req);
        if (r < 0) return r;
    }

    if (device->info.iKeyMapping) {
        for (uint8_t n = 0; n < 8; n++) {
            uint8_t p1[1] = { (uint8_t)(12 + n) };
            int req = CreateRequestAsync(device, PK_CMD_MATRIX_KEYBOARD_CFG,
                                         p1, 1, NULL, 0, PK_MKB_KeyCodeParse);
            if (req < 0) return req;
            kb_ctx[req].index = n;
            kb_ctx[req].used = 1;
            int r = SendRequestAsync(device, (uint8_t)req);
            if (r < 0) return r;

            if (device->info.iMatrixKeyboardTriggeredMapping) {
                uint8_t p2[1] = { (uint8_t)(32 + n) };
                int req2 = CreateRequestAsync(device, PK_CMD_MATRIX_KEYBOARD_CFG,
                                              p2, 1, NULL, 0, PK_MKB_KeyCodeUpParse);
                if (req2 < 0) return req2;
                kb_ctx[req2].index = n;
                kb_ctx[req2].used = 1;
                r = SendRequestAsync(device, (uint8_t)req2);
                if (r < 0) return r;
            }
        }
    }

    return PK_OK;
}

int PK_MatrixKBConfigurationSetAsync(sPoKeysDevice* device)
{
    if (!device) return PK_ERR_NOT_CONNECTED;

    if (device->info.iMatrixKeyboard) {
        uint8_t payload1[42];
        payload1[0] = 0; /* deactivate */
        payload1[1] = (uint8_t)(((device->matrixKB.matrixKBheight - 1) & 0x0F) |
                               (((device->matrixKB.matrixKBwidth - 1) << 4) & 0xF0));
        for (uint8_t n = 0; n < 8; n++) {
            payload1[2 + n]  = device->matrixKB.matrixKBrowsPins[n];
            payload1[34 + n] = device->matrixKB.matrixKBrowsPins[8 + n];
            payload1[10 + n] = device->matrixKB.matrixKBcolumnsPins[n];
        }
        memset(payload1 + 18, 0, 16);
        for (uint8_t n = 0; n < 128; n++) {
            if (device->matrixKB.macroMappingOptions[n])
                payload1[18 + n/8] |= (1 << (n%8));
        }
        int req = CreateRequestAsyncWithPayload(device,
                    PK_CMD_MATRIX_KEYBOARD_CFG,
                    (const uint8_t[]){1}, 1,
                    payload1, sizeof(payload1), NULL);
        if (req < 0) return req;
        int r = SendRequestAsync(device, (uint8_t)req);
        if (r < 0) return r;

        uint8_t payloadScan[1] = { device->matrixKB.matrixKBScanningDecimation };
        req = CreateRequestAsyncWithPayload(device,
                    PK_CMD_MATRIX_KEYBOARD_CFG,
                    (const uint8_t[]){50}, 1,
                    payloadScan, 1, NULL);
        if (req < 0) return req;
        r = SendRequestAsync(device, (uint8_t)req);
        if (r < 0) return r;
    }

    if (device->info.iKeyMapping) {
        for (uint8_t n = 0; n < 8; n++) {
            uint8_t payload[34];
            for (uint8_t k = 0; k < 16; k++) {
                payload[k]      = device->matrixKB.keyMappingKeyCode[n*16 + k];
                payload[16 + k] = device->matrixKB.keyMappingKeyModifier[n*16 + k];
            }
            uint8_t t0 = 0, t1 = 0;
            if (device->info.iMatrixKeyboardTriggeredMapping) {
                for (uint8_t x = 0; x < 8; x++) {
                    if (device->matrixKB.keyMappingTriggeredKey[n*16 + x])
                        t0 |= (1 << x);
                    if (device->matrixKB.keyMappingTriggeredKey[n*16 + 8 + x])
                        t1 |= (1 << x);
                }
            }
            payload[32] = t0;
            payload[33] = t1;
            int req = CreateRequestAsyncWithPayload(device,
                        PK_CMD_MATRIX_KEYBOARD_CFG,
                        (const uint8_t[]){ (uint8_t)(2 + n) }, 1,
                        payload, sizeof(payload), NULL);
            if (req < 0) return req;
            int r = SendRequestAsync(device, (uint8_t)req);
            if (r < 0) return r;

            if (device->info.iMatrixKeyboardTriggeredMapping) {
                uint8_t up_payload[32];
                for (uint8_t k = 0; k < 16; k++) {
                    up_payload[k]      = device->matrixKB.keyMappingKeyCodeUp[n*16 + k];
                    up_payload[16 + k] = device->matrixKB.keyMappingKeyModifierUp[n*16 + k];
                }
                req = CreateRequestAsyncWithPayload(device,
                        PK_CMD_MATRIX_KEYBOARD_CFG,
                        (const uint8_t[]){ (uint8_t)(22 + n) }, 1,
                        up_payload, sizeof(up_payload), NULL);
                if (req < 0) return req;
                r = SendRequestAsync(device, (uint8_t)req);
                if (r < 0) return r;
            }
        }
    }

    if (device->info.iMatrixKeyboard) {
        uint8_t payload1[42];
        payload1[0] = device->matrixKB.matrixKBconfiguration;
        payload1[1] = (uint8_t)(((device->matrixKB.matrixKBheight - 1) & 0x0F) |
                               (((device->matrixKB.matrixKBwidth - 1) << 4) & 0xF0));
        for (uint8_t n = 0; n < 8; n++) {
            payload1[2 + n]  = device->matrixKB.matrixKBrowsPins[n];
            payload1[34 + n] = device->matrixKB.matrixKBrowsPins[8 + n];
            payload1[10 + n] = device->matrixKB.matrixKBcolumnsPins[n];
        }
        memset(payload1 + 18, 0, 16);
        for (uint8_t n = 0; n < 128; n++) {
            if (device->matrixKB.macroMappingOptions[n])
                payload1[18 + n/8] |= (1 << (n%8));
        }
        int req = CreateRequestAsyncWithPayload(device,
                    PK_CMD_MATRIX_KEYBOARD_CFG,
                    (const uint8_t[]){1}, 1,
                    payload1, sizeof(payload1), NULL);
        if (req < 0) return req;
        int r = SendRequestAsync(device, (uint8_t)req);
        if (r < 0) return r;
    }

    return PK_OK;
}

int PK_MatrixKBStatusGetAsync(sPoKeysDevice* device)
{
    if (!device) return PK_ERR_NOT_CONNECTED;

    if (!device->info.iMatrixKeyboard)
        return PK_ERR_NOT_SUPPORTED;

    uint8_t p[1] = { 20 };
    int req = CreateRequestAsync(device, PK_CMD_MATRIX_KEYBOARD_CFG,
                                 p, 1, NULL, 0, PK_MKB_StatusParse);
    if (req < 0) return req;
    return SendRequestAsync(device, (uint8_t)req);
}

