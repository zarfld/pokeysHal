#ifndef POKEYSLIB_ASYNC_H
#define POKEYSLIB_ASYNC_H
#include "PoKeysLibHal.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>


#define MAX_TRANSACTIONS 64 // Maximum number of async transactions

typedef enum {
    TRANSACTION_PENDING = 0,
    TRANSACTION_COMPLETED = 1,
    TRANSACTION_TIMEOUT = 2,
    TRANSACTION_FAILED = 3
} transaction_status_t;

typedef enum {
    /* General commands */
    PK_CMD_READ_DEVICE_DATA              = 0x00,
    PK_CMD_RESERVED_01                   = 0x01,
    PK_CMD_SET_USER_ID                   = 0x02,
    PK_CMD_READ_USER_ID_LOCK             = 0x03,
    PK_CMD_READ_BUILD_DATE               = 0x04,
    PK_CMD_GET_SYSTEM_LOAD_STATUS        = 0x05,
    PK_CMD_DEVICE_NAME                   = 0x06,
    PK_CMD_CONFIGURE_USB                 = 0x07,
    PK_CMD_DELAYED_STARTUP_CFG           = 0x08,
    PK_CMD_PPM_DECODER_CFG               = 0x09,
    PK_CMD_SESSION_SETTINGS              = 0x0A,
    PK_CMD_MISC_USB_CONFIGURATION        = 0x0B,

    /* I/O and encoder configuration */
    PK_CMD_IO_SETTINGS_SET               = 0x10,
    PK_CMD_ENCODER_SETTINGS_SET          = 0x11,
    PK_CMD_ENCODER_KEYMAP_A_SET          = 0x12,
    PK_CMD_ENCODER_KEYMAP_B_SET          = 0x13,
    PK_CMD_GET_PIN_FUNCTION              = 0x15,
    PK_CMD_ENCODER_SETTINGS_GET          = 0x16,
    PK_CMD_ENCODER_KEYMAP_A_GET          = 0x17,
    PK_CMD_ENCODER_KEYMAP_B_GET          = 0x18,
    PK_CMD_ENCODER_RAW_VALUE_GET         = 0x19,
    PK_CMD_ENCODER_RAW_VALUE_RESET       = 0x1A,
    PK_CMD_CONNECTION_PIN_STATUS         = 0x1B,
    PK_CMD_ULTRAFAST_ENCODER_ENABLE      = 0x1C,
    PK_CMD_DIGITAL_COUNTERS_RESET        = 0x1D,
    PK_CMD_ADDITIONAL_PIN_SETTINGS       = 0x1E,
    PK_CMD_GET_PIN_CAPABILITIES          = 0x1F,
    PK_CMD_KEY_ASSOCIATION_SET           = 0x20,
    PK_CMD_PIN_TYPEMATIC_DELAY           = 0x21,
    PK_CMD_PIN_REPEAT_RATE               = 0x22,
    PK_CMD_KEY_ASSOCIATION_GET           = 0x25,

    /* Digital and analog I/O */
    PK_CMD_DIGITAL_INPUTS_GET            = 0x30,
    PK_CMD_DIGITAL_INPUTS_BLOCK_I        = 0x31,
    PK_CMD_DIGITAL_INPUTS_BLOCK_II       = 0x32,
    PK_CMD_ANALOG_INPUT_GET              = 0x35,
    PK_CMD_ANALOG_INPUT_BLOCK_4X8        = 0x36,
    PK_CMD_ANALOG_INPUT_BLOCK_3X10       = 0x37,
    PK_CMD_ANALOG_RC_FILTER_GET          = 0x38,
    PK_CMD_ANALOG_RC_FILTER_SET          = 0x39,
    PK_CMD_ANALOG_INPUTS_GET_ALL         = 0x3A,
    PK_CMD_FULL_IO                       = 0x3F,
    PK_CMD_DIGITAL_OUTPUTS_SET           = 0x40,
    PK_CMD_ANALOG_OUTPUTS_SET            = 0x41,
    PK_CMD_DIGITAL_OUTPUT_BLOCK_I_SET    = 0x42,
    PK_CMD_DIGITAL_OUTPUT_BLOCK_II_SET   = 0x43,

    /* Device specific subsystems */
    PK_CMD_MCS_SYSTEM                    = 0x4A,
    PK_CMD_WS2812_CONTROL                = 0x4B,
    PK_CMD_SIMPLE_PULSE_GENERATOR        = 0x4C,

    PK_CMD_CONFIGURATION_SAVE            = 0x50,
    PK_CMD_CONFIGURATION_SAVE_LOCK       = 0x51,
    PK_CMD_CONFIGURATION_RESET_UNLOCK    = 0x52,

    PK_CMD_JOYSTICK_CFG_GET              = 0x60,
    PK_CMD_JOYSTICK_UPBTN_GET            = 0x61,
    PK_CMD_JOYSTICK_CFG_SET              = 0x65,
    PK_CMD_JOYSTICK_UPBTN_SET            = 0x66,
    PK_CMD_JOYSTICK_ANALOG_TO_DIGITAL    = 0x6A,

    PK_CMD_SENSORS_SETUP_56              = 0x70,
    PK_CMD_DASHBOARD_ITEMS_SETUP         = 0x71,
    PK_CMD_WEB_USERS_SETUP               = 0x72,
    PK_CMD_WEB_SETTINGS_SETUP            = 0x73,
    PK_CMD_READ_ALL_SENSORS              = 0x74,
    PK_CMD_SENSORS_SETUP_57              = 0x76,
    PK_CMD_SENSOR_VALUES_READ_57         = 0x77,
    PK_CMD_DASHBOARD_ITEMS_SETUP_57      = 0x78,
    PK_CMD_SIMPLE_SENSOR_CFG             = 0x79,

    PK_CMD_PULSE_ENGINE_DEPRECATED       = 0x80,
    PK_CMD_FAILSAFE_SETTINGS             = 0x81,
    PK_CMD_POIL_COMMANDS                 = 0x82,
    PK_CMD_RTC_SETTINGS                  = 0x83,
    PK_CMD_SYSTEM_LOG_OPS                = 0x84,
    PK_CMD_PULSE_ENGINE_V2               = 0x85,
    PK_CMD_CAN_OPERATIONS                = 0x86,

    PK_CMD_UNLOCK_OPTION                 = 0x8F,
    PK_CMD_MACRO_CREATE                  = 0x90,
    PK_CMD_MACRO_MODIFY                  = 0x91,
    PK_CMD_MACRO_DELETE                  = 0x92,
    PK_CMD_MACRO_SAVE_TO_FLASH           = 0x93,
    PK_CMD_MACRO_RENAME                  = 0x94,
    PK_CMD_MACRO_TRANSFER                = 0x95,
    PK_CMD_MACRO_LENGTH_GET              = 0x96,
    PK_CMD_MACRO_NAME_GET                = 0x97,
    PK_CMD_MACRO_KEYS_GET                = 0x98,
    PK_CMD_MACRO_FREE_SPACE_GET          = 0x99,
    PK_CMD_MACRO_ACTIVE_GET              = 0x9A,

    PK_CMD_RTMODE_SETUP                  = 0xA0,
    PK_CMD_RTMODE_PACKET_IO              = 0xA1,
    PK_CMD_UDP_BINARY_REALTIME_CFG       = 0xA2,

    PK_CMD_MULTIPART_PACKET              = 0xB0,

    PK_CMD_PIN_CONFIGURATION             = 0xC0,
    PK_CMD_PIN_KEY_MAPPING               = 0xC1,
    PK_CMD_PIN_KEY_CODES                 = 0xC2,
    PK_CMD_PIN_KEY_MODIFIERS             = 0xC3,
    PK_CMD_ENCODER_OPTION_SET            = 0xC4,
    PK_CMD_ENCODER_CHANNELS_SET          = 0xC5,
    PK_CMD_ENCODER_KEYMAP_A_SET2         = 0xC6,
    PK_CMD_ENCODER_KEYMAP_B_SET2         = 0xC7,
    PK_CMD_MATRIX_KEYBOARD_CFG           = 0xCA,
    PK_CMD_PWM_CONFIGURATION             = 0xCB,
    PK_CMD_DEVICE_STATUS_GET             = 0xCC,
    PK_CMD_ENCODER_LONG_RAW_VALUES_GET   = 0xCD,
    PK_CMD_FAST_ENCODERS_SET             = 0xCE,
    PK_CMD_TICK_COUNTER_GET              = 0xCF,
    PK_CMD_LCD_CONFIGURATION             = 0xD0,
    PK_CMD_LCD_OPERATION                 = 0xD1,
    PK_CMD_MATRIX_LED_CONFIGURATION      = 0xD5,
    PK_CMD_MATRIX_LED_UPDATE             = 0xD6,
    PK_CMD_TRIGGERED_INPUT_SETTINGS      = 0xD7,
    PK_CMD_DIGITAL_COUNTERS_VALUES       = 0xD8,
    PK_CMD_DIGITAL_COUNTERS_DIRECTION    = 0xD9,
    PK_CMD_AUX_BUS_SETTINGS              = 0xDA,
    PK_CMD_I2C_COMMUNICATION             = 0xDB,
    PK_CMD_ONEWIRE_COMMUNICATION         = 0xDC,
    PK_CMD_POI2C_COMMUNICATION           = 0xDD,
    PK_CMD_UART_COMMUNICATION            = 0xDE,
    PK_CMD_NETWORK_SETTINGS              = 0xE0,
    PK_CMD_SECURITY_STATUS_GET           = 0xE1,
    PK_CMD_USER_AUTHORISE                = 0xE2,
    PK_CMD_USER_PASSWORD_SET             = 0xE3,
    PK_CMD_MODBUS_SETTINGS               = 0xE4,
    PK_CMD_SPI_COMMUNICATION             = 0xE5,
    PK_CMD_CAN_NODE_COMMANDS             = 0xE6,
    PK_CMD_COSM_SETTINGS                 = 0xEF,
    PK_CMD_REBOOT_SYSTEM                 = 0xF3,

    /* Bootloader operations */
    PK_CMD_BOOT_CLEAR_MEMORY             = 0xF0,
    PK_CMD_BOOT_BLOCK_OPTIONS            = 0xF1,
    PK_CMD_BOOT_TRANSFER_BLOCK           = 0xF2,
    PK_CMD_BOOT_START_APP                = 0xF3,
    PK_CMD_BOOT_CALC_AND_SAVE_CRC        = 0xF5,
    PK_CMD_BOOT_CLEAR_USER_SETTINGS      = 0xF6,
    PK_CMD_BOOT_CUSTOM_COMMAND1          = 0xFA,
    PK_CMD_BOOT_OEM_COMMAND              = 0xFB,
    PK_CMD_BOOT_OEM_PARAMETERS           = 0xFD
} pokeys_command_t;

