/**
 * @file hal_canon.h
 * @brief Canonical Device Interface definitions and export helpers for LinuxCNC HAL.
 *
 * This header provides data structures and function declarations for implementing
 * canonical HAL interfaces for digital I/O, analog I/O, and encoders as defined by
 * the LinuxCNC Canonical Device Interface (CDI) specification.
 *
 * It includes:
 * - Struct definitions for each canonical device type (digin, digout, adcin, adcout, encoder)
 * - Export functions to define HAL pins and parameters
 * - Registration functions to link periodic HAL functions (read/write)
 *
 * Each canonical device follows a consistent naming convention:
 * `<prefix>.<interface>.<index>.<pin|parameter|function>`
 *
 * This header is intended to be included in HAL component C files that
 * implement device-specific logic and want to expose a CDI-compliant interface.
 *
 * @see https://linuxcnc.org/docs/html/hal/canonical-devices.html
 *
 * @author zarfld
 */

#ifndef LINUXCNC_HAL_CANON_H
#define LINUXCNC_HAL_CANON_H

#include <hal.h>
#include <rtapi.h>
#include <rtapi_string.h>

#ifdef __cplusplus
extern "C" {
#endif


#if !defined(RTAPI) && !defined(ULAPI)
#define ULAPI
#endif
#if defined(RTAPI) && defined(ULAPI)
#undef RTAPI
#endif
/**
 * @defgroup hal_digital Canonical Digital I/O
 * @brief Export and registration helpers for canonical digital input/output.
 * @{
 */

/**
 * @defgroup hal_digin Canonical Digital Input
 * @brief Canonical HAL interface for binary hardware inputs.
 *
 * The canonical digital input (`digin`) provides a standardized interface
 * for reading binary values from hardware. Drivers implementing this interface
 * must provide the following pins:
 *
 * - `in` (BIT):  state of the hardware input
 * - `in-not` (BIT): inverse of the input state
 *
 * No HAL parameters are defined.
 *
 * The optional `read` function should update both pins from hardware input.
 *
 * @see https://linuxcnc.org/docs/html/hal/canonical-devices.html#_digital_input
 */
typedef struct {
    hal_bit_t *in;       // Pin: state of hardware input
    hal_bit_t *in_not;   // Pin: inverted state of input
} hal_digin_t;

/**
 * @brief Export canonical digin pins & parameter.
 */
int hal_export_digin(hal_digin_t *digin, const char *prefix, int index, int comp_id);

/**
 * @brief Register canonical digin read function.
 */
#ifdef RTAPI
int hal_register_digin_func(const char *prefix, int index,
                            void (*read_func)(void *, long),
                            void *inst, int comp_id);
#endif


/**
 * @brief Canonical digital output interface struct.
 * @ingroup hal_digital
 *
 * This structure represents a single canonical digital output channel
 * as defined in the LinuxCNC HAL canonical device interface specification.
 *
 * ### Pins
 * - `out` (BIT, HAL_OUT): The output value to be written to hardware.
 *
 * ### Parameters
 * - `invert` (BIT, HAL_RW): If TRUE, the `out` value is inverted before being sent to hardware.
 *
 * ### Behavior
 * The optional `write` function (registered via `hal_register_digout_func`) should read the
 * `out` and `invert` fields and apply the resulting value to the real hardware.
 *
 * @see https://linuxcnc.org/docs/html/hal/canonical-devices.html#_digital_output
 */
typedef struct {
    hal_bit_t *out;       // value to be written
    hal_bit_t invert;    // Parameter: if TRUE, out is inverted before hardware write
} hal_digout_t;

/**
 * @brief Export canonical digout pins and parameters.
 */
int hal_export_digout(hal_digout_t *digout, const char *prefix, int index, int comp_id);

/**
 * @brief Register canonical digout write function.
 */
#ifdef RTAPI
int hal_register_digout_func(const char *prefix, int index,
                             void (*write_func)(void *, long),
                             void *inst, int comp_id);
#endif

/** @} */ // end of hal_digital

/**
 * @defgroup hal_analog Canonical Analog I/O
 * @brief Canonical interface helpers for analog input and output devices.
 *
 * This module provides HAL-compatible data structures and functions
 * for exporting canonical analog inputs (`adcin`) and outputs (`adcout`)
 * as defined by the LinuxCNC Canonical Device Interface (CDI).
 *
 * ### Analog Input (`adcin`)
 * - Provides a scaled floating-point representation of analog hardware input.
 * - Canonical pins:
 *   - `value` (HAL_OUT, float): scaled analog input value
 * - Canonical parameters:
 *   - `scale`, `offset`, `bit-weight`, `hw-offset` (HAL_RW, float)
 * - Canonical function:
 *   - `read` – updates `value` from hardware, applying scale and offset
 *
 * ### Analog Output (`adcout`)
 * - Represents a continuous analog output signal, e.g. for DAC or PWM hardware.
 * - Canonical pins:
 *   - `value` (HAL_IN, float): value to write to hardware
 *   - `enable` (HAL_IN, bit): when false, output is forced to 0
 * - Canonical parameters:
 *   - `offset`, `scale`, `high-limit`, `low-limit`, `bit-weight`, `hw-offset` (HAL_RW, float)
 * - Canonical function:
 *   - `write` – reads value and parameters, writes scaled output to hardware
 *
 * See also: https://linuxcnc.org/docs/html/hal/canonical-devices.html
 */


/**
 * @brief Canonical analog input (adcin) structure for HAL.
 * @ingroup hal_analog
 *
 * This struct represents a single analog input channel according to the
 * Canonical Device Interface (CDI) specification in LinuxCNC.
 *
 * It provides a scaled and offset floating-point representation of a hardware
 * analog input value, suitable for integration via ADC or other converters.
 *
 * ### Pin
 * - `value` (HAL_OUT, float): Scaled result of the hardware input
 *
 * ### Parameters
 * - `scale` (float): Multiplier applied to the raw input
 * - `offset` (float): Value subtracted after scaling
 * - `bit_weight` (float): Resolution of one LSB (least significant bit)
 * - `hw_offset` (float): Voltage or current present when 0V is applied
 *
 * The canonical read function should acquire the raw hardware input and
 * apply the following formula:
 *
 * ```
 * value = (input * scale) - offset
 * ```
 *
 * @see https://linuxcnc.org/docs/html/hal/canonical-devices.html#sec:hal-cdi:analog-in
 */
typedef struct {
    hal_float_t *value;      // Pin: scaled and offset analog value
    hal_float_t scale;      // Parameter: multiplier
    hal_float_t offset;     // Parameter: subtraction after scaling
    hal_float_t bit_weight; // Parameter: value of one LSB
    hal_float_t hw_offset;  // Parameter: value present when 0V applied
} hal_adcin_t;

/**
 * @brief Export canonical adcin pins and parameters.
 */
int hal_export_adcin(hal_adcin_t *adcin, const char *prefix, int index, int comp_id);

#ifdef RTAPI
/**
 * @brief Register periodic read function for adcin.
 */
int hal_register_adcin_func(const char *prefix, int index,
                            void (*read_func)(void *, long),
                            void *inst, int comp_id);
#endif

/**
 * @brief Canonical analog output (adcout) structure for HAL.
 * @ingroup hal_analog
 *
 * This struct represents a single analog output channel following the
 * Canonical Device Interface (CDI) for analog signals in LinuxCNC.
 *
 * The HAL function registered via `hal_register_adcout_func()` will read the
 * `value` pin and apply all relevant output parameters before writing to the hardware.
 * If `enable` is false, the output is forced to 0 regardless of the value.
 *
 * ### Pins
 * - `value` (HAL_IN, float): Desired analog output value (unprocessed)
 * - `enable` (HAL_IN, bit): If false, hardware output is 0
 *
 * ### Parameters
 * - `offset` (float): Added to the scaled output
 * - `scale` (float): Multiplier applied to the value before offset
 * - `high_limit` (float): Optional maximum clamp
 * - `low_limit` (float): Optional minimum clamp
 * - `bit_weight` (float): Resolution of one LSB (in volts or mA)
 * - `hw_offset` (float): Physical output when value = 0 (e.g., 4 mA or 0V)
 *
 * The final hardware output is typically computed as:
 * ```
 * output = clamp( (scale * value) + offset )
 * ```
 * If `enable == false`, the output is set to 0.
 *
 * @see https://linuxcnc.org/docs/html/hal/canonical-devices.html#sec:hal-cdi:analog-out
 */
typedef struct {
    hal_float_t *value;      // Pin: input to hardware (after scale/offset)
    hal_bit_t   *enable;     // Pin: if false, output 0
    hal_float_t offset;     // Parameter: added to value before hardware write
    hal_float_t scale;      // Parameter: scale applied to value
    hal_float_t high_limit; // Parameter: optional upper clamp
    hal_float_t low_limit;  // Parameter: optional lower clamp
    hal_float_t bit_weight; // Parameter: LSB value
    hal_float_t hw_offset;  // Parameter: physical output when 0 written
} hal_adcout_t;

/**
 * @brief Export canonical adcout pins and parameters.
 */
int hal_export_adcout(hal_adcout_t *adcout, const char *prefix, int index, int comp_id);

#ifdef RTAPI
/**
 * @brief Register periodic write function for adcout.
 */
int hal_register_adcout_func(const char *prefix, int index,
                             void (*write_func)(void *, long),
                             void *inst, int comp_id);
#endif

/** @} */ // end of hal_analog

/**
 * @defgroup hal_encoder Canonical Encoder Interface
 * @brief Canonical HAL interface helpers for incremental encoders.
 *
 * This group provides export and registration functions for encoder devices
 * following the LinuxCNC Canonical Device Interface (CDI).
 *
 * @{
 */
/**
 * @brief Canonical encoder interface structure for HAL.
 * @ingroup hal_encoder
 *
 * This struct represents a single encoder channel based on the Canonical Device Interface (CDI).
 * It provides count, position, and velocity outputs as well as index handling and scale configuration.
 *
 * ### Pins
 * - `count` (HAL_OUT, s32): Raw encoder count
 * - `position` (HAL_OUT, float): Scaled position value
 * - `velocity` (HAL_OUT, float): Estimated velocity in position units per second
 * - `reset` (HAL_IO, bit): Set to TRUE to reset the count
 * - `index-enable` (HAL_IO, bit): Bidirectional pin for index-based reset handshake
 *
 * ### Parameters
 * - `scale` (float): Counts-to-position scale factor
 * - `max_index_vel` (float): Max velocity (pos-units/sec) for valid index reset (+/- 1 count)
 * - `velocity_resolution` (float): Resolution of `velocity` output (pos-units/sec)
 *
 * @see https://linuxcnc.org/docs/html/hal/canonical-devices.html#sec:hal-cdi:encoder
 */
typedef struct {
    hal_s32_t    *count;               // Pin: raw encoder count
    hal_float_t  *position;            // Pin: scaled position
    hal_float_t  *velocity;            // Pin: velocity in pos-units/s
    hal_bit_t    *reset;               // Pin: force reset counter to 0
    hal_bit_t    *index_enable;        // Pin: handshake index reset
    hal_float_t  scale;               // Parameter: counts per position unit
    hal_float_t  max_index_vel;       // optional Parameter: max velocity for index reset
    hal_float_t  velocity_resolution; // optional Parameter: quantization step of velocity
} hal_encoder_t;

/**
 * @brief Export canonical encoder HAL pins and parameters.
 */
int hal_export_encoder(hal_encoder_t *enc, const char *prefix, int index, int comp_id);




#ifdef RTAPI
/**
 * @brief Register a periodic HAL read function for an encoder.
 */
int hal_register_encoder_func(const char *prefix, int index,
                              void (*read_func)(void *, long),
                              void *inst, int comp_id);
#endif

/** @} */ // end of hal_encoder
#ifdef __cplusplus
}
#endif

#endif // LINUXCNC_HAL_CANON_H
