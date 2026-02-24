/**
 * @file PoKeysLibRTCAsync.c
 * @brief Asynchronous RTC (Real Time Clock) access for PoKeys devices
 *
 * This file provides an asynchronous, non-blocking method to read RTC data
 * from a PoKeys device using the CreateRequestAsync / SendRequestAsync /
 * PK_ReceiveAndDispatch framework.
 *
 * ## Features:
 * - Uses per-transaction response buffers (no global buffers)
 * - Parses RTC fields manually into HAL-safe `hal_u32_t` variables
 * - Exports RTC fields (sec, min, hour, dow, dom, month, doy, year) as HAL output pins
 * - Fully compatible with LinuxCNC Realtime HAL requirements
 *
 * ## Functional Flow:
 * @verbatim
 * [RT Thread]
 *    ↓
 *    PK_RTCGetAsync(device)
 *      ↳ CreateRequestAsync(cmd=0x83, param=0x00, parser=PK_RTCGetAsync_Process)
 *
 * [Send Loop]
 *    ↓
 *    SendRequestAsync() sends packet
 *
 * [Receive Loop]
 *    ↓
 *    PK_ReceiveAndDispatch() receives packet
 *      ↳ Calls PK_RTCGetAsync_Process(device, response)
 *          ↳ Parses RTC fields (SEC, MIN, HOUR, etc.)
 *          ↳ Fills device->RTC fields
 *
 * [HAL Pins]
 *    ↓
 *    HAL pins (rtc_sec, rtc_min, etc.) are now updated automatically.
 * @endverbatim
 *
 * ## Notes:
 * - `target_ptr` is NULL because parsing is handled manually in the parser function.
 * - No redundant rtc_response_buffer is needed — mailbox_entry's response_buffer[64] is used directly.
 */


#include "hal.h" // make sure this is included
#include "PoKeysLibHal.h"
#include "PoKeysLibAsync.h"

/**
 * @brief Exports RTC fields as HAL output pins.
 *
 * @param prefix HAL pin name prefix
 * @param comp_id HAL component ID
 * @param device Pointer to sPoKeysDevice (contains rtc_data field)
 * @return 0 on success, or negative HAL error code
 */
 int export_rtc_pins(const char *prefix, long comp_id, sPoKeysDevice *device)
 {
     int r;
 
     if (device == NULL) {
            rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: device is NULL\n", __FILE__, __FUNCTION__);
            return -1;
     }

     rtapi_print_msg(RTAPI_MSG_DBG, "PoKeys: %s:%s: %%s.rtc.sec\n", __FILE__, __FUNCTION__, prefix);
     r = hal_pin_u32_newf(HAL_OUT, &(device->RTC.SEC), comp_id, "%s.rtc.sec", prefix);
     if (r != 0) {
        rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: %%s.rtc.sec failed\n", __FILE__, __FUNCTION__, prefix);
        return r;
    }
 
     r = hal_pin_u32_newf(HAL_OUT, &(device->RTC.MIN), comp_id, "%s.rtc.min", prefix);
     if (r != 0) return r;
 
     r = hal_pin_u32_newf(HAL_OUT, &(device->RTC.HOUR), comp_id, "%s.rtc.hour", prefix);
     if (r != 0) return r;
 
     r = hal_pin_u32_newf(HAL_OUT, &(device->RTC.DOW), comp_id, "%s.rtc.dow", prefix);
     if (r != 0) return r;
 
     r = hal_pin_u32_newf(HAL_OUT, &(device->RTC.DOM), comp_id, "%s.rtc.dom", prefix);
     if (r != 0) return r;
 
     r = hal_pin_u32_newf(HAL_OUT, &(device->RTC.DOY), comp_id, "%s.rtc.doy", prefix);
     if (r != 0) return r;
 
     r = hal_pin_u32_newf(HAL_OUT, &(device->RTC.MONTH), comp_id, "%s.rtc.month", prefix);
     if (r != 0) return r;
 
     r = hal_pin_u32_newf(HAL_OUT, &(device->RTC.YEAR), comp_id, "%s.rtc.year", prefix);
     if (r != 0) return r;
 
     return 0;
 }

