/**
 * @file PoKeysLibPoNETHal.c
 * @brief HAL interface functions for PoNET devices (including kbd48CNC)
 *
 * This file provides HAL pin export and management functions for PoNET devices.
 * The kbd48CNC functionality uses the basic PoNET statusIn[] and statusOut[] arrays.
 *
 * ## Features:
 * - Exports PoNET status arrays for direct access
 * - Exports PoNET module control pins
 * - Compatible with kbd48CNC devices via statusIn/statusOut mapping
 * - Follows established pokeys_async naming conventions
 *
 * ## HAL Pin Naming Convention:
 * - Status arrays: `pokeys_async.N.ponet.status-in.K` / `pokeys_async.N.ponet.status-out.K`
 * - Module info: `pokeys_async.N.ponet.module-type` / `pokeys_async.N.ponet.module-id`
 * - Control: `pokeys_async.N.ponet.pwm-duty` / `pokeys_async.N.ponet.light-value`
 * - Status: `pokeys_async.N.ponet.status`
 *
 * ## kbd48CNC Usage:
 * kbd48CNC devices use the statusIn[] array for button states and statusOut[] for LED states.
 * External HAL components can map these arrays to specific kbd48CNC functions.
 *
 * @author zarfld
 */

#include "hal.h"
#include "PoKeysLibHal.h"
#include "PoKeysLibAsync.h"

/**
 * @brief Update PoNET HAL pins from device data.
 *
 * This function copies data between the device PoNET structure and the HAL pins.
 * Called during the update cycle to synchronize HAL with hardware state.
 *
 * @param device Pointer to sPoKeysDevice structure
 */
POKEYSDECL void update_ponet_hal_pins(sPoKeysDevice *device)
{
    if (!device) return;
    
    // Copy statusIn array from device to HAL (hardware to HAL)
    for (int i = 0; i < 16; i++) {
        if (device->PoNETmodule.statusIn_pins[i]) {
            *(device->PoNETmodule.statusIn_pins[i]) = device->PoNETmodule.statusIn[i];
        }
    }
    
    // Copy statusOut array from HAL to device (HAL to hardware)
    for (int i = 0; i < 16; i++) {
        if (device->PoNETmodule.statusOut_pins[i]) {
            device->PoNETmodule.statusOut[i] = (uint8_t)(*(device->PoNETmodule.statusOut_pins[i]));
        }
    }
    
    // Update single-value pins
    if (device->PoNETmodule.PoNETstatus_pin) {
        *(device->PoNETmodule.PoNETstatus_pin) = device->PoNETmodule.PoNETstatus;
    }
    
    if (device->PoNETmodule.moduleType_pin) {
        *(device->PoNETmodule.moduleType_pin) = device->PoNETmodule.moduleType;
    }
    
    if (device->PoNETmodule.moduleID_pin) {
        *(device->PoNETmodule.moduleID_pin) = device->PoNETmodule.moduleID;
    }
    
    if (device->PoNETmodule.lightValue_pin) {
        *(device->PoNETmodule.lightValue_pin) = device->PoNETmodule.lightValue;
    }
    
    if (device->PoNETmodule.PWMduty_pin) {
        device->PoNETmodule.PWMduty = (uint8_t)(*(device->PoNETmodule.PWMduty_pin));
    }
}

/**
 * @brief Export basic PoNET HAL pins for raw access.
 *
 * This function creates HAL pins for basic PoNET communication arrays.
 * The kbd48CNC functionality can be implemented on top of these basic pins
 * using HAL configuration or external mapping components.
 *
 * Each statusIn/statusOut byte becomes a separate u32 pin for HAL compatibility.
 *
 * @param prefix HAL component prefix (e.g., "pokeys_async.0")
 * @param comp_id HAL component ID
 * @param device Pointer to sPoKeysDevice structure
 * @return 0 on success, negative HAL error code on failure
 */
POKEYSDECL int export_ponet_basic_pins(const char *prefix, long comp_id, sPoKeysDevice *device)
{
    int r = 0;
    
    if (!device || !prefix) {
        rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: Invalid parameters\n", __FILE__, __FUNCTION__);
        return -1;
    }
    
    rtapi_print_msg(RTAPI_MSG_INFO, "PoKeys: %s:%s: Exporting basic PoNET pins for %s\n", 
                    __FILE__, __FUNCTION__, prefix);
    
    // Export PoNET status input array (16 bytes) - hardware to HAL
    for (int byte_idx = 0; byte_idx < 16; byte_idx++) {
        r = hal_pin_u32_newf(HAL_OUT, &device->PoNETmodule.statusIn_pins[byte_idx], 
                            comp_id, "%s.ponet.status-in.%02d", prefix, byte_idx);
        if (r != 0) {
            rtapi_print_msg(RTAPI_MSG_ERR, 
                           "PoKeys: %s:%s: Failed to create %s.ponet.status-in.%02d: %d\n", 
                           __FILE__, __FUNCTION__, prefix, byte_idx, r);
            return r;
        }
        rtapi_print_msg(RTAPI_MSG_DBG, "PoKeys: %s:%s: Created %s.ponet.status-in.%02d\n", 
                       __FILE__, __FUNCTION__, prefix, byte_idx);
    }
    
    // Export PoNET status output array (16 bytes) - HAL to hardware
    for (int byte_idx = 0; byte_idx < 16; byte_idx++) {
        r = hal_pin_u32_newf(HAL_IN, &device->PoNETmodule.statusOut_pins[byte_idx], 
                            comp_id, "%s.ponet.status-out.%02d", prefix, byte_idx);
        if (r != 0) {
            rtapi_print_msg(RTAPI_MSG_ERR, 
                           "PoKeys: %s:%s: Failed to create %s.ponet.status-out.%02d: %d\n", 
                           __FILE__, __FUNCTION__, prefix, byte_idx, r);
            return r;
        }
        rtapi_print_msg(RTAPI_MSG_DBG, "PoKeys: %s:%s: Created %s.ponet.status-out.%02d\n", 
                       __FILE__, __FUNCTION__, prefix, byte_idx);
    }
    
    rtapi_print_msg(RTAPI_MSG_INFO, "PoKeys: %s:%s: Successfully exported basic PoNET arrays for %s\n", 
                    __FILE__, __FUNCTION__, prefix);
    return 0;
}

