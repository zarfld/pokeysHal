/**
 * @file hal_digital.c
 * @brief Canonical HAL export functions for digital input and output devices.
 *
 * This file provides helper functions to export HAL pins and parameters
 * conforming to the LinuxCNC Canonical Device Interface specification.
 * 
 * Supported interfaces:
 * - Digital Input (digin)
 * - Digital Output (digout)
 *
 * All exported pins follow the canonical naming convention:
 * `<device>.<device-num>.<io-type>.<specific-name>`
 *
 * See: https://linuxcnc.org/docs/html/hal/canonical-devices.html
 *
 * @author
 * @copyright
 * MIT / GPLv2 – choose license as appropriate
 */
#include <hal.h>
#include <rtapi.h>
#include <rtapi_string.h>
#include "hal_canon.h"

/**
 * @defgroup hal_digital Canonical Digital I/O Export
 * @brief Export helpers for canonical digital input/output HAL interfaces.
 *
 * This group contains functions to export standard HAL pins and parameters
 * for digital input and output interfaces, as defined by the LinuxCNC HAL canonical device model.
 *
 * @{
 */

/**
 * @brief Export canonical digital input pins for a specific device channel.
 *
 * This function creates the HAL pins required by the canonical digital input interface.
 * The generated pin names follow the format:
 * `<prefix>.digin.<index>.in` and `<prefix>.digin.<index>.in-not`
 *
 * The <prefix> typically includes the device name and device number (e.g. "gpio.0").
 * The <index> corresponds to the input channel number within the device.
 *
 * @param digin     Pointer to the digital input struct instance.
 * @param prefix    Device-specific prefix, usually of the form "<device-name>.<device-num>".
 * @param index     Channel number (e.g. 0 for the first input of the device).
 * @param comp_id   HAL component ID, as returned by hal_init().
 *
 * @return 0 on success, or a negative error code on failure.
 *
 * @note This function only creates HAL pins. It does not register any HAL function.
 *
 * @see https://linuxcnc.org/docs/html/hal/canonical-devices.html
 */
int hal_export_digin(hal_digin_t *digin, const char *prefix, int index, int comp_id) {
    int ret;
    char buf[HAL_NAME_LEN];

    // Export 'in' pin
    rtapi_snprintf(buf, sizeof(buf), "%s.digin.%d.in", prefix, index);
    ret = hal_pin_bit_newf(HAL_IN, &(digin->in), comp_id, buf);
    if (ret != 0){
        rtapi_print_msg(RTAPI_MSG_ERR, "linuxcnc-hal-canon: %s:%s: %s.digin.%d.in failed\n", __FILE__, __FUNCTION__, prefix, index);
        return ret;
    }
		
    // Export 'in-not' pin
    rtapi_snprintf(buf, sizeof(buf), "%s.digin.%d.in-not", prefix, index);
    ret = hal_pin_bit_newf(HAL_OUT, &(digin->in_not), comp_id, buf);
    if (ret != 0) {
        rtapi_print_msg(RTAPI_MSG_ERR, "linuxcnc-hal-canon: %s:%s: %s.digin.%d.in-not failed\n", __FILE__, __FUNCTION__, prefix, index);
        return ret;
    }

    return 0;
}

/**
 * @brief Register a HAL read function for a canonical digital input instance.
 * @ingroup hal_digital
 *
 * Registers a HAL function named `<prefix>.digin.<index>.read`, which
 * is intended to read the physical input and update HAL pins `in` and `in-not`.
 *
 * @param prefix     Canonical device prefix (e.g. "gpio.0")
 * @param index      Channel index (e.g. 0)
 * @param read_func  Function to call each cycle, must match signature: void (*)(void*, long)
 * @param inst       Pointer passed to the function (e.g. context or digin struct)
 * @param comp_id    HAL component ID from hal_init()
 * @return 0 on success, or a negative HAL/RTAPI error code.
 *
 * @note The function must be non-NULL. Floating-point is enabled by default.
 */
int hal_register_digin_func(const char *prefix,
                            int index,
                            void (*read_func)(void *, long),
                            void *inst,
                            int comp_id)
{
    if (!read_func) return 0;

    char buf[HAL_NAME_LEN];
    rtapi_snprintf(buf, sizeof(buf), "%s.digin.%d.read", prefix, index);

    return hal_export_funct(buf, read_func, inst, 1, 0, comp_id);
}


/**
 * @brief Export canonical digital output pin and parameter for a specific device channel.
 *
 * This function creates the HAL output pin and the associated "invert" parameter
 * as defined by the canonical digital output interface.
 * The generated names follow this format:
 * `<prefix>.digout.<index>.out` and `<prefix>.digout.<index>.invert`
 *
 * The <prefix> usually includes both the device name and its instance number (e.g. "relay.1").
 * The <index> refers to the specific output channel.
 *
 * @param digout    Pointer to the digital output struct instance.
 * @param prefix    Device prefix (e.g. "relay.0", "gpio.2", etc.).
 * @param index     Output channel number for the device.
 * @param comp_id   HAL component ID from hal_init().
 *
 * @return 0 on success, or a negative HAL/RTAPI error code on failure.
 *
 * @note No HAL function is registered. Functional logic must be registered separately.
 *
 * @see https://linuxcnc.org/docs/html/hal/canonical-devices.html
 */
int hal_export_digout(hal_digout_t *digout, const char *prefix, int index, int comp_id) {
    int ret;
    char buf[HAL_NAME_LEN];

    // Export 'out' pin
    rtapi_snprintf(buf, sizeof(buf), "%s.digout.%d.out", prefix, index);
    ret = hal_pin_bit_newf(HAL_OUT, &(digout->out), comp_id, buf);
    if (ret != 0) {
        rtapi_print_msg(RTAPI_MSG_ERR, "linuxcnc-hal-canon: %s:%s: %s.digout.%d.out failed\n", __FILE__, __FUNCTION__, prefix, index);
        return ret;
    }

    // Export 'invert' parameter
    rtapi_snprintf(buf, sizeof(buf), "%s.digout.%d.invert", prefix, index);
    ret = hal_param_bit_newf(HAL_RW, &(digout->invert), comp_id, buf);
    if (ret != 0) {
        rtapi_print_msg(RTAPI_MSG_ERR, "linuxcnc-hal-canon: %s:%s: %s.digout.%d.invert failed\n", __FILE__, __FUNCTION__, prefix, index);
        return ret;
    }

    return 0;
}

#ifdef RTAPI
/**
 * @brief Register a HAL write function for a digout instance.
 * @ingroup hal_digital
 *
 * @param prefix      Canonical device prefix (e.g. "gpio.0")
 * @param index       Channel index (e.g. 0)
 * @param write_func  Function to call each cycle (must match void (*)(void*, long))
 * @param inst        Argument passed to write_func (usually your struct or context)
 * @param comp_id     HAL component ID
 * @return 0 on success or negative error code
 */
int hal_register_digout_func(const char *prefix,
                              int index,
                              void (*write_func)(void *, long),
                              void *inst,
                              int comp_id)
{
    if (!write_func) return 0;

    char buf[HAL_NAME_LEN];
    rtapi_snprintf(buf, sizeof(buf), "%s.digout.%d.write", prefix, index);

    return hal_export_funct(buf, write_func, inst, 1, 0, comp_id);
}
#endif
/** @} */ // end of hal_digital
