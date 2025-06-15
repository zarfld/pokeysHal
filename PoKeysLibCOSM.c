#include "PoKeysLibHal.h"
#include "PoKeysLibCore.h"
#include "PoKeysLibAsync.h"
#include <string.h>

int32_t PK_COSMSettingsGet(sPoKeysDevice* device)
{
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    // Request operation 0 - read basic settings
    CreateRequest(device->request, PK_CMD_COSM_SETTINGS, 0, 0, 0, 0);
    if (SendRequest(device) != PK_OK) return PK_ERR_TRANSFER;

    device->COSM.updateRate = device->response[9] | (device->response[10] << 8);
    memcpy(device->COSM.serverIP, device->response + 11, 4);
    device->COSM.requestType = device->response[15];
    device->COSM.lastStatusCode = device->response[16] | (device->response[17] << 8);
    device->COSM.serverPort = device->response[18] | (device->response[19] << 8);

    // Read HTTP headers - operations 1..5
    for (int page = 0; page < 5; page++)
    {
        CreateRequest(device->request, PK_CMD_COSM_SETTINGS, page + 1, 0, 0, 0);
        if (SendRequest(device) != PK_OK) return PK_ERR_TRANSFER;
        memcpy(device->COSM.requestHeaders[page], device->response + 9, 50);
    }

    return PK_OK;
}

int32_t PK_COSMSettingsSet(sPoKeysDevice* device)
{
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    // Operation 10 - set basic settings
    CreateRequest(device->request, PK_CMD_COSM_SETTINGS, 10, 0, 0, 0);
    device->request[9]  = device->COSM.updateRate & 0xFF;
    device->request[10] = (device->COSM.updateRate >> 8) & 0xFF;
    memcpy(device->request + 11, device->COSM.serverIP, 4);
    device->request[15] = device->COSM.requestType;
    device->request[16] = device->COSM.serverPort & 0xFF;
    device->request[17] = (device->COSM.serverPort >> 8) & 0xFF;
    memcpy(device->request + 18, device->COSM.protocolDescription, 46);
    if (SendRequest(device) != PK_OK) return PK_ERR_TRANSFER;

    // Send request header pages 0..4 using operations 11..15
    for (int page = 0; page < 5; page++)
    {
        CreateRequest(device->request, PK_CMD_COSM_SETTINGS, 11 + page, 0, 0, 0);
        memcpy(device->request + 9, device->COSM.requestHeaders[page], 50);
        if (SendRequest(device) != PK_OK) return PK_ERR_TRANSFER;
    }

    return PK_OK;
}
