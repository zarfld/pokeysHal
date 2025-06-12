#include "PoKeysLibHal.h"
#include "PoKeysLibAsync.h"

// Context structure for asynchronous I2C operations
typedef struct {
    uint8_t *status_ptr;
    uint8_t *read_bytes_ptr;
    uint8_t *buffer_ptr;
    uint8_t max_len;
    uint8_t *scan_results_ptr;
    uint8_t max_devices;
    uint8_t used;
} I2CAsyncContext;

// Simple table indexed by request ID
static I2CAsyncContext i2c_ctx[256];

static int PK_I2C_StatusParse(sPoKeysDevice *dev, const uint8_t *resp)
{
    uint8_t id = resp[6];
    I2CAsyncContext *c = &i2c_ctx[id];
    if (c->status_ptr)
        *(c->status_ptr) = resp[3];
    c->used = 0;
    return PK_OK;
}

static int PK_I2C_ReadStatusParse(sPoKeysDevice *dev, const uint8_t *resp)
{
    uint8_t id = resp[6];
    I2CAsyncContext *c = &i2c_ctx[id];
    if (c->status_ptr)
        *(c->status_ptr) = resp[3];
    if (c->read_bytes_ptr)
        *(c->read_bytes_ptr) = 0;

    if (resp[3] == PK_I2C_STAT_COMPLETE && c->read_bytes_ptr && c->buffer_ptr) {
        uint8_t count = resp[9];
        if (count > 32)
            count = 32;
        *(c->read_bytes_ptr) = count;
        for (uint8_t i = 0; i < c->max_len && i < count; i++)
            c->buffer_ptr[i] = resp[10 + i];
    }
    c->used = 0;
    return PK_OK;
}

static int PK_I2C_BusScanParse(sPoKeysDevice *dev, const uint8_t *resp)
{
    uint8_t id = resp[6];
    I2CAsyncContext *c = &i2c_ctx[id];
    if (c->status_ptr)
        *(c->status_ptr) = resp[3];
    if (resp[3] == PK_I2C_STAT_COMPLETE && c->scan_results_ptr) {
        for (uint8_t i = 0; i < c->max_devices; i++) {
            c->scan_results_ptr[i] =
                ((resp[9 + i / 8] & (1 << (i % 8))) > 0) ? PK_I2C_STAT_OK : PK_I2C_STAT_ERR;
        }
    }
    c->used = 0;
    return PK_OK;
}

int PK_I2CSetStatusAsync(sPoKeysDevice* device, uint8_t activated)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    uint8_t params[1] = { activated };
    int req = CreateRequestAsync(device, 0xDB, params, 1, NULL, 0, NULL);
    if (req < 0) return req;
    return SendRequestAsync(device, req);
}

int PK_I2CGetStatusAsync(sPoKeysDevice* device, uint8_t* activated)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    uint8_t params[1] = { 0x02 };
    int req = CreateRequestAsync(device, 0xDB, params, 1, NULL, 0, PK_I2C_StatusParse);
    if (req < 0) return req;
    i2c_ctx[req].status_ptr = activated;
    i2c_ctx[req].used = 1;
    return SendRequestAsync(device, req);
}

int PK_I2CWriteStartAsync(sPoKeysDevice* device, uint8_t address, uint8_t* buffer, uint8_t iDataLength)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    if (iDataLength > 32) iDataLength = 32;
    uint8_t params[3] = { 0x10, address, iDataLength };
    int req = CreateRequestAsyncWithPayload(device, 0xDB, params, 3, buffer, iDataLength, NULL);
    if (req < 0) return req;
    return SendRequestAsync(device, req);
}

int PK_I2CWriteAndReadStartAsync(sPoKeysDevice* device, uint8_t address, uint8_t* buffer, uint8_t iDataLengthWrite, uint8_t iDataLengthRead)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    if (iDataLengthWrite > 32) iDataLengthWrite = 32;
    uint8_t params[4] = { 0x10, address, iDataLengthWrite, iDataLengthRead };
    int req = CreateRequestAsyncWithPayload(device, 0xDB, params, 4, buffer, iDataLengthWrite, NULL);
    if (req < 0) return req;
    return SendRequestAsync(device, req);
}

int PK_I2CWriteStatusGetAsync(sPoKeysDevice* device, uint8_t* status)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    uint8_t params[1] = { 0x11 };
    int req = CreateRequestAsync(device, 0xDB, params, 1, NULL, 0, PK_I2C_StatusParse);
    if (req < 0) return req;
    i2c_ctx[req].status_ptr = status;
    i2c_ctx[req].used = 1;
    return SendRequestAsync(device, req);
}

int PK_I2CReadStartAsync(sPoKeysDevice* device, uint8_t address, uint8_t iDataLength)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    if (iDataLength > 32) iDataLength = 32;
    uint8_t params[3] = { 0x20, address, iDataLength };
    int req = CreateRequestAsync(device, 0xDB, params, 3, NULL, 0, NULL);
    if (req < 0) return req;
    return SendRequestAsync(device, req);
}

int PK_I2CReadStatusGetAsync(sPoKeysDevice* device, uint8_t* status, uint8_t* iReadBytes, uint8_t* buffer, uint8_t iMaxBufferLength)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    uint8_t params[1] = { 0x21 };
    int req = CreateRequestAsync(device, 0xDB, params, 1, NULL, 0, PK_I2C_ReadStatusParse);
    if (req < 0) return req;
    i2c_ctx[req].status_ptr = status;
    i2c_ctx[req].read_bytes_ptr = iReadBytes;
    i2c_ctx[req].buffer_ptr = buffer;
    i2c_ctx[req].max_len = iMaxBufferLength;
    i2c_ctx[req].used = 1;
    return SendRequestAsync(device, req);
}

int PK_I2CBusScanStartAsync(sPoKeysDevice* device)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    uint8_t params[1] = { 0x30 };
    int req = CreateRequestAsync(device, 0xDB, params, 1, NULL, 0, NULL);
    if (req < 0) return req;
    return SendRequestAsync(device, req);
}

int PK_I2CBusScanGetResultsAsync(sPoKeysDevice* device, uint8_t* status, uint8_t* presentDevices, uint8_t iMaxDevices)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    if (iMaxDevices > 128) iMaxDevices = 128;
    uint8_t params[1] = { 0x31 };
    int req = CreateRequestAsync(device, 0xDB, params, 1, NULL, 0, PK_I2C_BusScanParse);
    if (req < 0) return req;
    i2c_ctx[req].status_ptr = status;
    i2c_ctx[req].scan_results_ptr = presentDevices;
    i2c_ctx[req].max_devices = iMaxDevices;
    i2c_ctx[req].used = 1;
    return SendRequestAsync(device, req);
}