/**
 * @brief Export general PoNET status and control pins.
 *
 * This function exports general PoNET bus status pins that are not device-specific.
 *
 * @param prefix HAL component prefix (e.g., "pokeys_async.0")
 * @param comp_id HAL component ID
 * @param device Pointer to sPoKeysDevice structure
 * @return 0 on success, negative HAL error code on failure
 */
POKEYSDECL int export_ponet_status_pins(const char *prefix, long comp_id, sPoKeysDevice *device)
{
    int r = 0;
    
    if (!device || !prefix) {
        rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: Invalid parameters\n", __FILE__, __FUNCTION__);
        return -1;
    }
    
    rtapi_print_msg(RTAPI_MSG_INFO, "PoKeys: %s:%s: Exporting PoNET status pins for %s\n", 
                    __FILE__, __FUNCTION__, prefix);
    
    // Export PoNET status pin
    r = hal_pin_u32_newf(HAL_OUT, &device->PoNETmodule.PoNETstatus_pin, 
                        comp_id, "%s.ponet.status", prefix);
    if (r != 0) {
        rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: %s.ponet.status failed: %d\n", 
                       __FILE__, __FUNCTION__, prefix, r);
        return r;
    }
    
    // Export PoNET module type pin
    r = hal_pin_u32_newf(HAL_OUT, &device->PoNETmodule.moduleType_pin, 
                        comp_id, "%s.ponet.module-type", prefix);
    if (r != 0) {
        rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: %s.ponet.module-type failed: %d\n", 
                       __FILE__, __FUNCTION__, prefix, r);
        return r;
    }
    
    // Export PoNET module ID pin
    r = hal_pin_u32_newf(HAL_OUT, &device->PoNETmodule.moduleID_pin, 
                        comp_id, "%s.ponet.module-id", prefix);
    if (r != 0) {
        rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: %s.ponet.module-id failed: %d\n", 
                       __FILE__, __FUNCTION__, prefix, r);
        return r;
    }
    
    // Export light sensor value pin - output from hardware to HAL
    r = hal_pin_u32_newf(HAL_OUT, &device->PoNETmodule.lightValue_pin, 
                        comp_id, "%s.ponet.light-value", prefix);
    if (r != 0) {
        rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: %s.ponet.light-value failed: %d\n", 
                       __FILE__, __FUNCTION__, prefix, r);
        return r;
    }
    
    // Export PWM duty control pin - input from HAL to hardware
    r = hal_pin_u32_newf(HAL_IN, &device->PoNETmodule.PWMduty_pin, 
                        comp_id, "%s.ponet.pwm-duty", prefix);
    if (r != 0) {
        rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: %s.ponet.pwm-duty failed: %d\n", 
                       __FILE__, __FUNCTION__, prefix, r);
        return r;
    }
    
    rtapi_print_msg(RTAPI_MSG_INFO, "PoKeys: %s:%s: Successfully exported PoNET status pins for %s\n", 
                    __FILE__, __FUNCTION__, prefix);
    return 0;
}
/**
 * @brief Main export function for all PoNET HAL pins.
 *
 * This is the main entry point for exporting PoNET-related HAL pins.
 * It exports both basic PoNET communication arrays and general status pins.
 *
 * @param prefix HAL component prefix (e.g., "pokeys_async.0")
 * @param comp_id HAL component ID
 * @param device Pointer to sPoKeysDevice structure
 * @return 0 on success, negative HAL error code on failure
 */
POKEYSDECL int export_ponet_pins(const char *prefix, long comp_id, sPoKeysDevice *device)
{
    int r = 0;
    
    if (!device || !prefix) {
        rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: Invalid parameters\n", __FILE__, __FUNCTION__);
        return -1;
    }
    
    rtapi_print_msg(RTAPI_MSG_INFO, "PoKeys: %s:%s: Exporting PoNET pins for %s\n", 
                    __FILE__, __FUNCTION__, prefix);
    
    // Export PoNET status pins
    r = export_ponet_status_pins(prefix, comp_id, device);
    if (r != 0) {
        rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: export_ponet_status_pins failed: %d\n", 
                       __FILE__, __FUNCTION__, r);
        return r;
    }
    
    // Export basic PoNET communication pins
    r = export_ponet_basic_pins(prefix, comp_id, device);
    if (r != 0) {
        rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: export_ponet_basic_pins failed: %d\n", 
                       __FILE__, __FUNCTION__, r);
        return r;
    }
    
    rtapi_print_msg(RTAPI_MSG_INFO, "PoKeys: %s:%s: Successfully exported all PoNET pins for %s\n", 
                    __FILE__, __FUNCTION__, prefix);
    return 0;
}
