#include "PoKeysLibHal.h"
#include "PoKeysLibAsync.h"

typedef enum {
    PK_ENCODERCFG_STEP_NONE,
    PK_ENCODERCFG_STEP_BASIC_OPTIONS,
    PK_ENCODERCFG_STEP_CHANNEL_MAPPING,
    PK_ENCODERCFG_STEP_DIRA_MAPPING,
    PK_ENCODERCFG_STEP_DIRB_MAPPING,
    PK_ENCODERCFG_STEP_FAST_ENCODERS,
    PK_ENCODERCFG_STEP_ULTRA_FAST_ENCODERS,
    PK_ENCODERCFG_STEP_COMPLETE
} PKEncoderCfgStep;

typedef struct {
    PKEncoderCfgStep current_step;
    uint8_t request_id;
} PKEncoderCfgAsyncContext;

static PKEncoderCfgAsyncContext encoderCfgContext = {PK_ENCODERCFG_STEP_NONE, 0};

/**
 * @brief Exports all encoder HAL pins and parameters for a PoKeys device.
 *
 * @param prefix HAL naming prefix
 * @param comp_id HAL component ID
 * @param device Pointer to PoKeys device structure
 * @return 0 on success, negative HAL error code
 */
int export_encoder_pins(const char *prefix, long comp_id, sPoKeysDevice *device)
{
    if (device == NULL)
        return -1;

    int r = 0;

    // Export debounced encoder output value (optional - check if needed)

    // Loop over each basic encoder
    for (int i = 0; i < device->info.iBasicEncoderCount; i++) {
        sPoKeysEncoder *enc = &device->Encoders[i];

        r = hal_pin_s32_newf(HAL_OUT, &(enc->encoderValue), comp_id, "%s.encoder.%d.count", prefix, i);
        if (r != 0) return r;
        r = hal_pin_float_newf(HAL_OUT, &(enc->position), comp_id, "%s.encoder.%d.position", prefix, i);
        if (r != 0) return r;
        r = hal_pin_float_newf(HAL_OUT, &(enc->velocity), comp_id, "%s.encoder.%d.velocity", prefix, i);
        if (r != 0) return r;
        r = hal_pin_bit_newf(HAL_IN, &(enc->reset), comp_id, "%s.encoder.%d.reset", prefix, i);
        if (r != 0) return r;
        r = hal_pin_bit_newf(HAL_IN, &(enc->index_enable), comp_id, "%s.encoder.%d.index-enable", prefix, i);
        if (r != 0) return r;

        // Parameters
        r = hal_param_float_newf(HAL_RW, &(enc->scale), comp_id, "%s.encoder.%d.scale", prefix, i);
        if (r != 0) return r;
        r = hal_param_u32_newf(HAL_RW, &(enc->encoderOptions), comp_id, "%s.encoder.%d.encoderOptions", prefix, i);
        if (r != 0) return r;
        r = hal_param_bit_newf(HAL_RW, &(enc->enable), comp_id, "%s.encoder.%d.enable", prefix, i);
        if (r != 0) return r;
        r = hal_param_bit_newf(HAL_RW, &(enc->x4_sampling), comp_id, "%s.encoder.%d.x4_sampling", prefix, i);
        if (r != 0) return r;
        r = hal_param_bit_newf(HAL_RW, &(enc->x2_sampling), comp_id, "%s.encoder.%d.x2_sampling", prefix, i);
        if (r != 0) return r;

        r = hal_param_u32_newf(HAL_RW, &(enc->channelApin), comp_id, "%s.encoder.%d.channelApin", prefix, i);
        if (r != 0) return r;
        r = hal_param_u32_newf(HAL_RW, &(enc->channelBpin), comp_id, "%s.encoder.%d.channelBpin", prefix, i);
        if (r != 0) return r;
    }

    // Export FastEncoder and UltraFastEncoder options if supported
    if (device->info.iFastEncoders) {
        r = hal_param_u32_newf(HAL_RW, &(device->FastEncodersConfiguration), comp_id, "%s.encoder.FastEncoders.Configuration", prefix);
        if (r != 0) return r;
        r = hal_param_u32_newf(HAL_RW, &(device->FastEncodersOptions), comp_id, "%s.encoder.FastEncoders.Options", prefix);
        if (r != 0) return r;
    }

    if (device->info.iUltraFastEncoders) {
        r = hal_param_u32_newf(HAL_RW, &(device->UltraFastEncoderConfiguration), comp_id, "%s.encoder.UltraFastEncoders.Configuration", prefix);
        if (r != 0) return r;
        r = hal_param_u32_newf(HAL_RW, &(device->UltraFastEncoderOptions), comp_id, "%s.encoder.UltraFastEncoders.Options", prefix);
        if (r != 0) return r;
        r = hal_param_u32_newf(HAL_RW, &(device->UltraFastEncoderFilter), comp_id, "%s.encoder.UltraFastEncoders.Filter", prefix);
        if (r != 0) return r;
    }

    return 0;
}

