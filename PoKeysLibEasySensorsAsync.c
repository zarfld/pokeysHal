/*
 * Asynchronous EasySensors helpers using the PoKeysLibAsync framework.
 *
 * These functions mirror the blocking EasySensors routines in
 * PoKeysLibEasySensors.c but avoid any blocking socket I/O.
 * Designed for realtime usage with minimal CPU overhead and
 * asynchronous non-blocking communication.
 */

#include "PoKeysLibHal.h"
#include "PoKeysLibAsync.h"
#include <string.h>

typedef struct {
    sPoKeysEasySensor *sensor_ptr; /* pointer to first sensor in this request */
    uint8_t count;                 /* number of sensors parsed */
    uint8_t used;
} EasySensorAsyncContext;

static EasySensorAsyncContext es_ctx[256];

/* Parse EasySensor configuration response */
static int PK_EasySensorSetup_Parse(sPoKeysDevice *dev, const uint8_t *resp)
{
    uint8_t id = resp[6];
    EasySensorAsyncContext *c = &es_ctx[id];
    if (!c->sensor_ptr) {
        c->used = 0;
        return PK_ERR_GENERIC;
    }
    sPoKeysEasySensor *es = c->sensor_ptr;
    es->sensorValue = 0; /* clear value pointer */
    es->sensorType = resp[8];
    es->sensorReadingID = resp[9];
    es->sensorRefreshPeriod = resp[10];
    es->sensorFailsafeConfig = resp[11];
    memcpy(es->sensorID, resp + 12, 8);
    c->used = 0;
    return PK_OK;
}

/* Parse EasySensor values response */
static int PK_EasySensorValues_Parse(sPoKeysDevice *dev, const uint8_t *resp)
{
    uint8_t id = resp[6];
    EasySensorAsyncContext *c = &es_ctx[id];
    if (!c->sensor_ptr) {
        c->used = 0;
        return PK_ERR_GENERIC;
    }
    for (uint8_t t = 0; t < c->count; t++) {
        sPoKeysEasySensor *es = &c->sensor_ptr[t];
        *es->sensorValue = ((int32_t)resp[8 + t*4]) |
                           ((int32_t)resp[8 + t*4 + 1] << 8) |
                           ((int32_t)resp[8 + t*4 + 2] << 16) |
                           ((int32_t)resp[8 + t*4 + 3] << 24);
        es->sensorOKstatus = (resp[4 + t/8] >> (t % 8)) & 1;
    }
    c->used = 0;
    return PK_OK;
}

int PK_EasySensorsSetupGetAsync(sPoKeysDevice* device)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    if (device->info.iEasySensors == 0) return PK_ERR_NOT_SUPPORTED;

    for (uint32_t i = 0; i < device->info.iEasySensors; i++) {
        sPoKeysEasySensor *es = &device->EasySensors[i];
        uint8_t params[4] = { (uint8_t)i, 1, 0, 0 };
        int req = CreateRequestAsync(device, PK_CMD_SENSORS_SETUP_57,
                                     params, 4, NULL, 0,
                                     PK_EasySensorSetup_Parse);
        if (req < 0) return req;
        es_ctx[req].sensor_ptr = es;
        es_ctx[req].count = 1;
        es_ctx[req].used = 1;
        int err = SendRequestAsync(device, req);
        if (err != PK_OK) return err;
    }
    return PK_OK;
}

int PK_EasySensorsSetupSetAsync(sPoKeysDevice* device)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    if (device->info.iEasySensors == 0) return PK_ERR_NOT_SUPPORTED;

    for (uint32_t i = 0; i < device->info.iEasySensors; i++) {
        sPoKeysEasySensor *es = &device->EasySensors[i];
        uint8_t params[4] = { (uint8_t)i, 1, 1, 0 };
        uint8_t payload[12];
        payload[0] = es->sensorType;
        payload[1] = es->sensorReadingID;
        payload[2] = es->sensorRefreshPeriod;
        payload[3] = es->sensorFailsafeConfig;
        memcpy(payload + 4, es->sensorID, 8);
        int req = CreateRequestAsyncWithPayload(device, PK_CMD_SENSORS_SETUP_57,
                                                params, 4, payload, 12, NULL);
        if (req < 0) return req;
        int err = SendRequestAsync(device, req);
        if (err != PK_OK) return err;
    }
    return PK_OK;
}

int PK_EasySensorsValueGetAllAsync(sPoKeysDevice* device)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    if (device->info.iEasySensors == 0) return PK_ERR_NOT_SUPPORTED;

    for (uint32_t i = 0; i < device->info.iEasySensors; i += 13) {
        uint8_t readNum = 13;
        if (i + 13 > device->info.iEasySensors)
            readNum = device->info.iEasySensors - i;
        uint8_t params[4] = { (uint8_t)i, readNum, 0, 0 };
        int req = CreateRequestAsync(device, PK_CMD_SENSOR_VALUES_READ_57,
                                     params, 4, NULL, 0,
                                     PK_EasySensorValues_Parse);
        if (req < 0) return req;
        es_ctx[req].sensor_ptr = &device->EasySensors[i];
        es_ctx[req].count = readNum;
        es_ctx[req].used = 1;
        int err = SendRequestAsync(device, req);
        if (err != PK_OK) return err;
    }
    return PK_OK;
}