/**
 * @brief Processes the RTC response and fills HAL-safe structure.
 *
 * @param device Pointer to PoKeys device
 * @param response Pointer to full received UDP packet
 * @return PK_OK on success, PK_ERR otherwise
 */
 int PK_RTCGetAsync_Process(sPoKeysDevice* device, const uint8_t *response)
 {
     if (device == NULL || response == NULL)
         return PK_ERR_TRANSFER;

     /* RTAPI_MSG_ERR used intentionally: RTAPI_MSG_INFO/DBG are filtered
      * out by default in CI; ERR ensures these diagnostics are visible
      * without needing special halrun flags. */
     rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: called - parsing RTC response\n",
                     __FILE__, __FUNCTION__);

     /* Defense-in-depth: verify the command byte in the response matches 0x83.
      * PK_ReceiveAndDispatch already does this check before calling us, but if
      * the parser is ever invoked directly (unit tests, future refactors) this
      * guard prevents writing garbage into HAL pins. */
     if (response[1] != (uint8_t)PK_CMD_RTC_SETTINGS) {
         rtapi_print_msg(RTAPI_MSG_ERR,
             "PoKeys: %s:%s: wrong command byte 0x%02X (expected 0x%02X)"
             " - discarding response\n",
             __FILE__, __FUNCTION__,
             (unsigned)response[1], (unsigned)(uint8_t)PK_CMD_RTC_SETTINGS);
         return PK_ERR_TRANSFER;
     }

     /* Read raw values before range-checking. */
     uint8_t  sec   = response[8];
     uint8_t  min   = response[9];
     uint8_t  hour  = response[10];
     uint8_t  dow   = response[11];
     uint8_t  dom   = response[12];
     uint8_t  month = response[13];
     uint16_t doy   = ((uint16_t)response[14]) | (((uint16_t)response[15]) << 8);
     uint16_t year  = ((uint16_t)response[16]) | (((uint16_t)response[17]) << 8);

     rtapi_print_msg(RTAPI_MSG_ERR,
                     "PoKeys: %s:%s: RTC raw: year=%u month=%u dom=%u dow=%u"
                     " hour=%u min=%u sec=%u doy=%u\n",
                     __FILE__, __FUNCTION__,
                     (unsigned)year, (unsigned)month, (unsigned)dom,
                     (unsigned)dow,  (unsigned)hour,  (unsigned)min,
                     (unsigned)sec,  (unsigned)doy);

     /* Range-validate before writing HAL pins.  A command-code mismatch
      * (stale/recycled req_id dispatched to the wrong parser) would produce
      * wildly out-of-range values — month=14, year=3080 etc.  Reject and
      * leave the HAL pins at their last good value rather than overwriting
      * with garbage. */
     if (sec   > 59 || min   > 59 || hour  > 23 ||
         dom   < 1  || dom   > 31 || dow   > 6  ||
         month < 1  || month > 12 ||
         year  < 2000 || year > 2100) {
         rtapi_print_msg(RTAPI_MSG_ERR,
             "PoKeys: %s:%s: RTC values out of range"
             " (year=%u month=%u dom=%u dow=%u hour=%u min=%u sec=%u)"
             " - discarding, keeping previous HAL pin values\n",
             __FILE__, __FUNCTION__,
             (unsigned)year, (unsigned)month, (unsigned)dom, (unsigned)dow,
             (unsigned)hour, (unsigned)min,   (unsigned)sec);
         return PK_ERR_TRANSFER;
     }

     *(device->RTC.SEC)   = sec;
     *(device->RTC.MIN)   = min;
     *(device->RTC.HOUR)  = hour;
     *(device->RTC.DOW)   = dow;
     *(device->RTC.DOM)   = dom;
     *(device->RTC.MONTH) = month;
     *(device->RTC.DOY)   = doy;
     *(device->RTC.YEAR)  = year;

     rtapi_print_msg(RTAPI_MSG_ERR,
                     "PoKeys: %s:%s: RTC parsed OK: year=%u month=%u dom=%u dow=%u"
                     " hour=%u min=%u sec=%u doy=%u\n",
                     __FILE__, __FUNCTION__,
                     (unsigned)year, (unsigned)month, (unsigned)dom,
                     (unsigned)dow,  (unsigned)hour,  (unsigned)min,
                     (unsigned)sec,  (unsigned)doy);

     return PK_OK;
 }
 

/**
 * @brief Starts an asynchronous RTC read from the device.
 *
 * @param device Pointer to PoKeys device
 * @return Request ID on success, negative error code on failure
 */
 int PK_RTCGetAsync(sPoKeysDevice* device)
 {
     if (device == NULL)
         return PK_ERR_NOT_CONNECTED;
 
     // Create request:
     // Command 0x83 (RTC GET), param1 = 0x00
     uint8_t params[1] = { 0x00 }; // Only param1 used
 
     // Important: Target is NOT device->RTC directly, but temporary response buffer
    int req_id = CreateRequestAsync(device, PK_CMD_RTC_SETTINGS, params, 1,
                                        NULL, 0,
                                PK_RTCGetAsync_Process); // Set parser function!
    rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: PK_RTCGetAsync: req_id=%d\n", __FILE__, __FUNCTION__, req_id);

     if (req_id < 0) {
         rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: CreateRequestAsync failed\n", __FILE__, __FUNCTION__);
         return req_id; // Error in request creation
     }
 
     // Send the request
    return SendRequestAsync(device, req_id); // Send the request the first time
 }

 