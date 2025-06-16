/*
 * Asynchronous WS2812 LED strip helpers.
 *
 * This mirrors PoKeysLibWS2812.c but performs non-blocking
 * transfers using the PoKeysLibAsync infrastructure. LED data
 * packets are queued without waiting so realtime threads remain
 * deterministic with minimal CPU usage.
 */

#include "PoKeysLibHal.h"
#include "PoKeysLibCore.h"
#include "PoKeysLibAsync.h"
#include "PoKeysLibCoreSocketsAsync.h"
#include <string.h>

int PK_WS2812_UpdateAsync(sPoKeysDevice* device, uint16_t LEDcount, uint8_t updateFlag)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    uint8_t params[4] = { 0x00, (uint8_t)(LEDcount & 0xFF), (uint8_t)(LEDcount >> 8), updateFlag };
    int req = CreateRequestAsync(device, PK_CMD_WS2812_CONTROL,
                                 params, 4, NULL, 0, NULL);
    if (req < 0) return req;
    return SendRequestAsync(device, (uint8_t)req);
}

static int PK_WS2812_SendLEDdataExAsync(sPoKeysDevice* device, uint32_t *LEDdata,
                                        uint16_t LEDoffset, uint16_t startLED, uint8_t LEDcount)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    if (LEDcount > 18) return PK_ERR_PARAMETER;

    CreateRequest(device->request, PK_CMD_WS2812_CONTROL,
                  0x10, startLED & 0xFF, startLED >> 8, LEDcount);
    for (uint8_t i = 0; i < LEDcount; i++)
        memcpy(device->request + 8 + i*3, &LEDdata[LEDoffset + i], 3);

    device->request[63] = 0;
    for (int i = 8; i < 63; i++)
        device->request[63] += device->request[i];

    return PK_SendEthRequestNoResponseAsync(device);
}

int PK_WS2812_SendLEDdataAsync(sPoKeysDevice* device, uint32_t *LEDdata,
                               uint16_t startLED, uint8_t LEDcount)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    uint16_t offset = 0;

    while (LEDcount > 0) {
        uint8_t chunk = (LEDcount >= 18) ? 18 : LEDcount;
        int r = PK_WS2812_SendLEDdataExAsync(device, LEDdata, offset,
                                             (uint16_t)(startLED + offset), chunk);
        if (r != PK_OK) return r;
        offset += chunk;
        LEDcount -= chunk;
    }
    return PK_OK;
}