// Pulse engine v2 subcommands (used with PK_CMD_PULSE_ENGINE_V2)
typedef enum {
    /* Status and configuration */
    PEV2_CMD_GET_STATUS                  = 0x00,
    PEV2_CMD_SETUP                       = 0x01,
    PEV2_CMD_SET_STATE                   = 0x02,
    PEV2_CMD_SET_AXIS_POSITION           = 0x03,
    PEV2_CMD_SET_OUTPUTS                 = 0x04,
    PEV2_CMD_REBOOT                      = 0x05,
    PEV2_CMD_CONFIGURE_MISC              = 0x06,
    PEV2_CMD_GET_STATUS2                 = 0x08,
    PEV2_CMD_SETUP_SYNCED_PWM            = 0x0A,
    PEV2_CMD_SETUP_SYNCED_DIGITAL        = 0x0B,

    /* Axis operations */
    PEV2_CMD_GET_AXIS_CONFIGURATION      = 0x10,
    PEV2_CMD_SET_AXIS_CONFIGURATION      = 0x11,
    PEV2_CMD_GET_INTERNAL_DRIVERS        = 0x18,
    PEV2_CMD_SET_INTERNAL_DRIVERS        = 0x19,

    /* Motion */
    PEV2_CMD_MOVE                        = 0x20,
    PEV2_CMD_START_HOMING                = 0x21,
    PEV2_CMD_FINISH_HOMING               = 0x22,
    PEV2_CMD_START_PROBING               = 0x23,
    PEV2_CMD_FINISH_PROBING              = 0x24,
    PEV2_CMD_MOVE_PV                     = 0x25,

    /* Threading and trigger */
    PEV2_CMD_PREPARE_TRIGGER             = 0x30,
    PEV2_CMD_FORCE_TRIGGER_READY         = 0x31,
    PEV2_CMD_ARM_TRIGGER                 = 0x32,
    PEV2_CMD_RELEASE_TRIGGER             = 0x33,
    PEV2_CMD_CANCEL_THREADING            = 0x34,
    PEV2_CMD_GET_THREADING_STATUS        = 0x35,
    PEV2_CMD_SET_THREADING_PARAMS        = 0x36,
    PEV2_CMD_GET_ENCODER_TEST_RESULTS    = 0x37,

    /* Backlash compensation */
    PEV2_CMD_GET_BACKLASH_SETTINGS       = 0x40,
    PEV2_CMD_SET_BACKLASH_SETTINGS       = 0x41,

    /* Driver communication */
    PEV2_CMD_SETUP_DRIVER_COMM           = 0x50,
    PEV2_CMD_GET_DRIVER_STATUS           = 0x51,
    PEV2_CMD_DRIVER_CURRENT_PARAMS       = 0x52,
    PEV2_CMD_DRIVER_MODE_PARAMS          = 0x53,
    PEV2_CMD_GET_DRIVER_VERSIONS         = 0x54,

    /* Smart pulse generator */
    PEV2_CMD_SMART_CFG_GET               = 0x90,
    PEV2_CMD_SMART_CFG_SET               = 0x91,
    PEV2_CMD_SMART_COUNTERS_RESET        = 0x92,
    PEV2_CMD_SMART_STATUS                = 0x95,
    PEV2_CMD_SMART_ENCODERS              = 0x96,

    /* Buffer and raw transfer */
    PEV2_CMD_CLEAR_BUFFER                = 0xF0,
    PEV2_CMD_TRANSFER_RAW                = 0xE0,
    PEV2_CMD_FILL_BUFFER_8BIT            = 0xFF,
    PEV2_CMD_OUTPUT_ENABLE               = 0xFF
} PEv2_command_t;

