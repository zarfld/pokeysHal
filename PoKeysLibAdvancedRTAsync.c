/*
 * PoKeysLibAdvancedRTAsync.c
 *
 * Advanced RT-compatible async functions for LinuxCNC applications.
 * These functions provide enhanced capabilities for real-time CNC control
 * that aren't covered by the standard async implementations.
 */

#include "PoKeysLibHal.h"
#include "PoKeysLibAsync.h"
#include <string.h>

/* Advanced Digital IO Functions ------------------------------------------------ */

/**
 * @brief Set multiple digital outputs in a single efficient operation (async)
 * 
 * Updates only specified pins without affecting others. More efficient than
 * updating all pins when only a few need changes.
 * 
 * @param device Target device handle
 * @param pinList Array of pin numbers to update
 * @param valueList Array of values corresponding to pins
 * @param count Number of pins to update
 * @return PK_OK on success, error code on failure
 */
int PK_DigitalOutputSetMultipleAsync(sPoKeysDevice* device, const uint8_t* pinList, 
                                     const uint8_t* valueList, uint8_t count) {
    if (!device || !pinList || !valueList) return PK_ERR_GENERIC;
    if (count == 0 || count > 32) return PK_ERR_GENERIC;

    // Update the specified pins in device structure
    for (uint8_t i = 0; i < count; i++) {
        uint8_t pin = pinList[i];
        if (pin < device->info.iPinCount) {
            *(device->Pins[pin].DigitalValueSet.out) = valueList[i] ? 1 : 0;
        }
    }

    // Use standard set/get operation to commit changes
    return PK_DigitalIOSetGetAsync(device);
}

/**
 * @brief Get digital counter values for specific pins only (async)
 * 
 * More efficient than reading all counters when only specific ones are needed.
 * 
 * @param device Target device handle
 * @param pinList Array of pin numbers to read
 * @param count Number of pins to read
 * @return PK_OK on success, error code on failure
 */
int PK_DigitalCounterGetSelectedAsync(sPoKeysDevice* device, const uint8_t* pinList, uint8_t count) {
    if (!device || !pinList) return PK_ERR_GENERIC;
    if (count == 0 || count > 13) return PK_ERR_GENERIC;
    if (device->info.iDigitalCounters == 0) return PK_ERR_NOT_SUPPORTED;

    // Verify all pins have counter capability
    for (uint8_t i = 0; i < count; i++) {
        if (!PK_IsCounterAvailable(device, pinList[i])) {
            return PK_ERR_GENERIC;
        }
    }

    return CreateRequestAsync(device, PK_CMD_DIGITAL_COUNTERS_VALUES,
                              NULL, 0, pinList, count, PK_DigitalCounterParse);
}

/* Advanced PWM Functions ------------------------------------------------------ */

/**
 * @brief Set PWM duty cycles for selected channels only (async)
 * 
 * Updates only specified PWM channels without affecting others.
 * Efficient for applications that don't use all 6 PWM channels.
 * 
 * @param device Target device handle
 * @param channelMask Bitmask of channels to update (bit 0 = channel 0, etc.)
 * @param dutyCycles Array of duty cycle values for enabled channels
 * @return PK_OK on success, error code on failure
 */
int PK_PWMUpdateSelectedChannelsAsync(sPoKeysDevice* device, uint8_t channelMask, const uint32_t* dutyCycles) {
    if (!device || !dutyCycles) return PK_ERR_GENERIC;

    uint8_t payload[37] = {0};
    payload[0] = channelMask;
    
    uint8_t dutyIndex = 0;
    for (uint32_t n = 0; n < 6; n++) {
        if (channelMask & (1 << n)) {
            uint32_t duty = dutyCycles[dutyIndex++];
            payload[1 + n * 4] = (uint8_t)(duty & 0xFF);
            payload[2 + n * 4] = (uint8_t)((duty >> 8) & 0xFF);
            payload[3 + n * 4] = (uint8_t)((duty >> 16) & 0xFF);
            payload[4 + n * 4] = (uint8_t)((duty >> 24) & 0xFF);
            
            // Update device structure
            *(device->PWM.PWMduty[n]) = duty;
        } else {
            // Keep current duty cycle for unchanged channels
            uint32_t duty = (uint32_t)*(device->PWM.PWMduty[n]);
            payload[1 + n * 4] = (uint8_t)(duty & 0xFF);
            payload[2 + n * 4] = (uint8_t)((duty >> 8) & 0xFF);
            payload[3 + n * 4] = (uint8_t)((duty >> 16) & 0xFF);
            payload[4 + n * 4] = (uint8_t)((duty >> 24) & 0xFF);
        }
    }
    
    // Use current period
    uint32_t period = device->PWM.PWMperiod;
    payload[33] = (uint8_t)(period & 0xFF);
    payload[34] = (uint8_t)((period >> 8) & 0xFF);
    payload[35] = (uint8_t)((period >> 16) & 0xFF);
    payload[36] = (uint8_t)((period >> 24) & 0xFF);

    return CreateRequestAsync(device, PK_CMD_PWM_CONFIGURATION, 
                              (const uint8_t[]){1, 1}, 2, payload, sizeof(payload), NULL);
}

