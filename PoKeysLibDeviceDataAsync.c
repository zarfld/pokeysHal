/*
 * PoKeysLibDeviceDataAsync.c
 *
 * Non-blocking helpers for retrieving device information using the
 * asynchronous transaction framework provided in PoKeysLibAsync.c.
 *
 * The routines in this file are designed for realtime usage where no
 * blocking I/O may occur. Each helper simply queues an async request
 * which is later processed by PK_ReceiveAndDispatch() in a separate
 * non-realtime context.
 */

#include "PoKeysLibHal.h"
#include "PoKeysLibAsync.h"
#include <string.h>

/* Response parsers -------------------------------------------------------- */

static int PK_Parse_DeviceData(sPoKeysDevice *dev, const uint8_t *resp)
{
    if (!dev || !resp) return PK_ERR_GENERIC;

    sPoKeysDevice_Data *data = &dev->DeviceData;

    data->FirmwareVersionMajor = resp[4];
    data->FirmwareVersionMinor = resp[5];

    data->SerialNumber = (int)resp[12] |
                         ((int)resp[13] << 8) |
                         ((int)resp[14] << 16) |
                         ((int)resp[15] << 24);

    data->HWtype    = resp[18];
    data->ProductID = resp[57];

    memset(data->DeviceName, 0, sizeof(data->DeviceName));
    memcpy(data->DeviceName, resp + 31, 10);

    memset(data->BuildDate, 0, sizeof(data->BuildDate));
    memcpy(data->BuildDate, resp + 20, 11);

    return PK_OK;
}

static int PK_Parse_UserID(sPoKeysDevice *dev, const uint8_t *resp)
{
    if (!dev || !resp) return PK_ERR_GENERIC;

    dev->DeviceData.UserID          = resp[2];
    dev->DeviceData.DeviceLockStatus= resp[3];
    return PK_OK;
}

static int PK_Parse_NetworkInfo(sPoKeysDevice *dev, const uint8_t *resp)
{
    if (!dev || !resp) return PK_ERR_GENERIC;

    if (!dev->netDeviceData)
        dev->netDeviceData = hal_malloc(sizeof(sPoKeysNetworkDeviceInfo));
    if (!dev->netDeviceData) return PK_ERR_GENERIC;

    dev->netDeviceData->DHCP = resp[8];
    memcpy(dev->netDeviceData->IPAddressSetup,   resp + 9,  4);
    memcpy(dev->netDeviceData->IPAddressCurrent, resp + 13, 4);
    dev->netDeviceData->TCPtimeout = (uint16_t)resp[17] | ((uint16_t)resp[18] << 8);
    memcpy(dev->netDeviceData->DefaultGateway, resp + 19, 4);
    memcpy(dev->netDeviceData->Subnetmask,     resp + 23, 4);

    if ((resp[27] & 0xA0) == 0xA0)
        dev->netDeviceData->AdditionalNetworkOptions = resp[27];
    else
        dev->netDeviceData->AdditionalNetworkOptions = 0;

    return PK_OK;
}

static int PK_Parse_DeviceName(sPoKeysDevice *dev, const uint8_t *resp)
{
    if (!dev || !resp) return PK_ERR_GENERIC;
    sPoKeysDevice_Data *data = &dev->DeviceData;

    /* If available, try the long name (bytes 35..54) */
    int match = 1;
    for (int i=0;i<10;i++)
        if (resp[8+i] != resp[35+i]) { match=0; break; }

    if (match)
        memcpy(data->DeviceName, resp + 35, 20);
    else
        memcpy(data->DeviceName, resp + 8, 10);

    if (strlen(data->DeviceName) == 0)
        strcpy(data->DeviceName, data->DeviceTypeName);

    return PK_OK;
}

/* Public asynchronous helpers ------------------------------------------- */

int PK_DeviceDataGetAsync(sPoKeysDevice* device)
{
    if (!device) return PK_ERR_NOT_CONNECTED;

    /* Basic device info (serial, fw version, etc.) */
    CreateRequestAsync(device, PK_CMD_READ_DEVICE_DATA,
                       NULL, 0, NULL, 0,
                       PK_Parse_DeviceData);

    /* User ID and lock status */
    CreateRequestAsync(device, PK_CMD_READ_USER_ID_LOCK,
                       NULL, 0, NULL, 0,
                       PK_Parse_UserID);

    /* Network settings (if supported, parser will allocate structure) */
    CreateRequestAsync(device, PK_CMD_NETWORK_SETTINGS,
                       (const uint8_t[]){0}, 1, NULL, 0,
                       PK_Parse_NetworkInfo);

    /* Device name */
    CreateRequestAsync(device, PK_CMD_DEVICE_NAME,
                       NULL, 0, NULL, 0,
                       PK_Parse_DeviceName);

    return PK_OK;
}

