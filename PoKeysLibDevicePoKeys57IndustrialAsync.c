/*
 * Asynchronous support for the PoKeys57Industrial device.
 *
 * This file provides non-blocking wrappers around the basic
 * discovery, connect and full I/O update helpers.  The
 * implementation follows the realtime-compatible async
 * communication pattern used in PoKeysLibCoreAsync.c – no
 * blocking socket calls are executed inside these functions.
 */

#include "PoKeysLibHal.h"
#include "PoKeysLibAsync.h"
//#include "PoKeysLibDevicePoKeys57Industrial.h"
#include "PoKeysLibCoreSocketsAsync.h"
#include "hidapi.h"

/* declarations from async core not exposed in headers */
typedef enum { ENUM_IDLE, ENUM_ENUMERATING_1001, ENUM_ENUMERATING_1002, ENUM_DONE } PK_EnumStage;
typedef struct {
    int32_t numDevices;
    struct hid_device_info *devs;
    struct hid_device_info *cur_dev;
    PK_EnumStage state;
} PKUSBEnumerator;

extern int PK_SearchNetworkDevicesAsync_Start(sPoKeysNetworkDeviceSummary* devices, uint32_t timeout_ms, uint32_t serial);
extern int PK_SearchNetworkDevicesAsync_Process(void);
// extern int32_t PK_EnumerateUSBDevicesAsync(PKUSBEnumerator *ctx); no USB in RTAPI !!!
extern sPoKeysDevice* PK_ConnectToDeviceAsync(uint32_t deviceIndex);
#include <string.h>

/* ---------- Device discovery ---------- */

static sPoKeysNetworkDeviceSummary searchBuf[16];
static uint32_t processedIdx = 0;
static int searchActive = 0;

/* Start asynchronous network discovery. */
int PK57i_SearchDeviceAsync_Start(uint32_t timeout_ms)
{
    memset(searchBuf, 0, sizeof(searchBuf));
    processedIdx = 0;
    int r = PK_SearchNetworkDevicesAsync_Start(searchBuf, timeout_ms, 0);
    if (r == PK_OK)
        searchActive = 1;
    else
        searchActive = 0;
    return r;
}

/* Continue processing discovery responses. */
int PK57i_SearchDeviceAsync_Process(sPoKeysNetworkDeviceSummary* device)
{
    if (!searchActive)
        return PK_ERR_GENERIC;

    int r = PK_SearchNetworkDevicesAsync_Process();
    if (r == PK_ERR_TIMEOUT)
    {
        searchActive = 0;
        return r;
    }
    if (r < 0)
        return r;

    /* Check any newly discovered devices */
    for (; processedIdx < 16; processedIdx++)
    {
        if (searchBuf[processedIdx].SerialNumber == 0)
            break; /* no new entry yet */
        if (searchBuf[processedIdx].HWtype == 45)
        {
            if (device)
                *device = searchBuf[processedIdx];
            searchActive = 0;
            return PK_OK_FOUND;
        }
    }

    return PK_OK; /* still searching */
}

/* ---------- Async full I/O update ---------- */

typedef struct {
    sPoKeys57Industrial* inst;
} PK57iUpdateCtx;

static PK57iUpdateCtx updCtx[256];

static int PK57i_Update_Parse(sPoKeysDevice* dev, const uint8_t* resp)
{
    uint8_t id = resp[6];
    PK57iUpdateCtx* c = &updCtx[id];
    if (!c->inst)
        return PK_ERR_GENERIC;
    sPoKeys57Industrial* d = c->inst;
    for (uint32_t i=0;i<8;i++)
    {
        d->digitalOutputsFault[i] = (resp[4] & (1<<i)) ? 1 : 0;
        d->analogOutputsFault[i]  = (resp[5] & (1<<i)) ? 1 : 0;
        d->digitalInputs[i]       = (resp[8] & (1<<i)) ? 1 : 0;
        d->analogInputs[i]        = (uint16_t)resp[16+i*2] | ((uint16_t)resp[17+i*2]<<8);
    }
    c->inst = NULL;
    return PK_OK;
}