// PoCAN subcommands (used with PK_CMD_CAN_NODE_COMMANDS)
typedef enum {
    POCAN_CMD_STATUS                     = 0x00,
    POCAN_CMD_ENABLE                     = 0x01,
    POCAN_CMD_CONFIGURE                  = 0x02,

    /* CAN bus requests */
    POCAN_CMD_DEVICE_IDENTIFICATION      = 0x10,
    POCAN_CMD_CONFIGURATION_READ         = 0x11,
    POCAN_CMD_CONFIGURATION_WRITE        = 0x12,
    POCAN_CMD_CONFIGURATION_SAVE         = 0x13
} PoCAN_command_t;

// Supported PoCAN device types
typedef enum {
    POCAN_DEVICE_PORELAY8                = 0x01,
    POCAN_DEVICE_POCAN15I                = 0x02,
    POCAN_DEVICE_POCAN_LIN               = 0x03,
    POCAN_DEVICE_POCAN15TRIAC            = 0x04,
    POCAN_DEVICE_KBD48CNC               = 0x05,
    POCAN_DEVICE_POCAN4TRIAC             = 0x06
} PoCAN_DeviceType_T;

// PoNET operations (used with PK_CMD_POI2C_COMMUNICATION)
typedef enum {
    PONET_OP_GET_STATUS                = 0x00,
    PONET_OP_GET_MODULE_SETTINGS       = 0x10,
    PONET_OP_GET_FIRMWARE_VERSION      = 0x11,
    PONET_OP_SET_MODULE_SETTINGS       = 0x15,
    PONET_OP_CLEAR_MODULE_SETTINGS     = 0x20,
    PONET_OP_REINITIALIZE              = 0x21,
    PONET_OP_REINIT_AND_CLEAR          = 0x25,
    PONET_OP_DEVICE_DISCOVERY          = 0x30,
    PONET_OP_CHECK_FOR_DEVICES         = 0x40,
    PONET_OP_GET_MODULE_DATA           = 0x50,
    PONET_OP_SET_MODULE_DATA           = 0x55,
    PONET_OP_GET_LIGHT_SENSOR          = 0x60,
    PONET_OP_SET_PWM_VALUE             = 0x70,
    PONET_OP_START_BOOTLOADER          = 0xF0,
    PONET_OP_START_PROGRAMMING         = 0xF1,
    PONET_OP_TRANSFER_FIRMWARE_PART    = 0xF2,
    PONET_OP_FINISH_FIRMWARE_TRANSFER  = 0xF3,
    PONET_OP_EXIT_BOOTLOADER           = 0xF4,
    PONET_OP_ACTIVATE_BOOTLOADER       = 0xF5
} PoNET_operations_t;