/**
 * @brief Parses encoderOptions field and fills HAL encoder parameters.
 *
 * @param device Pointer to PoKeys device
 * @param response Full UDP response packet (starts at byte 0)
 * @return PK_OK on success, PK_ERR otherwise
 */
 int PK_EncoderOptionsParse(sPoKeysDevice* device, const uint8_t* response)
 {
     if (device == NULL || response == NULL)
         return PK_ERR_TRANSFER;
 
     uint8_t encoder_index = response[6]; // Request ID echoes encoder index (if you organize it that way)
     if (encoder_index >= device->info.iBasicEncoderCount)
         return PK_ERR_PARAMETER;
 
     sPoKeysEncoder* enc = &device->Encoders[encoder_index];
 
     uint8_t options = response[8]; // Raw options byte from device
 
     enc->encoderOptions = options; // Save full 8-bit value into hal_u32_t
 
     enc->enable = (options & (1 << 0)) ? 1 : 0;
     enc->x4_sampling = (options & (1 << 1)) ? 1 : 0;
     enc->x2_sampling = (options & (1 << 2)) ? 1 : 0;
     enc->keymap_dirA = (options & (1 << 4)) ? 1 : 0;
     enc->macro_dirA  = (options & (1 << 5)) ? 1 : 0;
     enc->keymap_dirB = (options & (1 << 6)) ? 1 : 0;
     enc->macro_dirB  = (options & (1 << 7)) ? 1 : 0;
 
     return PK_OK;
 }

 
/**
 * @brief Starts the asynchronous encoder configuration retrieval.
 *
 * This function issues multiple CreateRequestAsync() calls in sequence,
 * preparing the system for automatic response filling based on target_ptr.
 *
 * @param device Pointer to the PoKeys device
 * @return PK_OK if successful, PK_ERR otherwise
 */
int PK_EncoderConfigurationGetAsync(sPoKeysDevice* device)
{
    if (device == NULL) return PK_ERR_NOT_CONNECTED;
    if (!device->info.iBasicEncoderCount) return PK_ERR_NOT_SUPPORTED;

    // Step 1: Basic encoder options (0xC4)
    for (uint32_t i = 0; i < device->info.iBasicEncoderCount; i++) {
        CreateRequestAsync(device, 0xC4, NULL, 0,
            &device->Encoders[i].encoderOptions, sizeof(device->Encoders[i].encoderOptions),NULL);
    }

    // Step 2: Channel mappings (0xC5)
    for (uint32_t i = 0; i < device->info.iBasicEncoderCount; i++) {
        // Channel A pin
        CreateRequestAsync(device, 0xC5, NULL, 0,
            &device->Encoders[i].channelApin, sizeof(device->Encoders[i].channelApin));

        // Channel B pin
        CreateRequestAsync(device, 0xC5, NULL, 0,
            &device->Encoders[i].channelBpin, sizeof(device->Encoders[i].channelBpin),NULL);
    }

    if (device->info.iKeyMapping)
    {
        // Step 3: Direction A key mapping (0xC6)
        for (uint32_t i = 0; i < device->info.iBasicEncoderCount; i++) {
            CreateRequestAsync(device, 0xC6, NULL, 0,
                &device->Encoders[i].dirAkeyCode, sizeof(device->Encoders[i].dirAkeyCode),NULL);
            CreateRequestAsync(device, 0xC6, NULL, 0,
                &device->Encoders[i].dirAkeyModifier, sizeof(device->Encoders[i].dirAkeyModifier),NULL);
        }

        // Step 4: Direction B key mapping (0xC7)
        for (uint32_t i = 0; i < device->info.iBasicEncoderCount; i++) {
            CreateRequestAsync(device, 0xC7, NULL, 0,
                &device->Encoders[i].dirBkeyCode, sizeof(device->Encoders[i].dirBkeyCode),NULL);
            CreateRequestAsync(device, 0xC7, NULL, 0,
                &device->Encoders[i].dirBkeyModifier, sizeof(device->Encoders[i].dirBkeyModifier),NULL);
        }
    }

    if (device->info.iFastEncoders)
    {
        uint8_t params_fast[] = {2}; // As seen in original 0xCE, param=2
        CreateRequestAsync(device, 0xCE, params_fast, 1,
            &device->FastEncodersConfiguration, sizeof(device->FastEncodersConfiguration),NULL);
        CreateRequestAsync(device, 0xCE, params_fast, 1,
            &device->FastEncodersOptions, sizeof(device->FastEncodersOptions),NULL);
    }

    if (device->info.iUltraFastEncoders)
    {
        uint8_t params_ultra[] = {0xFF}; // 0x1C, param=0xFF
        CreateRequestAsync(device, 0x1C, params_ultra, 1,
            &device->UltraFastEncoderConfiguration, sizeof(device->UltraFastEncoderConfiguration),NULL);
        CreateRequestAsync(device, 0x1C, params_ultra, 1,
            &device->UltraFastEncoderOptions, sizeof(device->UltraFastEncoderOptions));
        CreateRequestAsync(device, 0x1C, params_ultra, 1,
            &device->UltraFastEncoderFilter, sizeof(device->UltraFastEncoderFilter),NULL);
    }

    return PK_OK;
}

