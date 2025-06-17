/*
 * PoKeysLibMatrixLEDAsync.c
 *
 * Asynchronous wrappers for matrix LED configuration and display updates.
 * These functions mirror the behaviour of the blocking API in
 * PoKeysLibMatrixLED.c but use the PoKeysLibAsync infrastructure so
 * callers never block waiting for UDP traffic.
 *
 * Each call only places a packet into the async transmit queue.
 * The receive dispatcher fills the device structure when responses
 * arrive which keeps realtime threads deterministic and responsive.
 */

#include "PoKeysLibHal.h"
#include "PoKeysLibAsync.h"
#include <string.h>

static int PK_MLED_ConfigParse(sPoKeysDevice *dev, const uint8_t *resp)
{
    if (!dev || !resp)
        return PK_ERR_GENERIC;

    dev->MatrixLED[0].displayEnabled = (resp[3] & 1) ? 1 : 0;
    dev->MatrixLED[0].rows          = resp[4] & 0x0F;
    dev->MatrixLED[0].columns       = (resp[4] >> 4) & 0x0F;

    dev->MatrixLED[1].displayEnabled = (resp[3] & 2) ? 1 : 0;
    dev->MatrixLED[1].rows          = resp[5] & 0x0F;
    dev->MatrixLED[1].columns       = (resp[5] >> 4) & 0x0F;
    return PK_OK;
}

int PK_MatrixLEDConfigurationGetAsync(sPoKeysDevice* device)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    if (device->info.iMatrixLED == 0) return PK_ERR_NOT_SUPPORTED;

    uint8_t params[1] = { 1 }; /* subcommand: read */
    int req = CreateRequestAsync(device, PK_CMD_MATRIX_LED_CONFIGURATION,
                                 params, 1, NULL, 0,
                                 PK_MLED_ConfigParse);
    if (req < 0)
        return req;
    return SendRequestAsync(device, (uint8_t)req);
}

int PK_MatrixLEDConfigurationSetAsync(sPoKeysDevice* device)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    if (device->info.iMatrixLED == 0) return PK_ERR_NOT_SUPPORTED;

    uint8_t params[4];
    params[0] = 0; /* subcommand: write */
    params[1] = (device->MatrixLED[0].displayEnabled ? 1 : 0) |
                (device->MatrixLED[1].displayEnabled ? 2 : 0);
    params[2] = (device->MatrixLED[0].rows & 0x0F) |
                ((device->MatrixLED[0].columns & 0x0F) << 4);
    params[3] = (device->MatrixLED[1].rows & 0x0F) |
                ((device->MatrixLED[1].columns & 0x0F) << 4);

    int req = CreateRequestAsync(device, PK_CMD_MATRIX_LED_CONFIGURATION,
                                 params, 4, NULL, 0, NULL);
    if (req < 0)
        return req;
    return SendRequestAsync(device, (uint8_t)req);
}

int PK_MatrixLEDUpdateAsync(sPoKeysDevice* device)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    if (device->info.iMatrixLED == 0) return PK_ERR_NOT_SUPPORTED;

    uint8_t displayCode[] = { 1, 11 };
    for (uint8_t i = 0; i < device->info.iMatrixLED && i < 2; i++) {
        if (device->MatrixLED[i].RefreshFlag) {
            uint8_t params[1] = { displayCode[i] };
            int req = CreateRequestAsyncWithPayload(device,
                        PK_CMD_MATRIX_LED_UPDATE,
                        params, 1,
                        device->MatrixLED[i].data, 8,
                        NULL);
            if (req < 0)
                return req;
            int r = SendRequestAsync(device, (uint8_t)req);
            if (r < 0)
                return r;
            device->MatrixLED[i].RefreshFlag = 0;
        }
    }
    return PK_OK;
}

