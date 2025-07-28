/*
 * PoKeysLibDeviceStatusAsync.c
 *
 * Asynchronous device status and connection monitoring functions for
 * LinuxCNC HAL integration. These functions provide critical monitoring
 * capabilities for real-time applications.
 */

#include "PoKeysLibHal.h"
#include "PoKeysLibAsync.h"
#include <string.h>
#include <sys/time.h>

/* Helper function to get millisecond timestamp */
static uint32_t PK_GetTimeMs(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint32_t)(tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

/* Helper function to handle connection failures */
static void PK_HandleConnectionFailure(sPoKeysDevice *dev)
{
    if (!dev) return;
    
    dev->connectionStatus.connectionAlive = 0;
    dev->connectionStatus.connectionState = 0;
    dev->connectionStatus.consecutiveFailures++;
    
    // Cap consecutive failures to prevent overflow
    if (dev->connectionStatus.consecutiveFailures > 10) {
        dev->connectionStatus.consecutiveFailures = 10;
    }
}

/* Response parsers ------------------------------------------------------- */

static int PK_Parse_DeviceAlive(sPoKeysDevice *dev, const uint8_t *resp)
{
    if (!dev || !resp) return PK_ERR_GENERIC;
    
    // Simple ping response indicates device is alive
    dev->connectionStatus.lastAliveTime = PK_GetTimeMs();
    dev->connectionStatus.connectionState = 1;  // Device is responsive
    dev->connectionStatus.connectionAlive = 1;
    dev->connectionStatus.consecutiveFailures = 0;
    
    return PK_OK;
}

static int PK_Parse_LoadStatus(sPoKeysDevice *dev, const uint8_t *resp)
{
    if (!dev || !resp) return PK_ERR_GENERIC;
    
    dev->deviceLoadStatus.CPUload = resp[8];
    dev->deviceLoadStatus.USBload = resp[9];
    dev->deviceLoadStatus.NetworkLoad = resp[10];
    dev->deviceLoadStatus.Temperature = (int16_t)(resp[11] | (resp[12] << 8));
    dev->deviceLoadStatus.bufferLoad = (uint32_t)(resp[13] | (resp[14] << 8) | (resp[15] << 16) | (resp[16] << 24));
    
    return PK_OK;
}

static int PK_Parse_ErrorStatus(sPoKeysDevice *dev, const uint8_t *resp)
{
    if (!dev || !resp) return PK_ERR_GENERIC;
    
    dev->deviceErrorStatus.errorFlags = (uint32_t)(resp[8] | (resp[9] << 8) | (resp[10] << 16) | (resp[11] << 24));
    dev->deviceErrorStatus.communicationErrors = (uint16_t)(resp[12] | (resp[13] << 8));
    dev->deviceErrorStatus.lastError = resp[14];
    dev->deviceErrorStatus.errorCount = (uint16_t)(resp[15] | (resp[16] << 8));
    
    return PK_OK;
}

/* Public asynchronous functions ----------------------------------------- */

/**
 * @brief Check if device is alive/responsive (async)
 * 
 * Sends a simple ping command to verify device connectivity.
 * Critical for LinuxCNC HAL to detect communication failures.
 * 
 * @param device Target device handle
 * @return PK_OK on success, error code on failure
 */
int PK_DeviceAliveCheckAsync(sPoKeysDevice* device)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    
    // Use basic device data read as ping
    return CreateRequestAsync(device, PK_CMD_READ_DEVICE_DATA,
                             NULL, 0, NULL, 0,
                             PK_Parse_DeviceAlive);
}

/**
 * @brief Get device load status (async)
 * 
 * Retrieves CPU load, USB load, network load, temperature and buffer load.
 * Important for monitoring device performance in real-time applications.
 * 
 * @param device Target device handle
 * @return PK_OK on success, error code on failure
 */
int PK_DeviceLoadStatusAsync(sPoKeysDevice* device)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    
    if (!device->info.iLoadStatus) 
        return PK_ERR_NOT_SUPPORTED;
    
    return CreateRequestAsync(device, PK_CMD_DEVICE_LOAD_STATUS,
                             NULL, 0, NULL, 0,
                             PK_Parse_LoadStatus);
}

