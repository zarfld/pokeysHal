// Returns a transaction ID, tracks discovery state via callback
int PK_SearchNetworkDevicesAsync(sPoKeysNetworkDeviceSummary *devices, uint32_t timeout_us);

// Connects using an async transaction (non-blocking connect)
int PK_ConnectToNetworkDeviceAsync(sPoKeysNetworkDeviceSummary *device);
