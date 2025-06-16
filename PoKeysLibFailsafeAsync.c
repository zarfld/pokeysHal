#include "PoKeysLibHal.h"
#include "PoKeysLibAsync.h"
#include <string.h>

/*
 * Asynchronous failsafe configuration helpers.
 *
 * These wrappers mirror PK_FailsafeSettingsGet and
 * PK_FailsafeSettingsSet but queue the UDP requests using the
 * generic async framework so the caller never blocks while
 * waiting for network traffic. This design is realtime-safe.
 */

static int PK_Failsafe_Parse(sPoKeysDevice *dev, const uint8_t *resp)
{
    if (!dev || !resp) return PK_ERR_GENERIC;

    dev->failsafeSettings.bFailSafeEnabled    = resp[3];
    dev->failsafeSettings.bFailSafePeripherals = resp[4];

    memcpy(dev->failsafeSettings.bFailSafeIO,       resp + 8, 7);
    memcpy(dev->failsafeSettings.bFailSafePoExtBus, resp + 15, 10);
    memcpy(dev->failsafeSettings.bFailSafePWM,      resp + 25, 6);
    return PK_OK;
}

int PK_FailsafeSettingsGetAsync(sPoKeysDevice* device)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    uint8_t params[1] = { 0x00 }; /* subcommand 0 */
    int req = CreateRequestAsync(device, PK_CMD_FAILSAFE_SETTINGS,
                                 params, 1, NULL, 0,
                                 PK_Failsafe_Parse);
    if (req < 0) return req;
    return SendRequestAsync(device, req);
}

int PK_FailsafeSettingsSetAsync(sPoKeysDevice* device)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    uint8_t params[4] = { 0x01,
                          device->failsafeSettings.bFailSafeEnabled,
                          device->failsafeSettings.bFailSafePeripherals,
                          0 };

    uint8_t payload[23];
    memcpy(payload, device->failsafeSettings.bFailSafeIO, 7);
    memcpy(payload + 7, device->failsafeSettings.bFailSafePoExtBus, 10);
    memcpy(payload + 17, device->failsafeSettings.bFailSafePWM, 6);

    int req = CreateRequestAsyncWithPayload(device, PK_CMD_FAILSAFE_SETTINGS,
                                            params, 4, payload, sizeof(payload),
                                            NULL);
    if (req < 0) return req;
    return SendRequestAsync(device, req);
}

