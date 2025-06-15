#include "PoKeysLibHal.h"
#include "PoKeysLibAsync.h"
#include <string.h>

typedef struct {
    sPoKeysCOSMSettings *settings;
    uint8_t page;
    uint8_t used;
} COSMAsyncContext;

static COSMAsyncContext cosm_ctx[256];

static int PK_COSM_ParseBasic(sPoKeysDevice *dev, const uint8_t *resp)
{
    uint8_t id = resp[6];
    COSMAsyncContext *c = &cosm_ctx[id];
    sPoKeysCOSMSettings *s = c->settings;
    if (s) {
        s->updateRate = resp[9] | (resp[10] << 8);
        memcpy(s->serverIP, resp + 11, 4);
        s->requestType = resp[15];
        s->lastStatusCode = resp[16] | (resp[17] << 8);
        s->serverPort = resp[18] | (resp[19] << 8);
    }
    c->used = 0;
    return PK_OK;
}

static int PK_COSM_ParseHeader(sPoKeysDevice *dev, const uint8_t *resp)
{
    uint8_t id = resp[6];
    COSMAsyncContext *c = &cosm_ctx[id];
    if (c->settings) {
        memcpy(c->settings->requestHeaders[c->page], resp + 9, 50);
    }
    c->used = 0;
    return PK_OK;
}

int PK_COSMSettingsGetAsync(sPoKeysDevice* device, sPoKeysCOSMSettings* settings)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    uint8_t param0[1] = { 0 };
    int req = CreateRequestAsync(device, PK_CMD_COSM_SETTINGS, param0, 1, NULL, 0, PK_COSM_ParseBasic);
    if (req < 0) return req;
    cosm_ctx[req].settings = settings;
    cosm_ctx[req].used = 1;
    int err = SendRequestAsync(device, req);
    if (err != PK_OK) return err;

    // Queue additional requests for headers
    for (int p=0; p<5; p++) {
        uint8_t param[1] = { (uint8_t)(p+1) };
        int r = CreateRequestAsync(device, PK_CMD_COSM_SETTINGS, param, 1, NULL, 0, PK_COSM_ParseHeader);
        if (r < 0) return r;
        cosm_ctx[r].settings = settings;
        cosm_ctx[r].page = p;
        cosm_ctx[r].used = 1;
        err = SendRequestAsync(device, r);
        if (err != PK_OK) return err;
    }
    return PK_OK;
}

int PK_COSMSettingsSetAsync(sPoKeysDevice* device, const sPoKeysCOSMSettings* settings)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    uint8_t param10[1] = { 10 };
    uint8_t tmp[64] = {0};
    tmp[1] = 0; // not used, but ensure zero
    tmp[0] = 0; // not used
    // Build payload for operation 10 starting at byte 9
    tmp[0] = settings->updateRate & 0xFF;
    tmp[1] = (settings->updateRate >> 8) & 0xFF;
    memcpy(tmp + 2, settings->serverIP, 4);
    tmp[6] = settings->requestType;
    tmp[7] = settings->serverPort & 0xFF;
    tmp[8] = (settings->serverPort >> 8) & 0xFF;
    memcpy(tmp + 9, settings->protocolDescription, 46);
    int req = CreateRequestAsyncWithPayload(device, PK_CMD_COSM_SETTINGS, param10, 1, tmp, 55, NULL);
    if (req < 0) return req;
    int err = SendRequestAsync(device, req);
    if (err != PK_OK) return err;

    // send headers
    for (int p=0; p<5; p++) {
        uint8_t param[1] = { (uint8_t)(11+p) };
        req = CreateRequestAsyncWithPayload(device, PK_CMD_COSM_SETTINGS, param, 1, settings->requestHeaders[p], 50, NULL);
        if (req < 0) return req;
        err = SendRequestAsync(device, req);
        if (err != PK_OK) return err;
    }
    return PK_OK;
}
