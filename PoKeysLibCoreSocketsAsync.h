// Global (or device-specific) discovery context
typedef struct {
    int txSocket;
    uint64_t start_time_us;
    uint32_t timeout_us;
    sPoKeysNetworkDeviceSummary* devices;
    int nrOfDetectedBoards;
    uint32_t serialNumberToFind;
} PoKeysDiscoveryContext;

// Returns a transaction ID, tracks discovery state via callback
int PK_SearchNetworkDevicesAsync(sPoKeysNetworkDeviceSummary *devices, uint32_t timeout_us);

// Connects using an async transaction (non-blocking connect)
int PK_ConnectToNetworkDeviceAsync(sPoKeysNetworkDeviceSummary *device);
