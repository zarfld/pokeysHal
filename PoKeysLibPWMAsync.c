/*
 * PoKeysLibPWMAsync.c
 *
 * Asynchronous PWM (Pulse Width Modulation) functions for real-time
 * LinuxCNC HAL integration. PWM is critical for spindle speed control,
 * analog outputs, and servo control in CNC applications.
 */

#include "PoKeysLibHal.h"
#include "PoKeysLibAsync.h"
#include <string.h>

/* Response parsers ------------------------------------------------------- */

static int PK_Parse_PWMConfiguration(sPoKeysDevice *dev, const uint8_t *resp)
{
    if (!dev || !resp) return PK_ERR_GENERIC;
    
    // Parse PWM period (32-bit value)
    dev->PWM.PWMperiod = (uint32_t)resp[8] |
                         ((uint32_t)resp[9] << 8) |
                         ((uint32_t)resp[10] << 16) |
                         ((uint32_t)resp[11] << 24);
    
    // Parse enabled channels (6 channels)
    if (dev->PWM.PWMenabledChannels) {
        for (int i = 0; i < 6; i++) {
            dev->PWM.PWMenabledChannels[i] = resp[12 + i];
        }
    }
    
    // Parse duty cycles (6 channels x 4 bytes each)
    if (dev->PWM.PWMduty) {
        for (int i = 0; i < 6; i++) {
            dev->PWM.PWMduty[i] = (uint32_t)resp[18 + i*4] |
                                  ((uint32_t)resp[19 + i*4] << 8) |
                                  ((uint32_t)resp[20 + i*4] << 16) |
                                  ((uint32_t)resp[21 + i*4] << 24);
        }
    }
    
    return PK_OK;
}

/* Public asynchronous functions ----------------------------------------- */

/**
 * @brief Get PWM configuration (async)
 * 
 * Retrieves current PWM period, enabled channels, and duty cycles.
 * Essential for reading back current PWM state in LinuxCNC.
 * 
 * @param device Target device handle
 * @return PK_OK on success, error code on failure
 */
int PK_PWMConfigurationGetAsync(sPoKeysDevice* device)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    
    if (device->info.iPWMCount == 0) 
        return PK_ERR_NOT_SUPPORTED;
    
    static const uint8_t params[] = {0x00}; // Get configuration
    
    return CreateRequestAsync(device, PK_CMD_PWM_CONFIGURATION,
                             params, 1, NULL, 0,
                             PK_Parse_PWMConfiguration);
}

/**
 * @brief Set PWM configuration (async)
 * 
 * Configures PWM period and enabled channels. This is typically done
 * during initialization and when changing PWM frequency.
 * 
 * @param device Target device handle
 * @return PK_OK on success, error code on failure
 */
int PK_PWMConfigurationSetAsync(sPoKeysDevice* device)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    
    if (device->info.iPWMCount == 0) 
        return PK_ERR_NOT_SUPPORTED;
    
    if (!device->PWM.PWMenabledChannels)
        return PK_ERR_PARAMETER;
    
    uint8_t params[11];
    params[0] = 0x01; // Set configuration command
    
    // Pack PWM period (32-bit)
    params[1] = device->PWM.PWMperiod & 0xFF;
    params[2] = (device->PWM.PWMperiod >> 8) & 0xFF;
    params[3] = (device->PWM.PWMperiod >> 16) & 0xFF;
    params[4] = (device->PWM.PWMperiod >> 24) & 0xFF;
    
    // Pack enabled channels (6 channels)
    for (int i = 0; i < 6; i++) {
        params[5 + i] = device->PWM.PWMenabledChannels[i];
    }
    
    return CreateRequestAsync(device, PK_CMD_PWM_CONFIGURATION,
                             params, 11, NULL, 0,
                             NULL);
}

/**
 * @brief Update PWM duty cycles (async)
 * 
 * Updates duty cycles for all PWM channels without changing period.
 * This is the most frequently used PWM function for real-time control.
 * 
 * @param device Target device handle
 * @return PK_OK on success, error code on failure
 */
