#include "PoKeysLibHal.h"
#include "PoKeysLibCore.h"
#include "PoKeysLibAsync.h"
#include <string.h>

int32_t PK_SecurityStatusGet(sPoKeysDevice* device, uint8_t* level, uint8_t* seed)
{
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    CreateRequest(device->request, PK_CMD_SECURITY_STATUS_GET, 0, 0, 0, 0);
    if (SendRequest(device) != PK_OK) return PK_ERR_TRANSFER;

    if (level) *level = device->response[8];
    if (seed) memcpy(seed, device->response + 9, 32);
    return PK_OK;
}

int32_t PK_UserAuthorise(sPoKeysDevice* device, uint8_t level, const uint8_t* hash, uint8_t* status)
{
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    CreateRequest(device->request, PK_CMD_USER_AUTHORISE, level, 0, 0, 0);
    memcpy(device->request + 8, hash, 20);
    if (SendRequest(device) != PK_OK) return PK_ERR_TRANSFER;

    if (status) *status = device->response[8];
    return PK_OK;
}

int32_t PK_UserPasswordSet(sPoKeysDevice* device, uint8_t defaultLevel, const uint8_t* password)
{
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    CreateRequest(device->request, PK_CMD_USER_PASSWORD_SET, defaultLevel, 0, 0, 0);
    memcpy(device->request + 8, password, 32);
    if (SendRequest(device) != PK_OK) return PK_ERR_TRANSFER;

    return PK_OK;
}