int PK57i_UpdateAsync(sPoKeys57Industrial* device, uint8_t resetFaults)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    sPoKeysDevice* dev = (sPoKeysDevice*)device->deviceStructure;

    uint8_t params[4] = {0,0,0,resetFaults};
    for (uint32_t i=0;i<8;i++)
        if (device->digitalOutputsEnable[i])
            params[2] |= (1<<i);

    uint8_t payload[17] = {0};
    for (uint32_t i=0;i<8;i++)
        if (device->digitalOutputs[i])
            payload[0] |= (1<<i);
    for (uint32_t i=0;i<8;i++)
    {
        uint16_t v = device->analogOutputs[i];
        payload[1+i*2] = v & 0xFF;
        payload[2+i*2] = v >> 8;
    }

    int req = CreateRequestAsyncWithPayload(dev, 0x3F,
                                            params, 4,
                                            payload, sizeof(payload),
                                            PK57i_Update_Parse);
    if (req < 0) return req;
    updCtx[req].inst = device;
    return SendRequestAsync(dev, req);
}

/* ---------- Connection helpers ---------- */

typedef enum {
    C_IDLE,
    C_SEARCH_START,
    C_SEARCH_RUN,
    C_CONNECT_NET,
    C_ENUM_START,
    C_ENUM_WAIT,
    C_CONNECT_USB,
    C_DONE
} PK57iConnStage;

static PK57iConnStage connStage = C_IDLE;
static sPoKeysNetworkDeviceSummary foundDev;
static PKUSBEnumerator usbEnum;
static uint32_t usbIdx;

sPoKeys57Industrial* PK57i_ConnectAsync(void)
{
    static sPoKeys57Industrial* result = NULL;
    static sPoKeysDevice* tmp = NULL;

    switch (connStage)
    {
        case C_IDLE:
            result = NULL;
            connStage = C_SEARCH_START;
            break;

        case C_SEARCH_START:
            if (PK57i_SearchDeviceAsync_Start(100) == PK_OK)
                connStage = C_SEARCH_RUN;
            else
                connStage = C_ENUM_START;
            break;

        case C_SEARCH_RUN:
        {
            int r = PK57i_SearchDeviceAsync_Process(&foundDev);
            if (r == PK_OK_FOUND)
                connStage = C_CONNECT_NET;
            else if (r == PK_ERR_TIMEOUT)
                connStage = C_ENUM_START;
            break;
        }
        case C_CONNECT_NET:
            tmp = PK_ConnectToNetworkDeviceAsync(&foundDev);
            if (tmp)
            {
                result = hal_malloc(sizeof(sPoKeys57Industrial));
                if (!result) { connStage = C_IDLE; break; }
                memset(result,0,sizeof(*result));
                result->deviceStructure = tmp;
                connStage = C_DONE;
            }
            break;
/* no USB in RTAPI !!!
        case C_ENUM_START:
            memset(&usbEnum,0,sizeof(usbEnum));
            connStage = C_ENUM_WAIT;
            break;

        case C_ENUM_WAIT:
            if (PK_EnumerateUSBDevicesAsync(&usbEnum) >= 0)
            {
                usbIdx = 0;
                connStage = C_CONNECT_USB;
            }
            break;
*/
        case C_CONNECT_USB:
            if (usbIdx < (uint32_t)usbEnum.numDevices)
            {
                tmp = PK_ConnectToDeviceAsync(usbIdx);
                if (tmp)
                {
                    if (tmp->DeviceData.DeviceType == 45)
                    {
                        result = hal_malloc(sizeof(sPoKeys57Industrial));
                        if (!result) { connStage = C_IDLE; break; }
                        memset(result,0,sizeof(*result));
                        result->deviceStructure = tmp;
                        connStage = C_DONE;
                    }
                    else
                    {
                        PK_DisconnectDevice(tmp);
                        usbIdx++;
                    }
                }
            }
            else
            {
                connStage = C_IDLE; /* not found */
            }
            break;

        case C_DONE:
            connStage = C_IDLE;
            return result;
    }
    return NULL;
}

void PK57i_DisconnectAsync(sPoKeys57Industrial* device)
{
    PK57i_Disconnect(device);
}