int PK_PWMUpdateAsync(sPoKeysDevice* device)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    
    if (device->info.iPWMCount == 0) 
        return PK_ERR_NOT_SUPPORTED;
    
    if (!device->PWM.PWMduty)
        return PK_ERR_PARAMETER;
    
    uint8_t params[25];
    params[0] = 0x02; // Update duty cycles command
    
    // Pack duty cycles (6 channels x 4 bytes each)
    for (int i = 0; i < 6; i++) {
        params[1 + i*4] = device->PWM.PWMduty[i] & 0xFF;
        params[2 + i*4] = (device->PWM.PWMduty[i] >> 8) & 0xFF;
        params[3 + i*4] = (device->PWM.PWMduty[i] >> 16) & 0xFF;
        params[4 + i*4] = (device->PWM.PWMduty[i] >> 24) & 0xFF;
    }
    
    return CreateRequestAsync(device, PK_CMD_PWM_CONFIGURATION,
                             params, 25, NULL, 0,
                             NULL);
}

/**
 * @brief Set PWM configuration directly (async)
 * 
 * Convenience function to set PWM period and enabled channels in one call.
 * Useful for initialization sequences.
 * 
 * @param device Target device handle
 * @param PWMperiod PWM period in clock ticks
 * @param enabledChannels Array of 6 enabled channel flags
 * @return PK_OK on success, error code on failure
 */
int PK_PWMConfigurationSetDirectlyAsync(sPoKeysDevice* device, uint32_t PWMperiod, uint8_t* enabledChannels)
{
    if (!device || !enabledChannels) return PK_ERR_PARAMETER;
    
    if (device->info.iPWMCount == 0) 
        return PK_ERR_NOT_SUPPORTED;
    
    // Update device structure
    device->PWM.PWMperiod = PWMperiod;
    if (device->PWM.PWMenabledChannels) {
        for (int i = 0; i < 6; i++) {
            device->PWM.PWMenabledChannels[i] = enabledChannels[i];
        }
    }
    
    return PK_PWMConfigurationSetAsync(device);
}

/**
 * @brief Update PWM duty cycles directly (async)
 * 
 * Convenience function to update duty cycles from an array.
 * Frequently used in LinuxCNC HAL for real-time PWM updates.
 * 
 * @param device Target device handle
 * @param dutyCycles Array of 6 duty cycle values
 * @return PK_OK on success, error code on failure
 */
int PK_PWMUpdateDirectlyAsync(sPoKeysDevice* device, uint32_t* dutyCycles)
{
    if (!device || !dutyCycles) return PK_ERR_PARAMETER;
    
    if (device->info.iPWMCount == 0) 
        return PK_ERR_NOT_SUPPORTED;
    
    // Update device structure
    if (device->PWM.PWMduty) {
        for (int i = 0; i < 6; i++) {
            device->PWM.PWMduty[i] = dutyCycles[i];
        }
    }
    
    return PK_PWMUpdateAsync(device);
}

/**
 * @brief Set single PWM channel duty cycle (async)
 * 
 * Updates a single PWM channel without affecting others.
 * Efficient for applications that need to update individual channels.
 * 
 * @param device Target device handle
 * @param channel PWM channel number (0-5)
 * @param dutyCycle Duty cycle value
 * @return PK_OK on success, error code on failure
 */
int PK_PWMSetSingleChannelAsync(sPoKeysDevice* device, uint8_t channel, uint32_t dutyCycle)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    
    if (device->info.iPWMCount == 0) 
        return PK_ERR_NOT_SUPPORTED;
    
    if (channel >= 6) return PK_ERR_PARAMETER;
    
    uint8_t params[6];
    params[0] = 0x03; // Set single channel command
    params[1] = channel;
    params[2] = dutyCycle & 0xFF;
    params[3] = (dutyCycle >> 8) & 0xFF;
    params[4] = (dutyCycle >> 16) & 0xFF;
    params[5] = (dutyCycle >> 24) & 0xFF;
    
    // Update device structure
    if (device->PWM.PWMduty) {
        device->PWM.PWMduty[channel] = dutyCycle;
    }
    
    return CreateRequestAsync(device, PK_CMD_PWM_CONFIGURATION,
                             params, 6, NULL, 0,
                             NULL);
}

/**
 * @brief Get PWM pin assignments (async)
 * 
 * Retrieves which physical pins are assigned to PWM channels.
 * Important for LinuxCNC configuration and diagnostics.
 * 
 * @param device Target device handle
 * @return PK_OK on success, error code on failure
 */
int PK_PWMGetPinAssignmentsAsync(sPoKeysDevice* device)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    
    if (device->info.iPWMCount == 0) 
        return PK_ERR_NOT_SUPPORTED;
    
    // Fill PWM pin assignments based on device type
    int ret = PK_FillPWMPinNumbers(device);
    if (ret != PK_OK) return ret;
    
    return PK_OK;
}