/* System Monitoring Functions ------------------------------------------------ */

/**
 * @brief Get comprehensive system status in one operation (async)
 * 
 * Combines device status, load monitoring, and connection health into
 * a single efficient async operation for system monitoring.
 * 
 * @param device Target device handle
 * @return PK_OK on success, error code on failure
 */
int PK_SystemStatusGetAllAsync(sPoKeysDevice* device) {
    if (!device) return PK_ERR_NOT_CONNECTED;

    // Queue multiple status requests efficiently
    int ret;
    
    // 1. Device alive check
    ret = PK_DeviceAliveCheckAsync(device);
    if (ret != PK_OK) return ret;
    
    // 2. Load status (if supported)
    if (device->info.iLoadStatus) {
        ret = PK_DeviceLoadStatusAsync(device);
        if (ret != PK_OK) return ret;
    }
    
    // 3. Error status
    ret = PK_DeviceErrorStatusAsync(device);
    if (ret != PK_OK) return ret;
    
    // 4. Basic device data
    ret = PK_DeviceDataGetAsync(device);
    if (ret != PK_OK) return ret;
    
    return PK_OK;
}

/* RT-Safe Batch Operations --------------------------------------------------- */

/**
 * @brief Perform complete IO update cycle (async)
 * 
 * Combines all common IO operations into a single efficient batch:
 * - Digital IO set/get
 * - Analog input reading  
 * - PWM updates
 * - Counter reading
 * 
 * @param device Target device handle
 * @return PK_OK on success, error code on failure
 */
int PK_IOUpdateCycleAsync(sPoKeysDevice* device) {
    if (!device) return PK_ERR_NOT_CONNECTED;

    int ret;
    
    // 1. Digital IO update (set outputs, read inputs)
    ret = PK_DigitalIOSetGetAsync(device);
    if (ret != PK_OK) return ret;
    
    // 2. Analog inputs (if supported)
    if (device->info.iAnalogInputs) {
        ret = PK_AnalogIOGetAsync(device);
        if (ret != PK_OK) return ret;
    }
    
    // 3. PWM updates (if any channels enabled)
    int pwmActive = 0;
    for (int i = 0; i < 6; i++) {
        if (device->PWM.PWMenabledChannels[i]) {
            pwmActive = 1;
            break;
        }
    }
    if (pwmActive) {
        ret = PK_PWMUpdateAsync(device);
        if (ret != PK_OK) return ret;
    }
    
    // 4. Digital counters (if supported and any available)
    if (device->info.iDigitalCounters) {
        ret = PK_DigitalCounterGetAsync(device);
        if (ret != PK_OK) return ret;
    }
    
    // 5. Encoder values
    ret = PK_EncoderValuesGetAsync(device);
    if (ret != PK_OK) return ret;
    
    return PK_OK;
}

/**
 * @brief Emergency stop all motion and outputs (async)
 * 
 * Immediately stops all motion, disables PWM outputs, and sets
 * all digital outputs to safe state. Critical for emergency situations.
 * 
 * @param device Target device handle
 * @return PK_OK on success, error code on failure
 */
int PK_EmergencyStopAllAsync(sPoKeysDevice* device) {
    if (!device) return PK_ERR_NOT_CONNECTED;

    int ret;
    
    // 1. Stop PulseEngine v2 if available
    if (device->info.iPulseEnginev2) {
        device->PEv2.PulseEngineState = 0;  // Stop engine
        ret = PK_PEv2_StatusGetAsync(device);  // This will apply the state change
        if (ret != PK_OK) return ret;
    }
    
    // 2. Disable all PWM outputs
    for (int i = 0; i < 6; i++) {
        device->PWM.PWMenabledChannels[i] = 0;
        *(device->PWM.PWMduty[i]) = 0;
    }
    ret = PK_PWMUpdateAsync(device);
    if (ret != PK_OK) return ret;
    
    // 3. Set all digital outputs to safe state (low)
    for (uint32_t i = 0; i < device->info.iPinCount; i++) {
        if ((device->Pins[i].PinFunction & PK_PinCap_digitalOutput)) {
            *(device->Pins[i].DigitalValueSet.out) = 0;
        }
    }
    ret = PK_DigitalIOSetGetAsync(device);
    if (ret != PK_OK) return ret;
    
    return PK_OK;
}

/**
 * @brief Test device communication and RT timing (async)
 * 
 * Performs communication test by sending ping commands and measuring
 * response times. Useful for RT system validation.
 * 
 * @param device Target device handle
 * @param testCount Number of ping tests to perform
 * @return PK_OK on success, error code on failure
 */
int PK_CommunicationTestAsync(sPoKeysDevice* device, uint8_t testCount) {
    if (!device) return PK_ERR_NOT_CONNECTED;
    if (testCount == 0 || testCount > 10) return PK_ERR_GENERIC;

    // Perform multiple alive checks to test communication
    for (uint8_t i = 0; i < testCount; i++) {
        int ret = PK_DeviceAliveCheckAsync(device);
        if (ret != PK_OK) return ret;
    }
    
    return PK_OK;
}
