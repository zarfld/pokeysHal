/**
 * @file hal_encoder.c
 * @brief HAL export and function registration helpers for canonical encoder devices.
 *
 * This file provides functions to export HAL pins and parameters and register
 * a periodic read function for encoder channels that conform to the
 * LinuxCNC Canonical Device Interface (CDI).
 *
 * The encoder interface exposes both raw and scaled data to HAL,
 * including position and velocity information, with index-reset logic support.
 *
 * Functions provided:
 * - `hal_export_encoder()` – defines HAL pins and parameters for an encoder
 * - `hal_register_encoder_func()` – registers a periodic HAL function to read the encoder
 *
 * All exported symbols follow the canonical naming format:
 * `<prefix>.encoder.<index>.<pin|param|func>`
 *
 * @see https://linuxcnc.org/docs/html/hal/canonical-devices.html#sec:hal-cdi:encoder
 */
#include <hal.h>
#include <rtapi.h>
#include <rtapi_string.h>
#include "hal_canon.h"


/**
 * @brief Export HAL pins and parameters for a canonical encoder channel.
 * @ingroup hal_encoder
 *
 * Creates all HAL pins and parameters as defined by the Canonical Device Interface (CDI)
 * for encoder devices. The pin names follow the format:
 * `<prefix>.encoder.<index>.<field>`
 *
 * @param enc       Pointer to the encoder struct to populate.
 * @param prefix    Canonical name prefix, e.g. "enc.0" or "hw.1".
 * @param index     Channel number (0-based) within the device.
 * @param comp_id   HAL component ID from `hal_init()`.
 *
 * @return 0 on success, or a negative HAL/RTAPI error code on failure.
 *
 * @see https://linuxcnc.org/docs/html/hal/canonical-devices.html#sec:hal-cdi:encoder
 */
int hal_export_encoder(hal_encoder_t *enc, const char *prefix, int index, int comp_id) {
    int ret;
    char buf[HAL_NAME_LEN];

    // --- Pins ---

    // count
    rtapi_snprintf(buf, sizeof(buf), "%s.encoder.%d.count", prefix, index);
    ret = hal_pin_s32_newf(HAL_OUT, &enc->count, comp_id, buf);
    if (ret != 0) return ret;

    // position
    rtapi_snprintf(buf, sizeof(buf), "%s.encoder.%d.position", prefix, index);
    ret = hal_pin_float_newf(HAL_OUT, &enc->position, comp_id, buf);
    if (ret != 0) return ret;

    // velocity
    rtapi_snprintf(buf, sizeof(buf), "%s.encoder.%d.velocity", prefix, index);
    ret = hal_pin_float_newf(HAL_OUT, &enc->velocity, comp_id, buf);
    if (ret != 0) return ret;

    // reset
    rtapi_snprintf(buf, sizeof(buf), "%s.encoder.%d.reset", prefix, index);
    ret = hal_pin_bit_newf(HAL_IO, &enc->reset, comp_id, buf);
    if (ret != 0) return ret;

    // index-enable
    rtapi_snprintf(buf, sizeof(buf), "%s.encoder.%d.index-enable", prefix, index);
    ret = hal_pin_bit_newf(HAL_IO, &enc->index_enable, comp_id, buf);
    if (ret != 0) return ret;

    // --- Parameters ---

    // scale
    rtapi_snprintf(buf, sizeof(buf), "%s.encoder.%d.scale", prefix, index);
    ret = hal_param_float_newf(HAL_RW, &enc->scale, comp_id, buf);
    if (ret != 0) return ret;

    // max-index-vel
    rtapi_snprintf(buf, sizeof(buf), "%s.encoder.%d.max-index-vel", prefix, index);
    ret = hal_param_float_newf(HAL_RW, &enc->max_index_vel, comp_id, buf);
    if (ret != 0) return ret;

    // velocity-resolution
    rtapi_snprintf(buf, sizeof(buf), "%s.encoder.%d.velocity-resolution", prefix, index);
    ret = hal_param_float_newf(HAL_RW, &enc->velocity_resolution, comp_id, buf);
    if (ret != 0) return ret;

    return 0;
}

#ifdef RTAPI
/**
 * @brief Register periodic read function for a canonical encoder channel.
 * @ingroup hal_encoder
 *
 * Registers a HAL function named `<prefix>.encoder.<index>.read` that is expected to:
 * - Capture the hardware encoder count
 * - Update the canonical `count`, `position`, and `velocity` pins
 *
 * The registered function will be called cyclically from the HAL thread context and must match the signature:
 * `void read_func(void *inst, long period_nsec);`
 *
 * @param prefix      Canonical name prefix (e.g. "enc.0")
 * @param index       Channel number (0-based)
 * @param read_func   Function pointer to the implementation
 * @param inst        Context pointer (typically a pointer to `hal_encoder_t`)
 * @param comp_id     HAL component ID returned by `hal_init()`
 *
 * @return 0 on success, or a negative error code on failure.
 *
 * @note Floating-point math is enabled (`uses_fp = 1`).
 */
int hal_register_encoder_func(const char *prefix, int index,
                              void (*read_func)(void *, long),
                              void *inst, int comp_id) {
    if (!read_func) return 0;

    char buf[HAL_NAME_LEN];
    rtapi_snprintf(buf, sizeof(buf), "%s.encoder.%d.read", prefix, index);

    return hal_export_funct(buf, read_func, inst, 1, 0, comp_id);  // uses_fp = 1
}
#endif
