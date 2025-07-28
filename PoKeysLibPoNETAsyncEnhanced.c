#include "PoKeysLibHal.h"
#include "PoKeysLibAsync.h"
#include <string.h>

/*
 * Enhanced PoNET Async Functions for kbd48CNC and advanced PoNET operations.
 * These complement the existing basic PoNET async functions with:
 * - Enhanced kbd48CNC operations for LinuxCNC integration
 * - Combined operations for efficiency
 * - Advanced device management
 * - RT-safe error handling
 *
 * Designed specifically for production LinuxCNC applications with
 * kbd48CNC devices and other PoNET modules.
 */

// Enhanced kbd48CNC complete update cycle - combines all operations
int PK_PoNETKbd48CNCUpdateCycleAsync(sPoKeysDevice* device)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    
    // This function combines:
    // 1. Module settings check
    // 2. Light sensor reading  
    // 3. Button status reading (statusIn)
    // 4. LED status update (statusOut)
    // 5. PWM brightness update
    
    int result;
    
    // Step 1: Get module settings to verify kbd48CNC is available
    result = PK_PoNETGetModuleSettingsAsync(device);
    if (result != PK_OK) return result;
    
    // Step 2: Request light sensor reading for brightness control
    result = PK_PoNETGetModuleLightRequestAsync(device);
    if (result != PK_OK) return result;
    
    // Step 3: Get light sensor value
    result = PK_PoNETGetModuleLightAsync(device);
    if (result != PK_OK) return result;
    
    // Step 4: Request button status
    result = PK_PoNETGetModuleStatusRequestAsync(device);
    if (result != PK_OK) return result;
    
    // Step 5: Get button status (statusIn for button presses)
    result = PK_PoNETGetModuleStatusAsync(device);
    if (result != PK_OK) return result;
    
    // Step 6: Set LED status (statusOut for LED control)
    result = PK_PoNETSetModuleStatusAsync(device);
    if (result != PK_OK) return result;
    
    // Step 7: Update PWM brightness if needed
    result = PK_PoNETSetModulePWMAsync(device);
    if (result != PK_OK) return result;
    
    return PK_OK;
}

// Set LED pattern for kbd48CNC - optimized for common patterns
int PK_PoNETKbd48CNCLEDSetPatternAsync(sPoKeysDevice* device, uint8_t pattern[6])
{
    if (!device || !pattern) return PK_ERR_GENERIC;
    
    // Copy pattern to device statusOut (48 buttons = 6 bytes)
    memcpy(device->PoNETmodule.statusOut, pattern, 6);
    
    // Send the LED pattern update
    return PK_PoNETSetModuleStatusAsync(device);
}

// Set kbd48CNC brightness with automatic PWM calculation
int PK_PoNETKbd48CNCBrightnessSetAsync(sPoKeysDevice* device, uint8_t brightness)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    
    // Convert brightness (0-255) to PWM duty cycle
    // kbd48CNC uses inverted brightness (255 = bright, 0 = dim)
    device->PoNETmodule.PWMduty = 255 - brightness;
    
    return PK_PoNETSetModulePWMAsync(device);
}

// Enhanced device discovery for PoNET modules
int PK_PoNETDeviceDiscoveryAsync(sPoKeysDevice* device)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    
    uint8_t params[1] = { PONET_OP_DEVICE_DISCOVERY };
    int req = CreateRequestAsync(device, PK_CMD_POI2C_COMMUNICATION,
                                 params, 1, NULL, 0, NULL);
    if (req < 0) return req;
    return SendRequestAsync(device, req);
}

// Reinitialize specific PoNET module (useful for error recovery)
int PK_PoNETModuleReinitializeAsync(sPoKeysDevice* device, uint8_t moduleID)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    
    uint8_t params[2] = { PONET_OP_REINITIALIZE, moduleID };
    int req = CreateRequestAsync(device, PK_CMD_POI2C_COMMUNICATION,
                                 params, 2, NULL, 0, NULL);
    if (req < 0) return req;
    return SendRequestAsync(device, req);
}

// Advanced kbd48CNC button mapping function
static void PK_PoNETKbd48CNCMapButtonID(int logicalButton, int* physicalID)
{
    /*
     * kbd48CNC uses a 4-group mapping scheme for buttons:
     * This matches the mapping used in PoKeysCompPoNet.c
     * 
     * Physical layout is remapped to logical button indices 0-47
     * for easier LinuxCNC integration.
     */
    int offset[] = { 15, 8, 7, 0 };
    int top = (logicalButton & 0xF0) + offset[(logicalButton / 4) % 4];
    int y = logicalButton % 4;
    int ID = top + y;
    
    // Handle even/odd row inversion
    if (((logicalButton / 4) % 2) == 0) {
        ID = top - y;
    }
    
    *physicalID = ID;
}

// Get specific kbd48CNC button state (helper function)
int PK_PoNETKbd48CNCGetButtonStateAsync(sPoKeysDevice* device, uint8_t buttonIndex, bool* pressed)
{
    if (!device || !pressed || buttonIndex >= 48) return PK_ERR_GENERIC;
    
    // Map logical button to physical ID
    int physicalID;
    PK_PoNETKbd48CNCMapButtonID(buttonIndex, &physicalID);
    
    // First ensure we have current status
    int result = PK_PoNETGetModuleStatusAsync(device);
    if (result != PK_OK) return result;
    
    // Extract button state from statusIn
    uint8_t byteIndex = physicalID / 8;
    uint8_t bitIndex = physicalID % 8;
    
    *pressed = (device->PoNETmodule.statusIn[byteIndex] & (1 << bitIndex)) != 0;
    
    return PK_OK;
}

// Set specific kbd48CNC LED state (helper function)
int PK_PoNETKbd48CNCSetLEDStateAsync(sPoKeysDevice* device, uint8_t ledIndex, bool on)
{
    if (!device || ledIndex >= 48) return PK_ERR_GENERIC;
    
    // Map logical LED to physical ID
    int physicalID;
    PK_PoNETKbd48CNCMapButtonID(ledIndex, &physicalID);
    
    // Update statusOut
    uint8_t byteIndex = physicalID / 8;
    uint8_t bitIndex = physicalID % 8;
    
    if (on) {
        device->PoNETmodule.statusOut[byteIndex] |= (1 << bitIndex);
    } else {
        device->PoNETmodule.statusOut[byteIndex] &= ~(1 << bitIndex);
    }
    
    // Send the update
    return PK_PoNETSetModuleStatusAsync(device);
}

// Batch update multiple kbd48CNC LEDs efficiently
int PK_PoNETKbd48CNCSetMultipleLEDsAsync(sPoKeysDevice* device, const uint8_t* ledStates)
{
    if (!device || !ledStates) return PK_ERR_GENERIC;
    
    // Clear all statusOut first
    memset(device->PoNETmodule.statusOut, 0, 16);
    
    // Set LEDs according to ledStates array (48 bits = 6 bytes)
    for (int i = 0; i < 48; i++) {
        if (ledStates[i / 8] & (1 << (i % 8))) {
            int physicalID;
            PK_PoNETKbd48CNCMapButtonID(i, &physicalID);
            
            uint8_t byteIndex = physicalID / 8;
            uint8_t bitIndex = physicalID % 8;
            device->PoNETmodule.statusOut[byteIndex] |= (1 << bitIndex);
        }
    }
    
    // Send the batch update
    return PK_PoNETSetModuleStatusAsync(device);
}