typedef int (*pokeys_response_parser_t)(sPoKeysDevice *dev, const uint8_t *response);

typedef struct {
    uint8_t request_buffer[64];
    uint8_t response_buffer[64];
    uint8_t request_id;
    pokeys_command_t command_sent;

    uint64_t timestamp_sent;
    uint8_t retries_left;

    transaction_status_t status;
    bool response_ready;
    // Optional parser
    int (*response_parser)(sPoKeysDevice *dev, const uint8_t *response);

    void *target_ptr;
    size_t target_size;
} async_transaction_t;

typedef struct {
    uint8_t request_id;
    pokeys_command_t command_sent;
    uint64_t timestamp_sent;
    int retries_left;
    bool response_ready;

    void *target_ptr;
    size_t target_size;
    pokeys_response_parser_t response_parser; // << NEW! Optional per-request parser function

    uint8_t request_buffer[64];
    uint8_t response_buffer[64];
} mailbox_entry_t;

// Function declarations
int CreateRequestAsync(sPoKeysDevice *dev, pokeys_command_t cmd,
    const uint8_t *params, size_t params_len,
    void *target_ptr, size_t target_size,
    int (*parser_func)(sPoKeysDevice *dev, const uint8_t *response));

int CreateRequestAsyncWithPayload(
        sPoKeysDevice *device,
        pokeys_command_t cmd,
        const uint8_t *params,
        size_t params_len,
        const void *payload,
        size_t payload_size,
        pokeys_response_parser_t parser_func
    );

int SendRequestAsync(sPoKeysDevice *dev, uint8_t request_id);

int PK_ReceiveAndDispatch(sPoKeysDevice *dev);

void PK_TimeoutAndRetryCheck(sPoKeysDevice *dev, uint64_t timeout_us);

// PulseEngine v2 Async Functions
int PK_PEv2_StatusGetAsync(sPoKeysDevice *device);
int PK_PEv2_Status2GetAsync(sPoKeysDevice *device);
int PK_PEv2_PulseEngineSetupAsync(sPoKeysDevice *device);
int PK_PEv2_AdditionalParametersGetAsync(sPoKeysDevice *device);
int PK_PEv2_AdditionalParametersSetAsync(sPoKeysDevice *device);
int PK_PEv2_AxisConfigurationGetAsync(sPoKeysDevice *device);
int PK_PEv2_AxisConfigurationSetAsync(sPoKeysDevice *device);
int PK_PEv2_PulseEngineMovePVAsync(sPoKeysDevice *device);
int PK_PEv2_HomingStartAsync(sPoKeysDevice *device);
int PK_PEv2_ExternalOutputsSetAsync(sPoKeysDevice *device);

#endif // POKEYSLIB_ASYNC_H
