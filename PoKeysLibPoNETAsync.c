#include "PoKeysLibHal.h"
#include "PoKeysLibAsync.h"
#include <string.h>

/*
 * Asynchronous PoNET helper functions.
 * These mirror the blocking PoNET routines found in PoKeysLibPoNET.c
 * but use the non-blocking CreateRequestAsync/SendRequestAsync framework.
 *
 * Designed for realtime threads: minimal CPU usage and no blocking IO.
 */

static int PK_PoNET_StatusParse(sPoKeysDevice *dev, const uint8_t *resp)
{
    if (!dev || !resp) return PK_ERR_GENERIC;
    dev->PoNETmodule.PoNETstatus = resp[8];
    return PK_OK;
}

static int PK_PoNET_ModuleSettingsParse(sPoKeysDevice *dev, const uint8_t *resp)
{
    if (!dev || !resp) return PK_ERR_GENERIC;
    dev->PoNETmodule.i2cAddress   = resp[8];
    dev->PoNETmodule.moduleType   = resp[9];
    dev->PoNETmodule.moduleSize   = resp[10];
    dev->PoNETmodule.moduleOptions= resp[11];
    return PK_OK;
}

static int PK_PoNET_ModuleStatusParse(sPoKeysDevice *dev, const uint8_t *resp)
{
    if (!dev || !resp) return PK_ERR_GENERIC;
    if (resp[3] != 1 || resp[8] != 0) return PK_ERR_GENERIC;
    memcpy(dev->PoNETmodule.statusIn, resp + 9, 16);
    return PK_OK;
}

static int PK_PoNET_ModuleLightParse(sPoKeysDevice *dev, const uint8_t *resp)
{
    if (!dev || !resp) return PK_ERR_GENERIC;
    if (resp[8] != 0) return PK_ERR_GENERIC;
    dev->PoNETmodule.lightValue = resp[9];
    return PK_OK;
}

int PK_PoNETGetPoNETStatusAsync(sPoKeysDevice* device)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    uint8_t params[1] = { PONET_OP_GET_STATUS };
    int req = CreateRequestAsync(device, PK_CMD_POI2C_COMMUNICATION,
                                 params, 1, NULL, 0,
                                 PK_PoNET_StatusParse);
    if (req < 0) return req;
    return SendRequestAsync(device, req);
}

int PK_PoNETGetModuleSettingsAsync(sPoKeysDevice* device)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    uint8_t params[2] = { PONET_OP_GET_MODULE_SETTINGS,
                          device->PoNETmodule.moduleID };
    int req = CreateRequestAsync(device, PK_CMD_POI2C_COMMUNICATION,
                                 params, 2, NULL, 0,
                                 PK_PoNET_ModuleSettingsParse);
    if (req < 0) return req;
    return SendRequestAsync(device, req);
}

int PK_PoNETGetModuleStatusRequestAsync(sPoKeysDevice* device)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    uint8_t params[3] = { PONET_OP_GET_MODULE_DATA, 0x10,
                          device->PoNETmodule.moduleID };
    int req = CreateRequestAsync(device, PK_CMD_POI2C_COMMUNICATION,
                                 params, 3, NULL, 0, NULL);
    if (req < 0) return req;
    return SendRequestAsync(device, req);
}

int PK_PoNETGetModuleStatusAsync(sPoKeysDevice* device)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    uint8_t params[2] = { PONET_OP_GET_MODULE_DATA, 0x30 };
    int req = CreateRequestAsync(device, PK_CMD_POI2C_COMMUNICATION,
                                 params, 2, NULL, 0,
                                 PK_PoNET_ModuleStatusParse);
    if (req < 0) return req;
    return SendRequestAsync(device, req);
}

int PK_PoNETSetModuleStatusAsync(sPoKeysDevice* device)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    uint8_t params[2] = { PONET_OP_SET_MODULE_DATA,
                          device->PoNETmodule.moduleID };
    int req = CreateRequestAsyncWithPayload(device, PK_CMD_POI2C_COMMUNICATION,
                                            params, 2,
                                            device->PoNETmodule.statusOut, 16,
                                            NULL);
    if (req < 0) return req;
    return SendRequestAsync(device, req);
}

int PK_PoNETSetModulePWMAsync(sPoKeysDevice* device)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    uint8_t params[3] = { PONET_OP_SET_PWM_VALUE,
                          device->PoNETmodule.moduleID,
                          device->PoNETmodule.PWMduty };
    int req = CreateRequestAsync(device, PK_CMD_POI2C_COMMUNICATION,
                                 params, 3, NULL, 0, NULL);
    if (req < 0) return req;
    return SendRequestAsync(device, req);
}

int PK_PoNETGetModuleLightRequestAsync(sPoKeysDevice* device)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    uint8_t params[3] = { PONET_OP_GET_LIGHT_SENSOR, 0x10,
                          device->PoNETmodule.moduleID };
    int req = CreateRequestAsync(device, PK_CMD_POI2C_COMMUNICATION,
                                 params, 3, NULL, 0, NULL);
    if (req < 0) return req;
    return SendRequestAsync(device, req);
}

int PK_PoNETGetModuleLightAsync(sPoKeysDevice* device)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    uint8_t params[3] = { PONET_OP_GET_LIGHT_SENSOR, 0x30,
                          device->PoNETmodule.moduleID };
    int req = CreateRequestAsync(device, PK_CMD_POI2C_COMMUNICATION,
                                 params, 3, NULL, 0,
                                 PK_PoNET_ModuleLightParse);
    if (req < 0) return req;
    return SendRequestAsync(device, req);
}

