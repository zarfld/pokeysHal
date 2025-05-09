#ifndef POKEYSLIBCORESOCKETSASYNC_H
#define POKEYSLIBCORESOCKETSASYNC_H
#include "PoKeysLibHal.h"


#ifdef __cplusplus
extern "C" {
#endif

    #define PK_ERR_SOCKET    (-201)
    #define PK_ERR_TIMEOUT   (-202)
    #define PK_ERR_AGAIN     (-203)
    #define PK_OK_FOUND      (201)

    // Global (or device-specific) discovery context
    typedef struct {
        int txSocket;
        uint64_t start_time_us;
        uint32_t timeout_us;
        sPoKeysNetworkDeviceSummary* devices;
        int nrOfDetectedBoards;
        uint32_t serialNumberToFind;
    } PoKeysDiscoveryContext;

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
