#include "PoKeysLibHal.h"
#include "PoKeysLibAsync.h"
#include <string.h>

typedef struct {
    uint8_t *level_ptr;
    uint8_t *seed_ptr;
    uint8_t *status_ptr;
    uint8_t used;
} SecurityAsyncContext;

static SecurityAsyncContext sec_ctx[256];

static int PK_SecurityStatus_Parse(sPoKeysDevice *dev, const uint8_t *resp)
{
    uint8_t id = resp[6];
    SecurityAsyncContext *c = &sec_ctx[id];
    if (c->level_ptr)
        *(c->level_ptr) = resp[8];
    if (c->seed_ptr)
        memcpy(c->seed_ptr, resp + 9, 32);
    c->used = 0;
    return PK_OK;
}

static int PK_UserAuthorise_Parse(sPoKeysDevice *dev, const uint8_t *resp)
{
    uint8_t id = resp[6];
    SecurityAsyncContext *c = &sec_ctx[id];
    if (c->status_ptr)
        *(c->status_ptr) = resp[8];
    c->used = 0;
    return PK_OK;
}

int PK_SecurityStatusGetAsync(sPoKeysDevice* device, uint8_t* level, uint8_t* seed)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    int req = CreateRequestAsync(device, PK_CMD_SECURITY_STATUS_GET, NULL, 0, NULL, 0, PK_SecurityStatus_Parse);
    if (req < 0) return req;
    sec_ctx[req].level_ptr = level;
    sec_ctx[req].seed_ptr = seed;
    sec_ctx[req].used = 1;
    return SendRequestAsync(device, req);
}

int PK_UserAuthoriseAsync(sPoKeysDevice* device, uint8_t level, const uint8_t* hash, uint8_t* status)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    uint8_t params[1] = { level };
    int req = CreateRequestAsyncWithPayload(device, PK_CMD_USER_AUTHORISE, params, 1, hash, 20, PK_UserAuthorise_Parse);
    if (req < 0) return req;
    sec_ctx[req].status_ptr = status;
    sec_ctx[req].used = 1;
    return SendRequestAsync(device, req);
}

int PK_UserPasswordSetAsync(sPoKeysDevice* device, uint8_t defaultLevel, const uint8_t* password)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    uint8_t params[1] = { defaultLevel };
    int req = CreateRequestAsyncWithPayload(device, PK_CMD_USER_PASSWORD_SET, params, 1, password, 32, NULL);
    if (req < 0) return req;
    return SendRequestAsync(device, req);
}
