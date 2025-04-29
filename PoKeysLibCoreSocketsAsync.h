#ifndef POKEYSLIBCORESOCKETSASYNC_H
#define POKEYSLIBCORESOCKETSASYNC_H

#include "PoKeysLibHal.h"  // sPoKeysDevice, sPoKeysNetworkDeviceSummary

#ifdef __cplusplus
extern "C" {
#endif

// Global (or device-specific) discovery context
typedef struct {
    int txSocket;
    uint64_t start_time_us;
    uint32_t timeout_us;
    sPoKeysNetworkDeviceSummary* devices;
    int nrOfDetectedBoards;
    uint32_t serialNumberToFind;
} PoKeysDiscoveryContext;

uint32_t* PK_GetBroadcastAddressesAsync(void);

int32_t PK_SearchNetworkDevicesAsync(sPoKeysNetworkDeviceSummary* devices, uint32_t timeout_ms, uint32_t serialNumberToFind);

sPoKeysDevice* PK_ConnectToNetworkDeviceAsync(const sPoKeysNetworkDeviceSummary* device);

int32_t PK_SendEthRequestAsync(sPoKeysDevice* device);

int32_t PK_SendEthRequestNoResponseAsync(sPoKeysDevice* device);

int32_t PK_SendEthRequestBigAsync(sPoKeysDevice* device);

int32_t PK_RecvEthBigResponseAsync(sPoKeysDevice* device);

#ifdef __cplusplus
}
#endif

#endif // POKEYSLIBCORESOCKETSASYNC_H
