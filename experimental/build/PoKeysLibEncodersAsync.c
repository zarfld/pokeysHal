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
        r = hal_param_u32_newf(HAL_RW, &(device->FastEncodersConfiguration), comp_id, "%s.encoder.fast.Configuration", prefix);
        if (r != 0) return r;
    
        r = hal_param_u32_newf(HAL_RW, &(device->FastEncodersOptions), comp_id, "%s.encoder.fast.Options", prefix);
        if (r != 0) return r;
    
        r = hal_param_bit_newf(HAL_OUT, &(device->FastEncoder_enable), comp_id, "%s.encoder.fast.enable", prefix);
        if (r != 0) return r;
    
        r = hal_param_bit_newf(HAL_OUT, &(device->FastEncoder_invert_E1), comp_id, "%s.encoder.fast.invert_E1", prefix);
        if (r != 0) return r;
    
        r = hal_param_bit_newf(HAL_OUT, &(device->FastEncoder_invert_E2), comp_id, "%s.encoder.fast.invert_E2", prefix);
        if (r != 0) return r;
    
        r = hal_param_bit_newf(HAL_OUT, &(device->FastEncoder_invert_E3), comp_id, "%s.encoder.fast.invert_E3", prefix);
        if (r != 0) return r;
    
        r = hal_param_bit_newf(HAL_OUT, &(device->FastEncoder_disable_4x), comp_id, "%s.encoder.fast.disable_4x", prefix);
        if (r != 0) return r;
    }

    if (device->info.iUltraFastEncoders) {
        r = hal_param_u32_newf(HAL_RW, &(device->UltraFastEncoderConfiguration), comp_id, "%s.encoder.ultra.Configuration", prefix);
        if (r != 0) return r;
        r = hal_param_u32_newf(HAL_RW, &(device->UltraFastEncoderOptions), comp_id, "%s.encoder.ultra.Options", prefix);
        if (r != 0) return r;
        r = hal_param_u32_newf(HAL_RW, &(device->UltraFastEncoderFilter), comp_id, "%s.encoder.ultra.Filter", prefix);
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
 * @brief Parses FastEncodersOptions byte into individual HAL parameters.
 *
 * @param device Pointer to PoKeys device
 * @param response Pointer to received UDP packet
 * @return PK_OK on success, PK_ERR otherwise
 */
 int PK_FastEncodersOptionsParse(sPoKeysDevice* device, const uint8_t* response)
 {
     if (device == NULL || response == NULL)
         return PK_ERR_TRANSFER;
 
     device->FastEncodersConfiguration = response[2];
     device->FastEncodersOptions = response[3];
 
     uint8_t options = response[3];
     device->FastEncoder_enable        = (options & (1 << 0)) ? 1 : 0;
     device->FastEncoder_invert_E1     = (options & (1 << 1)) ? 1 : 0;
     device->FastEncoder_invert_E2     = (options & (1 << 2)) ? 1 : 0;
     device->FastEncoder_invert_E3     = (options & (1 << 3)) ? 1 : 0;
     device->FastEncoder_disable_4x    = (options & (1 << 4)) ? 1 : 0;
 
     return PK_OK;
 }
 

 int PK_UltraFastEncoderOptionsParse(sPoKeysDevice* device, const uint8_t* response)
 {
     if (device == NULL || response == NULL)
         return PK_ERR_TRANSFER;
 
     // Extract configuration fields
     device->UltraFastEncoderConfiguration = response[2];
     device->UltraFastEncoderOptions = response[3];
     device->UltraFastEncoderFilter = *((uint32_t*)&response[8]); // Little endian assumed
 
     // Unpack UltraFastEncoderOptions bits
     uint8_t options = response[3];
     device->UltraFastEncoder_invert_direction = (options & (1 << 0)) ? 1 : 0;
     device->UltraFastEncoder_signal_mode      = (options & (1 << 1)) ? 1 : 0;
     device->UltraFastEncoder_enable_4x        = (options & (1 << 2)) ? 1 : 0;
 
     return PK_OK;
 }
 
 int PK_EncoderValuesGetAsync_ProcessPage0(sPoKeysDevice *device, const uint8_t *response)
 {
     if (device == NULL || response == NULL) return PK_ERR_TRANSFER;
 
     for (uint32_t i = 0; i < 13; i++) {
        *(device->Encoders[i].encoderValue) = *(uint32_t*)&response[8 + (i * 4)];
     }
     return PK_OK;
 }
 
 int PK_EncoderValuesGetAsync_ProcessPage1(sPoKeysDevice *device, const uint8_t *response)
 {
     if (device == NULL || response == NULL) return PK_ERR_TRANSFER;
 
     for (uint32_t i = 0; i < 13; i++) {
        *(device->Encoders[13 + i].encoderValue) = *(uint32_t*)&response[8 + (i * 4)];
     }
     return PK_OK;
 }
 
 int PK_EncoderValuesGetAsync_ProcessPage1_FastOnly(sPoKeysDevice *device, const uint8_t *response)
 {
     if (device == NULL || response == NULL) return PK_ERR_TRANSFER;
 
     for (uint32_t i = 0; i < 12; i++) {
        *(device->Encoders[13 + i].encoderValue) = *(uint32_t*)&response[8 + (i * 4)];
     }
     return PK_OK;
 }
 
 int PK_EncoderValuesGetAsync_ProcessUltraFast(sPoKeysDevice *device, const uint8_t *response)
 {
     if (device == NULL || response == NULL) return PK_ERR_TRANSFER;
 
     device->PEv2.EncoderIndexCount    = *(uint32_t*)&response[8];
     device->PEv2.EncoderTicksPerRotation = *(uint32_t*)&response[12];
     device->PEv2.EncoderVelocity       = *(uint32_t*)&response[16];
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
        uint32_t temp = device->Encoders[i].encoderOptions;
        CreateRequestAsync(device, 0xC4, NULL, 0,
            &temp, sizeof(temp),PK_EncoderOptionsParse);
    }

    // Step 2: Channel mappings (0xC5)
    for (uint32_t i = 0; i < device->info.iBasicEncoderCount; i++) {
        // Channel A pin
        uint32_t tempA = device->Encoders[i].channelApin;
        CreateRequestAsync(device, 0xC5, NULL, 0,
            &tempA, sizeof(tempA),NULL);

        // Channel B pin
        uint32_t tempB = device->Encoders[i].channelBpin;
        CreateRequestAsync(device, 0xC5, NULL, 0,
            &tempB, sizeof(tempB),NULL);
    }

    if (device->info.iKeyMapping)
    {
        // Step 3: Direction A key mapping (0xC6)
        for (uint32_t i = 0; i < device->info.iBasicEncoderCount; i++) {
            uint32_t tempA1 = device->Encoders[i].dirAkeyCode;
            uint32_t tempA2 = device->Encoders[i].dirAkeyModifier;
            CreateRequestAsync(device, 0xC6, NULL, 0,
                &tempA1, sizeof(tempA1),NULL);
            CreateRequestAsync(device, 0xC6, NULL, 0,
                &tempA2, sizeof(tempA2),NULL);


        // Step 4: Direction B key mapping (0xC7)
        uint32_t tempB1 = device->Encoders[i].dirBkeyCode;
        uint32_t tempB2 = device->Encoders[i].dirBkeyModifier;
            CreateRequestAsync(device, 0xC7, NULL, 0,
                &tempB1, sizeof(tempB1),NULL);
            CreateRequestAsync(device, 0xC7, NULL, 0,
                &tempB2, sizeof(tempB2),NULL);
        }
    }

    if (device->info.iFastEncoders)
    {
        uint8_t params_fast[] = {2}; // As seen in original 0xCE, param=2

        CreateRequestAsync(device, 0xCE, params_fast, 1,
                   NULL, 0,
                   PK_FastEncodersOptionsParse);
    }

    if (device->info.iUltraFastEncoders)
    {
        uint8_t params_ultra[] = {0xFF}; // 0x1C, param=0xFF
        CreateRequestAsync(device, 0x1C, params_ultra, 1,
            NULL, 0,
            PK_UltraFastEncoderOptionsParse);
    }

    return PK_OK;
}

