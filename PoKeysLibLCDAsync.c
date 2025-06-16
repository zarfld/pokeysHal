/**
 * @file PoKeysLibLCDAsync.c
 * @brief Asynchronous LCD helper functions.
 *
 * These routines mirror a subset of the blocking LCD API but use
 * the PoKeysLibAsync request queue so that no network operation
 * blocks the caller.  Responses are parsed via small callbacks and
 * device state is updated when packets arrive.
 *
 * This design keeps the realtime thread free from any waiting on
 * UDP traffic and fits the general asynchronous communication model
 * used throughout the PoKeysHal project.
 */

#include "PoKeysLibHal.h"
#include "PoKeysLibAsync.h"
#include <string.h>

// -------------------------------------------------------------------------
// Helper parser for configuration readback
static int PK_LCD_ConfigParse(sPoKeysDevice *dev, const uint8_t *resp)
{
    if (!dev || !resp) return PK_ERR_GENERIC;
    dev->LCD.Configuration = resp[3];
    dev->LCD.Rows         = resp[4];
    dev->LCD.Columns      = resp[5];
    return PK_OK;
}

int PK_LCDConfigurationGetAsync(sPoKeysDevice* device)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    if (device->info.iLCD == 0) return PK_ERR_NOT_SUPPORTED;

    uint8_t params[1] = { 1 };    // param1=1 means "read"
    int req = CreateRequestAsync(device, PK_CMD_LCD_CONFIGURATION,
                                 params, 1, NULL, 0,
                                 PK_LCD_ConfigParse);
    if (req < 0) return req;
    return SendRequestAsync(device, (uint8_t)req);
}

int PK_LCDConfigurationSetAsync(sPoKeysDevice* device)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    if (device->info.iLCD == 0) return PK_ERR_NOT_SUPPORTED;

    uint8_t cfg_params[4] = {
        0,
        device->LCD.Configuration,
        device->LCD.Rows,
        device->LCD.Columns
    };
    int req = CreateRequestAsync(device, PK_CMD_LCD_CONFIGURATION,
                                 cfg_params, 4, NULL, 0, NULL);
    if (req < 0) return req;
    int r = SendRequestAsync(device, (uint8_t)req);
    if (r < 0) return r;

    // Send initialise command
    uint8_t op_params[1] = { 0x00 };
    req = CreateRequestAsync(device, PK_CMD_LCD_OPERATION,
                             op_params, 1, NULL, 0, NULL);
    if (req < 0) return req;
    r = SendRequestAsync(device, (uint8_t)req);
    if (r < 0) return r;

    // Clear display
    uint8_t clr_params[1] = { 0x10 };
    req = CreateRequestAsync(device, PK_CMD_LCD_OPERATION,
                             clr_params, 1, NULL, 0, NULL);
    if (req < 0) return req;
    return SendRequestAsync(device, (uint8_t)req);
}

int PK_LCDUpdateAsync(sPoKeysDevice* device)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    if (device->info.iLCD == 0) return PK_ERR_NOT_SUPPORTED;

    uint8_t *lines[4] = {
        device->LCD.line1,
        device->LCD.line2,
        device->LCD.line3,
        device->LCD.line4
    };

    for (int n = 0; n < 4; n++) {
        if (device->LCD.RowRefreshFlags & (1 << n)) {
            uint8_t params[2] = { 0x85, (uint8_t)(n + 1) };
            int req = CreateRequestAsyncWithPayload(device,
                    PK_CMD_LCD_OPERATION,
                    params, 2,
                    lines[n], 20,
                    NULL);
            if (req < 0) return req;
            int r = SendRequestAsync(device, (uint8_t)req);
            if (r < 0) return r;
            device->LCD.RowRefreshFlags &= ~(1 << n);
        }
    }
    return PK_OK;
}

int PK_LCDSetCustomCharactersAsync(sPoKeysDevice* device)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    if (device->info.iLCD == 0) return PK_ERR_NOT_SUPPORTED;

    for (int n = 0; n < 8; n++) {
        uint8_t params[1] = { 0x40 };
        uint8_t payload[9];
        payload[0] = (uint8_t)n;
        memcpy(payload + 1, device->LCD.customCharacters[n], 8);

        int req = CreateRequestAsyncWithPayload(device,
                    PK_CMD_LCD_OPERATION,
                    params, 1,
                    payload, 9,
                    NULL);
        if (req < 0) return req;
        int r = SendRequestAsync(device, (uint8_t)req);
        if (r < 0) return r;
    }
    return PK_OK;
}

