/**
 * @file hal_analog.c
 * @brief HAL export and function registration for canonical analog input and output.
 *
 * This file provides helper functions to export HAL pins and parameters
 * for analog input (`adcin`) and output (`adcout`) channels in compliance
 * with the Canonical Device Interface (CDI) specification.
 *
 * Functions provided:
 * - `hal_export_adcin()` – exports `value` pin and parameters for analog inputs
 * - `hal_register_adcin_func()` – registers a periodic `read` function
 * - `hal_export_adcout()` – exports `value`, `enable`, and parameters for analog outputs
 * - `hal_register_adcout_func()` – registers a `write` function to apply scaled output
 *
 * Pin naming follows the canonical format:
 * `<prefix>.adcin.<index>.<pin>` and `<prefix>.adcout.<index>.<pin>`
 *
 * @see https://linuxcnc.org/docs/html/hal/canonical-devices.html#sec:hal-cdi:analog-in
 * @see https://linuxcnc.org/docs/html/hal/canonical-devices.html#sec:hal-cdi:analog-out
 */
#include <hal.h>
#include <rtapi.h>
#include <rtapi_string.h>
#include "hal_canon.h" 

#if !defined(RTAPI) && !defined(ULAPI)
#define ULAPI
#endif
#if defined(RTAPI) && defined(ULAPI)
#undef RTAPI
#endif

/**
 * @brief Export HAL pins and parameters for a canonical analog input channel.
 * @ingroup hal_analog
 *
 * This function creates all HAL entities required for a single analog input (`adcin`)
 * channel as defined in the Canonical Device Interface (CDI).
 *
 * The following names will be created (formatted as `<prefix>.adcin.<index>.<field>`):
 * - `value` (HAL_OUT, float): the scaled analog input value
 * - `scale` (HAL_RW, float): multiplier applied to the raw hardware value
 * - `offset` (HAL_RW, float): added after scaling
 * - `bit-weight` (HAL_RW, float): granularity of input resolution (LSB size)
 * - `hw-offset` (HAL_RW, float): value present at the input for 0V (calibration)
 *
 * @param adcin     Pointer to the analog input struct to be populated.
 * @param prefix    Canonical name prefix, e.g. "adc.0" or "device.2".
 * @param index     Channel number (0-based) within the device.
 * @param comp_id   HAL component ID as returned by hal_init().
 *
 * @return 0 on success, or a negative HAL/RTAPI error code on failure.
 *
 * @see https://linuxcnc.org/docs/html/hal/canonical-devices.html#sec:hal-cdi:analog-in
 */
int hal_export_adcin(hal_adcin_t *adcin, const char *prefix, int index, int comp_id) {
    int ret;
    char buf[HAL_NAME_LEN];

    // value (output)
    rtapi_snprintf(buf, sizeof(buf), "%s.adcin.%d.value", prefix, index);
    ret = hal_pin_float_newf(HAL_OUT, &adcin->value, comp_id, buf);
    if (ret != 0) return ret;

    // scale
    rtapi_snprintf(buf, sizeof(buf), "%s.adcin.%d.scale", prefix, index);
    ret = hal_param_float_newf(HAL_RW, &adcin->scale, comp_id, buf);
    if (ret != 0) return ret;

    // offset
    rtapi_snprintf(buf, sizeof(buf), "%s.adcin.%d.offset", prefix, index);
    ret = hal_param_float_newf(HAL_RW, &adcin->offset, comp_id, buf);
    if (ret != 0) return ret;

    // bit_weight
    rtapi_snprintf(buf, sizeof(buf), "%s.adcin.%d.bit-weight", prefix, index);
    ret = hal_param_float_newf(HAL_RW, &adcin->bit_weight, comp_id, buf);
    if (ret != 0) return ret;

    // hw_offset
    rtapi_snprintf(buf, sizeof(buf), "%s.adcin.%d.hw-offset", prefix, index);
    ret = hal_param_float_newf(HAL_RW, &adcin->hw_offset, comp_id, buf);
    if (ret != 0) return ret;

    return 0;
}

/**
 * @brief Register a periodic HAL read function for a canonical analog input.
 * @ingroup hal_analog
 *
 * This function registers a HAL function named `<prefix>.adcin.<index>.read`
 * which should read a raw hardware analog value and update the scaled `value` pin.
 * 
 * The user-defined `read_func` is called cyclically by the HAL scheduler and
 * is responsible for applying the scaling logic (scale, offset, bit-weight, hw-offset)
 * and writing the result to `adcin->value`.
 *
 * The function must follow the HAL function signature:
 * `void read_func(void *inst, long period_nsec);`
 *
 * @param prefix      Canonical device prefix, e.g. "adc.0".
 * @param index       Channel number (e.g. 0 for the first input).
 * @param read_func   User-defined function to call every cycle.
 * @param inst        Pointer passed to the function (e.g. `hal_adcin_t*` or user context).
 * @param comp_id     HAL component ID from `hal_init()`.
 *
 * @return 0 on success, or a negative HAL/RTAPI error code on failure.
 *
 * @note Floating-point math is enabled (`uses_fp = 1`).
 * @see https://linuxcnc.org/docs/html/hal/canonical-devices.html#sec:hal-cdi:analog-in
 */
int hal_register_adcin_func(const char *prefix, int index,
                            void (*read_func)(void *, long),
                            void *inst, int comp_id) {
    if (!read_func) return 0;

    char buf[HAL_NAME_LEN];
    rtapi_snprintf(buf, sizeof(buf), "%s.adcin.%d.read", prefix, index);

    return hal_export_funct(buf, read_func, inst, 1, 0, comp_id);
}