/**
 * @brief Get device error status (async)
 * 
 * Retrieves error flags, communication error count, and last error code.
 * Essential for diagnostic information in LinuxCNC applications.
 * 
 * @param device Target device handle
 * @return PK_OK on success, error code on failure
 */
int PK_DeviceErrorStatusAsync(sPoKeysDevice* device)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    
    return CreateRequestAsync(device, PK_CMD_DEVICE_ERROR_STATUS,
                             NULL, 0, NULL, 0,
                             PK_Parse_ErrorStatus);
}

/**
 * @brief Reset device error counters (async)
 * 
 * Clears accumulated error counters and flags.
 * Useful for recovering from transient communication issues.
 * 
 * @param device Target device handle
 * @return PK_OK on success, error code on failure
 */
int PK_DeviceErrorResetAsync(sPoKeysDevice* device)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    
    static const uint8_t params[] = {0x01}; // Reset command
    
    return CreateRequestAsync(device, PK_CMD_DEVICE_ERROR_STATUS,
                             params, 1, NULL, 0,
                             NULL);
}

/**
 * @brief Comprehensive device status check (async)
 * 
 * Combines alive check, load status, and error status into a single
 * efficient monitoring function. Ideal for periodic HAL monitoring.
 * 
 * @param device Target device handle
 * @return PK_OK on success, error code on failure
 */
int PK_DeviceStatusFullAsync(sPoKeysDevice* device)
{
    int ret;
    
    if (!device) return PK_ERR_NOT_CONNECTED;
    
    // 1. Check if device is alive
    ret = PK_DeviceAliveCheckAsync(device);
    if (ret != PK_OK) return ret;
    
    // 2. Get load status (if supported)
    if (device->info.iLoadStatus) {
        ret = PK_DeviceLoadStatusAsync(device);
        if (ret != PK_OK) return ret;
    }
    
    // 3. Get error status
    ret = PK_DeviceErrorStatusAsync(device);
    if (ret != PK_OK) return ret;
    
    return PK_OK;
}

/**
 * @brief Check connection quality metrics (async)
 * 
 * Evaluates connection stability based on response times and error rates.
 * Provides quality metrics for adaptive behavior in LinuxCNC.
 * 
 * @param device Target device handle
 * @param quality Output pointer for quality percentage (0-100)
 * @return PK_OK on success, error code on failure
 */
int PK_DeviceConnectionQualityAsync(sPoKeysDevice* device, uint8_t* quality)
{
    if (!device || !quality) return PK_ERR_GENERIC;
    
    *quality = 0;
    
    // Calculate quality based on recent performance
    uint32_t currentTime = PK_GetTimeMs();
    uint32_t timeSinceLastResponse = currentTime - device->connectionStatus.lastAliveTime;
    
    // Quality decreases with time since last response
    if (timeSinceLastResponse < 100) {
        *quality = 100;
    } else if (timeSinceLastResponse < 500) {
        *quality = 90 - (timeSinceLastResponse - 100) / 10;
    } else if (timeSinceLastResponse < 1000) {
        *quality = 50 - (timeSinceLastResponse - 500) / 20;
    } else {
        *quality = 0;
    }
    
    // Reduce quality based on consecutive failures
    if (device->connectionStatus.consecutiveFailures > 0) {
        *quality = (*quality * (10 - device->connectionStatus.consecutiveFailures)) / 10;
        if (*quality < 0) *quality = 0;
    }
    
    // Store the calculated quality
    device->connectionStatus.connectionQuality = *quality;
    
    return PK_OK;
}

/**
 * @brief Handle communication failure (for retry logic)
 * 
 * Should be called when async communication fails to update
 * connection monitoring counters.
 * 
 * @param device Target device handle
 * @return PK_OK always
 */
int PK_DeviceConnectionFailureAsync(sPoKeysDevice* device)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    
    PK_HandleConnectionFailure(device);
    
    return PK_OK;
}