/**
 * @brief Starts the asynchronous encoder configuration write sequence.
 *
 * Fills requests with current device->Encoders[] values, using CreateRequestAsync().
 *
 * @param device Pointer to PoKeys device
 * @return PK_OK on success, PK_ERR otherwise
 */
int PK_EncoderConfigurationSetAsync(sPoKeysDevice *device)
{
    if (device == NULL) return PK_ERR_NOT_CONNECTED;
    if (!device->info.iBasicEncoderCount) return PK_ERR_NOT_SUPPORTED;

    uint8_t params_basic[] = {1}; // 0xC4, 1
    uint8_t params_mapping[] = {1}; // 0xC5, 1
    uint8_t params_dir_mapping[] = {1}; // 0xC6 / 0xC7, 1

    // 1. Set basic encoder options (0xC4, 1)
    uint8_t buffer_basic[64] = {0};
    buffer_basic[0] = 0xBB;
    buffer_basic[1] = 0xC4;
    buffer_basic[2] = 1; // Operation: Write

    for (uint32_t i = 0; i < device->info.iBasicEncoderCount; i++) {
        buffer_basic[8 + i] = device->Encoders[i].encoderOptions;
    }

    CreateRequestAsync(device, 0xC4, params_basic, 1, NULL, 0);
    memcpy(device->request + 8, buffer_basic + 8, device->info.iBasicEncoderCount);

    // 2. Set channel mappings (0xC5, 1)
    uint8_t buffer_mapping[64] = {0};
    buffer_mapping[0] = 0xBB;
    buffer_mapping[1] = 0xC5;
    buffer_mapping[2] = 1; // Operation: Write

    for (uint32_t i = 0; i < device->info.iBasicEncoderCount; i++) {
        buffer_mapping[8 + i] = device->Encoders[i].channelApin;
        buffer_mapping[33 + i] = device->Encoders[i].channelBpin;
    }

    CreateRequestAsync(device, 0xC5, params_mapping, 1, NULL, 0,NULL);
    memcpy(device->request + 8, buffer_mapping + 8, device->info.iBasicEncoderCount);
    memcpy(device->request + 33, buffer_mapping + 33, device->info.iBasicEncoderCount);

    if (device->info.iKeyMapping)
    {
        // 3. Direction A key mapping (0xC6)
        uint8_t buffer_dirA[64] = {0};
        buffer_dirA[0] = 0xBB;
        buffer_dirA[1] = 0xC6;
        buffer_dirA[2] = 1;

        for (uint32_t i = 0; i < device->info.iBasicEncoderCount; i++) {
            buffer_dirA[8 + i] = device->Encoders[i].dirAkeyCode;
            buffer_dirA[33 + i] = device->Encoders[i].dirAkeyModifier;
        }

        CreateRequestAsync(device, 0xC6, params_dir_mapping, 1, NULL, 0, NULL);
        memcpy(device->request + 8, buffer_dirA + 8, device->info.iBasicEncoderCount);
        memcpy(device->request + 33, buffer_dirA + 33, device->info.iBasicEncoderCount);

        // 4. Direction B key mapping (0xC7)
        uint8_t buffer_dirB[64] = {0};
        buffer_dirB[0] = 0xBB;
        buffer_dirB[1] = 0xC7;
        buffer_dirB[2] = 1;

        for (uint32_t i = 0; i < device->info.iBasicEncoderCount; i++) {
            buffer_dirB[8 + i] = device->Encoders[i].dirBkeyCode;
            buffer_dirB[33 + i] = device->Encoders[i].dirBkeyModifier;
        }

        CreateRequestAsync(device, 0xC7, params_dir_mapping, 1, NULL, 0, NULL);
        memcpy(device->request + 8, buffer_dirB + 8, device->info.iBasicEncoderCount);
        memcpy(device->request + 33, buffer_dirB + 33, device->info.iBasicEncoderCount);
    }

    if (device->info.iFastEncoders)
    {
        // 5. Set fast encoders (0xCE)
        uint8_t params_fast[] = { device->FastEncodersConfiguration, device->FastEncodersOptions };
        CreateRequestAsync(device, 0xCE, params_fast, 2, NULL, 0);
    }

    if (device->info.iUltraFastEncoders)
    {
        // 6. Set ultra fast encoders (0x1C)
        uint8_t params_ultra[] = { device->UltraFastEncoderConfiguration, device->UltraFastEncoderOptions };
        CreateRequestAsync(device, 0x1C, params_ultra, 2, NULL, 0);
        CreateRequestAsync(device, 0x1C, NULL, 0, 
                           &device->UltraFastEncoderFilter, sizeof(device->UltraFastEncoderFilter));
    }

    return PK_OK;
}