/**
 * @brief Export HAL pins and parameters for a canonical analog output channel.
 * @ingroup hal_analog
 *
 * This function creates the HAL pins and parameters needed to implement a single
 * canonical analog output (`adcout`) as specified in the LinuxCNC Canonical Device Interface.
 *
 * The following HAL names are generated, using the format `<prefix>.adcout.<index>.<field>`:
 *
 * ### Pins
 * - `value` (HAL_IN, float): input signal that defines the analog output level
 * - `enable` (HAL_IN, bit): when false, output is forced to 0
 *
 * ### Parameters
 * - `offset` (HAL_RW, float): added to `value` before hardware write
 * - `scale` (HAL_RW, float): multiplier applied before write
 * - `high-limit` (HAL_RW, float): upper clamp value (optional)
 * - `low-limit` (HAL_RW, float): lower clamp value (optional)
 * - `bit-weight` (HAL_RW, float): LSB resolution of the DAC (optional)
 * - `hw-offset` (HAL_RW, float): physical value produced when output is 0 (e.g., 4 mA)
 *
 * @param adcout    Pointer to the analog output struct to populate.
 * @param prefix    Canonical name prefix, e.g. "dac.0" or "analog.1".
 * @param index     Output channel number (0-based).
 * @param comp_id   HAL component ID as returned by `hal_init()`.
 *
 * @return 0 on success, or a negative HAL/RTAPI error code on failure.
 *
 * @see https://linuxcnc.org/docs/html/hal/canonical-devices.html#sec:hal-cdi:analog-out
 */
int hal_export_adcout(hal_adcout_t *adcout, const char *prefix, int index, int comp_id) {
    int ret;
    char buf[HAL_NAME_LEN];

    // value
    rtapi_snprintf(buf, sizeof(buf), "%s.adcout.%d.value", prefix, index);
    ret = hal_pin_float_newf(HAL_IN, &adcout->value, comp_id, buf);
    if (ret != 0) return ret;

    // enable
    rtapi_snprintf(buf, sizeof(buf), "%s.adcout.%d.enable", prefix, index);
    ret = hal_pin_bit_newf(HAL_IN, &adcout->enable, comp_id, buf);
    if (ret != 0) return ret;

    // offset
    rtapi_snprintf(buf, sizeof(buf), "%s.adcout.%d.offset", prefix, index);
    ret = hal_param_float_newf(HAL_RW, &adcout->offset, comp_id, buf);
    if (ret != 0) return ret;

    // scale
    rtapi_snprintf(buf, sizeof(buf), "%s.adcout.%d.scale", prefix, index);
    ret = hal_param_float_newf(HAL_RW, &adcout->scale, comp_id, buf);
    if (ret != 0) return ret;

    // high_limit
    rtapi_snprintf(buf, sizeof(buf), "%s.adcout.%d.high-limit", prefix, index);
    ret = hal_param_float_newf(HAL_RW, &adcout->high_limit, comp_id, buf);
    if (ret != 0) return ret;

    // low_limit
    rtapi_snprintf(buf, sizeof(buf), "%s.adcout.%d.low-limit", prefix, index);
    ret = hal_param_float_newf(HAL_RW, &adcout->low_limit, comp_id, buf);
    if (ret != 0) return ret;

    // bit_weight
    rtapi_snprintf(buf, sizeof(buf), "%s.adcout.%d.bit-weight", prefix, index);
    ret = hal_param_float_newf(HAL_RW, &adcout->bit_weight, comp_id, buf);
    if (ret != 0) return ret;

    // hw_offset
    rtapi_snprintf(buf, sizeof(buf), "%s.adcout.%d.hw-offset", prefix, index);
    ret = hal_param_float_newf(HAL_RW, &adcout->hw_offset, comp_id, buf);
    if (ret != 0) return ret;

    return 0;
}

#ifdef RTAPI
/**
 * @brief Register a HAL write function for a canonical analog output channel.
 * @ingroup hal_analog
 *
 * Registers a periodic HAL function named `<prefix>.adcout.<index>.write` that
 * should read the HAL pin `value`, apply the defined parameters
 * (e.g. `scale`, `offset`, `limit`), and write the final result to hardware.
 *
 * The user-defined function must have the signature:
 * `void write_func(void *inst, long period_nsec);`
 *
 * This function will be called automatically by the HAL scheduler in the context of the
 * real-time thread (typically base-thread or servo-thread). The argument `inst` is a user-defined
 * context, typically a pointer to the `hal_adcout_t` structure or a device-specific context.
 *
 * @param prefix       Canonical device prefix, e.g. "dac.0".
 * @param index        Output channel index within the device.
 * @param write_func   Pointer to the user-defined function to register.
 * @param inst         Pointer passed to the function (usually `hal_adcout_t*` or similar).
 * @param comp_id      HAL component ID returned by `hal_init()`.
 *
 * @return 0 on success, or a negative HAL/RTAPI error code on failure.
 *
 * @note Floating-point operations are enabled (`uses_fp = 1`).
 * @see https://linuxcnc.org/docs/html/hal/canonical-devices.html#sec:hal-cdi:analog-out
 */
int hal_register_adcout_func(const char *prefix, int index,
                             void (*write_func)(void *, long),
                             void *inst, int comp_id) {
    if (!write_func) return 0;

    char buf[HAL_NAME_LEN];
    rtapi_snprintf(buf, sizeof(buf), "%s.adcout.%d.write", prefix, index);

    return hal_export_funct(buf, write_func, inst, 1, 0, comp_id);
}
#endif