/**
 * @brief Asynchronously sets the full PoKeys encoder configuration.
 *
 * Prepares asynchronous requests to configure:
 * - Basic encoder options (0xC4)
 * - Encoder channel A/B mappings (0xC5)
 * - Direction A and B key codes and modifiers (0xC6/0xC7)
 * - Fast encoder configuration (0xCE)
 * - Ultra fast encoder configuration and filter (0x1C)
 *
 * @param device Pointer to PoKeys device structure.
 * @return PK_OK (0) on success, or negative error code.
 */
 int PK_EncoderConfigurationSetAsync(sPoKeysDevice *device)
 {
     if (device == NULL) return PK_ERR_NOT_CONNECTED;
     if (!device->info.iBasicEncoderCount) return PK_ERR_NOT_SUPPORTED;
 
     uint8_t params_basic[] = {1};       // Write mode
     uint8_t params_mapping[] = {1};
     uint8_t params_dir_mapping[] = {1};
     uint8_t params_fast[] = {1};
     uint8_t params_ultra[] = {1};
 
     // 1. Set basic encoder options (0xC4)
     uint8_t payload_encoderOptions[56] = {0};
     for (uint32_t i = 0; i < device->info.iBasicEncoderCount; i++) {
         payload_encoderOptions[i] = device->Encoders[i].encoderOptions;
     }
     CreateRequestAsyncWithPayload(device, 0xC4, params_basic, 1,
                                    payload_encoderOptions, device->info.iBasicEncoderCount,
                                    NULL);
 
     // 2. Set channel mappings (0xC5)
     uint8_t payload_mapping[56] = {0};
     for (uint32_t i = 0; i < device->info.iBasicEncoderCount; i++) {
         payload_mapping[i] = device->Encoders[i].channelApin;
         payload_mapping[25 + i + 1] = device->Encoders[i].channelBpin;
     }
     CreateRequestAsyncWithPayload(device, 0xC5, params_mapping, 1,
                                    payload_mapping, sizeof(payload_mapping),
                                    NULL);
 
     if (device->info.iKeyMapping)
     {
         // 3. Direction A mapping (0xC6)
         uint8_t payload_dirA[56] = {0};
         uint8_t payload_dirB[56] = {0};
 
         for (uint32_t i = 0; i < device->info.iBasicEncoderCount; i++) {
             payload_dirA[i] = device->Encoders[i].dirAkeyCode;
             payload_dirA[25 + i + 1] = device->Encoders[i].dirAkeyModifier;
 
             payload_dirB[i] = device->Encoders[i].dirBkeyCode;
             payload_dirB[25 + i + 1] = device->Encoders[i].dirBkeyModifier;
         }
 
         CreateRequestAsyncWithPayload(device, 0xC6, params_dir_mapping, 1,
                                        payload_dirA, sizeof(payload_dirA),
                                        NULL);
 
         // 4. Direction B mapping (0xC7)
         CreateRequestAsyncWithPayload(device, 0xC7, params_dir_mapping, 1,
                                        payload_dirB, sizeof(payload_dirB),
                                        NULL);
     }
 
     if (device->info.iFastEncoders)
     {
         // 5. Set fast encoders (0xCE)
         uint8_t payload_fast[2] = {
             device->FastEncodersConfiguration,
             device->FastEncodersOptions
         };
 
         CreateRequestAsyncWithPayload(device, 0xCE, params_fast, 1,
                                        payload_fast, sizeof(payload_fast),
                                        NULL);
     }
 
     if (device->info.iUltraFastEncoders)
     {
         // 6. Set ultra fast encoders (0x1C)
         uint8_t payload_ultra[6] = {0};
         payload_ultra[0] = device->UltraFastEncoderConfiguration;
         payload_ultra[1] = device->UltraFastEncoderOptions;
 
         uint32_t filter = device->UltraFastEncoderFilter;
         payload_ultra[2] = (filter >>  0) & 0xFF;
         payload_ultra[3] = (filter >>  8) & 0xFF;
         payload_ultra[4] = (filter >> 16) & 0xFF;
         payload_ultra[5] = (filter >> 24) & 0xFF;
 
         CreateRequestAsyncWithPayload(device, 0x1C, params_ultra, 1,
                                        payload_ultra, sizeof(payload_ultra),
                                        NULL);
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
 
     // Always read the first 13 encoders
     CreateRequestAsync(device, 0xCD, NULL, 0,
         NULL, 0, // target_ptr=NULL: will parse manually
         PK_EncoderValuesGetAsync_ProcessPage0);
 
     if (device->info.iBasicEncoderCount >= 25)
     {
         if (device->info.iUltraFastEncoders != 0)
         {
             // Read the next 13 encoders (Page 1, with UltraFast)
             uint8_t param_next = 1;
             CreateRequestAsync(device, 0xCD, &param_next, 1,
                 NULL, 0,
                 PK_EncoderValuesGetAsync_ProcessPage1);
 
             // Read UltraFast encoder extra values
             uint8_t params_ultrafast[] = {0x37}; // Sub-command for test mode
             CreateRequestAsync(device, 0x85, params_ultrafast, 1,
                 NULL, 0,
                 PK_EncoderValuesGetAsync_ProcessUltraFast);
         }
         else
         {
             // Fast encoders only: next 12 encoders (Page 1)
             uint8_t param_next = 1;
             CreateRequestAsync(device, 0xCD, &param_next, 1,
                 NULL, 0,
                 PK_EncoderValuesGetAsync_ProcessPage1_FastOnly);
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
         uint8_t param_first = 10;
         uint8_t payload[56] = {0}; // Payload for 13 encoders × 4 bytes each
 
         for (uint32_t i = 0; i < 13; i++) {
             uint32_t val = (uint32_t)*(device->Encoders[i].encoderValue);
             payload[8 + i * 4 + 0] = (val >> 0) & 0xFF;
             payload[8 + i * 4 + 1] = (val >> 8) & 0xFF;
             payload[8 + i * 4 + 2] = (val >> 16) & 0xFF;
             payload[8 + i * 4 + 3] = (val >> 24) & 0xFF;
         }
 
         CreateRequestAsyncWithPayload(device, 0xCD, &param_first, 1,
                                       &payload[8], 13 * 4, NULL); // Skip first 8 bytes
     }
 
     if (device->info.iBasicEncoderCount >= 25)
     {
         uint8_t param_next = 11;
         uint8_t payload[56] = {0};
 
         for (uint32_t i = 0; i < 13; i++) {
             uint32_t val = (uint32_t)*(device->Encoders[13 + i].encoderValue);
             payload[8 + i * 4 + 0] = (val >> 0) & 0xFF;
             payload[8 + i * 4 + 1] = (val >> 8) & 0xFF;
             payload[8 + i * 4 + 2] = (val >> 16) & 0xFF;
             payload[8 + i * 4 + 3] = (val >> 24) & 0xFF;
         }
 
         CreateRequestAsyncWithPayload(device, 0xCD, &param_next, 1,
                                       &payload[8], 13 * 4, NULL);
     }
 
     return PK_OK;
 }
 