/**
 * @brief Starts asynchronous encoder values retrieval.
 *
 * @param device Pointer to PoKeys device
 * @return PK_OK on success, PK_ERR otherwise
 */
int PK_EncoderValuesGetAsync(sPoKeysDevice *device)
{
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    // Read the first 13 encoders (always needed)
    CreateRequestAsync(device, 0xCD, NULL, 0,
                       &device->Encoders[0].encoderValue,
                       13 * sizeof(uint32_t)); // 13 encoders × 4 bytes each

    if (device->info.iBasicEncoderCount >= 25)
    {
        // Depending if UltraFast is present
        if (device->info.iUltraFastEncoders != 0)
        {
            // Read the next 13 encoders
            uint8_t param_next = 1; // Page 1
            CreateRequestAsync(device, 0xCD, &param_next, 1,
                               &device->Encoders[13].encoderValue,
                               13 * sizeof(uint32_t)); // 13 more encoders

            // Read UltraFast encoder extra values
            uint8_t params_ultrafast[] = {0x37}; // specific sub-command
            CreateRequestAsync(device, 0x85, params_ultrafast, 1,
                               &device->PEv2.EncoderIndexCount,
                               3 * sizeof(uint32_t)); // 3 fields: IndexCount, TicksPerRotation, Velocity
        }
        else
        {
            // Fast mode: Read next 12 encoders only (no UltraFast)
            uint8_t param_next = 1; // Page 1
            CreateRequestAsync(device, 0xCD, &param_next, 1,
                               &device->Encoders[13].encoderValue,
                               12 * sizeof(uint32_t)); // 12 encoders
        }
    }

    return PK_OK;
}

/**
 * @brief Starts asynchronous encoder values set sequence.
 *
 * @param device Pointer to PoKeys device
 * @return PK_OK on success, PK_ERR otherwise
 */
int PK_EncoderValuesSetAsync(sPoKeysDevice *device)
{
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    if (device->info.iBasicEncoderCount >= 13)
    {
        // Write first 13 encoder values (0xCD, param 10)
        uint8_t param_first = 10;
        CreateRequestAsync(device, 0xCD, &param_first, 1,
                           &device->Encoders[0].encoderValue,
                           13 * sizeof(uint32_t)); // 13 encoders × 4 bytes each
    }

    if (device->info.iBasicEncoderCount >= 25)
    {
        // Write next 12 encoder values (0xCD, param 11)
        uint8_t param_next = 11;
        CreateRequestAsync(device, 0xCD, &param_next, 1,
                           &device->Encoders[13].encoderValue,
                           13 * sizeof(uint32_t)); // 13 more encoders
    }

    return PK_OK;
}
