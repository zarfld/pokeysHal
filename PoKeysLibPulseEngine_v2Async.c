#include "PoKeysLibHal.h"
#include "PoKeysLibAsync.h"

// Forward declaration for transaction_find function
async_transaction_t* transaction_find(uint8_t request_id);

/*
 * Asynchronous helpers for Pulse Engine v2 configuration and status.
 * These mirror their blocking counterparts in PoKeysLibPulseEngine_v2.c
 * but schedule requests using CreateRequestAsync/SendRequestAsync for
 * realtime-compatible, non-blocking communication.
 */

static void PK_PEv2_DecodeStatusFromResp(sPoKeysDevice *dev, const uint8_t *ans)
{
    if (!dev || !ans) return;
    dev->PEv2.SoftLimitStatus = ans[3];
    dev->PEv2.AxisEnabledStatesMask = ans[4];
    dev->PEv2.LimitOverride = ans[5];
    dev->PEv2.PulseEngineEnabled = ans[8];
    dev->PEv2.PulseEngineActivated = ans[9];
    dev->PEv2.PulseEngineState = ans[10];
    dev->PEv2.ChargePumpEnabled = ans[11];
    dev->PEv2.PulseGeneratorType = ans[15];
    dev->PEv2.LimitStatusP = ans[12];
    dev->PEv2.LimitStatusN = ans[13];
    dev->PEv2.HomeStatus = ans[14];
    memcpy(dev->PEv2.AxesState, ans + 16, 8);
    for (int i = 0; i < 8; i++) {
        dev->PEv2.CurrentPosition[i] =
            ((int32_t)ans[24 + i * 4]) |
            ((int32_t)ans[25 + i * 4] << 8) |
            ((int32_t)ans[26 + i * 4] << 16) |
            ((int32_t)ans[27 + i * 4] << 24);
    }
    dev->PEv2.info.nrOfAxes = ans[56];
    dev->PEv2.info.maxPulseFrequency = ans[57];
    dev->PEv2.info.bufferDepth = ans[58];
    dev->PEv2.info.slotTiming = ans[59];
    dev->PEv2.EmergencySwitchPolarity = ans[60];
    dev->PEv2.ErrorInputStatus = ans[61];
    dev->PEv2.MiscInputStatus = ans[62];
}

static int PK_PEv2_StatusParse(sPoKeysDevice *dev, const uint8_t *resp)
{
    if (!dev || !resp) return PK_ERR_GENERIC;
    uint8_t req_id = resp[6];
    uint8_t tstB = (0x10 + req_id) % 199;
    if (resp[63] != (uint8_t)(tstB + 0x5A)) {
        dev->PEv2.PulseEngineActivated = 0;
        dev->PEv2.PulseEngineEnabled = 0;
        return PK_ERR_GENERIC;
    }
    PK_PEv2_DecodeStatusFromResp(dev, resp);
    return PK_OK;
}

static int PK_PEv2_Status2Parse(sPoKeysDevice *dev, const uint8_t *resp)
{
    if (!dev || !resp) return PK_ERR_GENERIC;
    dev->PEv2.DedicatedLimitNInputs = resp[8];
    dev->PEv2.DedicatedLimitPInputs = resp[9];
    dev->PEv2.DedicatedHomeInputs = resp[10];
    return PK_OK;
}

static int PK_PEv2_AdditionalParamsParse(sPoKeysDevice *dev, const uint8_t *resp)
{
    if (!dev || !resp) return PK_ERR_GENERIC;
    dev->PEv2.EmergencyInputPin = resp[8];
    return PK_OK;
}

static int PK_PEv2_AxisConfigParse(sPoKeysDevice *dev, const uint8_t *resp)
{
    if (!dev || !resp) return PK_ERR_GENERIC;
    sPoKeysPEv2 *pe = &dev->PEv2;
    uint8_t ax = pe->param1;
    pe->AxesConfig[ax] = resp[8];
    pe->AxesSwitchConfig[ax] = resp[9];
    pe->PinHomeSwitch[ax] = resp[10];
    pe->PinLimitMSwitch[ax] = resp[11];
    pe->PinLimitPSwitch[ax] = resp[12];
    pe->HomingSpeed[ax] = resp[13];
    pe->HomingReturnSpeed[ax] = resp[14];
    pe->MPGjogEncoder[ax] = resp[15];
    pe->MaxSpeed[ax] = *(float*)(resp + 16);
    pe->MaxAcceleration[ax] = *(float*)(resp + 20);
    pe->MaxDecceleration[ax] = *(float*)(resp + 24);
    pe->SoftLimitMinimum[ax] = *(int32_t*)(resp + 28);
    pe->SoftLimitMaximum[ax] = *(int32_t*)(resp + 32);
    pe->MPGjogMultiplier[ax] = *(int16_t*)(resp + 36);
    pe->AxisEnableOutputPins[ax] = resp[38];
    pe->InvertAxisEnable[ax] = resp[39];
    pe->FilterLimitMSwitch[ax] = resp[40];
    pe->FilterLimitPSwitch[ax] = resp[41];
    pe->FilterHomeSwitch[ax] = resp[42];
    pe->HomingAlgorithm[ax] = resp[43];
    pe->HomeBackOffDistance[ax] = *(uint32_t*)(resp + 45);
    pe->MPGjogDivider[ax] = *(uint16_t*)(resp + 49);
    pe->AxisSignalOptions[ax] = resp[51];
    pe->FilterProbeInput = resp[52];
    return PK_OK;
}

int PK_PEv2_StatusGetAsync(sPoKeysDevice *device)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    int req = CreateRequestAsync(device, PK_CMD_PULSE_ENGINE_V2,
                                 (const uint8_t[]){PEV2_CMD_GET_STATUS,0}, 2,
                                 NULL, 0, PK_PEv2_StatusParse);
    if (req < 0) return req;
    async_transaction_t *t = transaction_find(req);
    if (!t) return PK_ERR_GENERIC;
    uint8_t tstB = (0x10 + req) % 199;
    t->request_buffer[3] = tstB;
    uint8_t cs = 0;
    for (int i=0;i<=6;i++) cs += t->request_buffer[i];
    t->request_buffer[7] = cs;
    return SendRequestAsync(device, req);
}

int PK_PEv2_Status2GetAsync(sPoKeysDevice *device)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    int req = CreateRequestAsync(device, PK_CMD_PULSE_ENGINE_V2,
                                 (const uint8_t[]){PEV2_CMD_GET_STATUS2}, 1,
                                 NULL, 0, PK_PEv2_Status2Parse);
    if (req < 0) return req;
    return SendRequestAsync(device, req);
}

int PK_PEv2_PulseEngineSetupAsync(sPoKeysDevice *device)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    uint8_t payload[6] = {
        device->PEv2.PulseEngineEnabled,
        device->PEv2.ChargePumpEnabled,
        device->PEv2.PulseGeneratorType,
        device->PEv2.PulseEngineBufferSize,
        device->PEv2.EmergencySwitchPolarity,
        device->PEv2.AxisEnabledStatesMask
    };
    int req = CreateRequestAsyncWithPayload(device, PK_CMD_PULSE_ENGINE_V2,
                                            (const uint8_t[]){PEV2_CMD_SETUP}, 1,
                                            payload, sizeof(payload), NULL);
    if (req < 0) return req;
    return SendRequestAsync(device, req);
}

int PK_PEv2_AdditionalParametersGetAsync(sPoKeysDevice *device)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    int req = CreateRequestAsync(device, PK_CMD_PULSE_ENGINE_V2,
                                 (const uint8_t[]){PEV2_CMD_CONFIGURE_MISC,0,0,1}, 4,
                                 NULL, 0, PK_PEv2_AdditionalParamsParse);
    if (req < 0) return req;
    return SendRequestAsync(device, req);
}

int PK_PEv2_AdditionalParametersSetAsync(sPoKeysDevice *device)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    uint8_t pin = device->PEv2.EmergencyInputPin;
    int req = CreateRequestAsyncWithPayload(device, PK_CMD_PULSE_ENGINE_V2,
                                            (const uint8_t[]){PEV2_CMD_CONFIGURE_MISC,1}, 2,
                                            &pin, 1, NULL);
    if (req < 0) return req;
    return SendRequestAsync(device, req);
}

int PK_PEv2_AxisConfigurationGetAsync(sPoKeysDevice *device)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    if (device->PEv2.param1 >= 8) return PK_ERR_PARAMETER;
    int req = CreateRequestAsync(device, PK_CMD_PULSE_ENGINE_V2,
                                 (const uint8_t[]){PEV2_CMD_GET_AXIS_CONFIGURATION, device->PEv2.param1}, 2,
                                 NULL, 0, PK_PEv2_AxisConfigParse);
    if (req < 0) return req;
    return SendRequestAsync(device, req);
}

int PK_PEv2_AxisConfigurationSetAsync(sPoKeysDevice *device)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    if (device->PEv2.param1 >= 8) return PK_ERR_PARAMETER;
    sPoKeysPEv2 *pe = &device->PEv2;
    uint8_t payload[45] = {0};
    payload[0] = pe->AxesConfig[pe->param1];
    payload[1] = pe->AxesSwitchConfig[pe->param1];
    payload[2] = pe->PinHomeSwitch[pe->param1];
    payload[3] = pe->PinLimitMSwitch[pe->param1];
    payload[4] = pe->PinLimitPSwitch[pe->param1];
    payload[5] = pe->HomingSpeed[pe->param1];
    payload[6] = pe->HomingReturnSpeed[pe->param1];
    payload[7] = pe->MPGjogEncoder[pe->param1];
    *(float*)(payload + 8)  = pe->MaxSpeed[pe->param1];
    *(float*)(payload + 12) = pe->MaxAcceleration[pe->param1];
    *(float*)(payload + 16) = pe->MaxDecceleration[pe->param1];
    *(int32_t*)(payload + 20) = pe->SoftLimitMinimum[pe->param1];
    *(int32_t*)(payload + 24) = pe->SoftLimitMaximum[pe->param1];
    *(int16_t*)(payload + 28) = (int16_t)pe->MPGjogMultiplier[pe->param1];
    payload[30] = pe->AxisEnableOutputPins[pe->param1];
    payload[31] = pe->InvertAxisEnable[pe->param1];
    payload[32] = pe->FilterLimitMSwitch[pe->param1];
    payload[33] = pe->FilterLimitPSwitch[pe->param1];
    payload[34] = pe->FilterHomeSwitch[pe->param1];
    payload[35] = pe->HomingAlgorithm[pe->param1];
    payload[36] = 0;
    *(uint32_t*)(payload + 37) = pe->HomeBackOffDistance[pe->param1];
    *(uint16_t*)(payload + 41) = pe->MPGjogDivider[pe->param1];
    payload[43] = pe->AxisSignalOptions[pe->param1];
    payload[44] = pe->FilterProbeInput;
    int req = CreateRequestAsyncWithPayload(device, PK_CMD_PULSE_ENGINE_V2,
                                            (const uint8_t[]){PEV2_CMD_SET_AXIS_CONFIGURATION, pe->param1}, 2,
                                            payload, sizeof(payload), NULL);
    if (req < 0) return req;
    return SendRequestAsync(device, req);
}

int PK_PEv2_PulseEngineMovePVAsync(sPoKeysDevice *device)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    
    // Prepare MovePV payload with position and velocity data
    uint8_t payload[56]; // 32 bytes pos + 16 bytes vel + 8 bytes header
    
    // Copy position commands (32 bytes)
    for (int i = 0; i < 8; i++) {
        int32_t pos = device->PEv2.ReferencePositionSpeed[i];
        payload[i*4+0] = (uint8_t)(pos & 0xFF);
        payload[i*4+1] = (uint8_t)((pos >> 8) & 0xFF);
        payload[i*4+2] = (uint8_t)((pos >> 16) & 0xFF);
        payload[i*4+3] = (uint8_t)((pos >> 24) & 0xFF);
    }
    
    // Copy velocity ratios (16 bytes)
    for (int i = 0; i < 8; i++) {
        uint16_t vel = (uint16_t)(device->PEv2.ReferenceVelocityPV[i] * 65535);
        payload[32 + i*2+0] = (uint8_t)(vel & 0xFF);
        payload[32 + i*2+1] = (uint8_t)((vel >> 8) & 0xFF);
    }
    
    int req = CreateRequestAsyncWithPayload(device, PK_CMD_PULSE_ENGINE_V2,
                                            (const uint8_t[]){PEV2_CMD_MOVE_PV, device->PEv2.param2}, 2,
                                            payload, 48, NULL);  // 32 + 16 bytes
    if (req < 0) return req;
    return SendRequestAsync(device, req);
}

int PK_PEv2_HomingStartAsync(sPoKeysDevice *device)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    uint8_t axis_mask = device->PEv2.param2;
    int req = CreateRequestAsyncWithPayload(device, PK_CMD_PULSE_ENGINE_V2,
                                            (const uint8_t[]){PEV2_CMD_START_HOMING}, 1,
                                            &axis_mask, 1, NULL);
    if (req < 0) return req;
    return SendRequestAsync(device, req);
}

int PK_PEv2_ExternalOutputsSetAsync(sPoKeysDevice *device)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    uint8_t payload[2] = {
        device->PEv2.ExternalRelayOutputs,
        device->PEv2.ExternalOCOutputs
    };
    int req = CreateRequestAsyncWithPayload(device, PK_CMD_PULSE_ENGINE_V2,
                                            (const uint8_t[]){PEV2_CMD_SET_OUTPUTS}, 1,
                                            payload, sizeof(payload), NULL);
    if (req < 0) return req;
    return SendRequestAsync(device, req);
}

/*
 * Parse callback for PK_PEv2_BufferFill[_16]Async.
 *
 * The BufferFill response is identical to the GetStatus response except
 * that byte[2] carries the number of accepted motion-buffer slots instead
 * of the status echo.  The remaining bytes [3..62] are the same engine
 * status fields decoded by PK_PEv2_DecodeStatusFromResp().
 */
static int PK_PEv2_BufferFillParse(sPoKeysDevice *dev, const uint8_t *resp)
{
    if (!dev || !resp) return PK_ERR_GENERIC;
    dev->PEv2.motionBufferEntriesAccepted = resp[2];
    PK_PEv2_DecodeStatusFromResp(dev, resp);
    return PK_OK;
}

/**
 * @brief Asynchronously transfer 8-bit motion buffer entries to the device.
 *
 * Mirrors @ref PK_PEv2_BufferFill but schedules the packet through the
 * async infrastructure.  @c newMotionBufferEntries must be set before
 * calling.  On completion the parse callback populates
 * @c motionBufferEntriesAccepted and refreshes the engine status cache.
 *
 * @param device PoKeys device handle.
 * @return Request ID (>= 0) on success, negative error code on failure.
 */
int PK_PEv2_BufferFillAsync(sPoKeysDevice *device)
{
    if (!device) return PK_ERR_NOT_CONNECTED;

    const uint8_t subcmds[4] = {
        (uint8_t)PEV2_CMD_FILL_BUFFER_8BIT,
        device->PEv2.newMotionBufferEntries,
        (uint8_t)(device->PEv2.PulseEngineEnabled & 0x0F),
        0
    };

    int req = CreateRequestAsyncWithPayload(device, PK_CMD_PULSE_ENGINE_V2,
                                            subcmds, 4,
                                            device->PEv2.MotionBuffer, 56,
                                            PK_PEv2_BufferFillParse);
    if (req < 0) return req;
    return SendRequestAsync(device, req);
}

/**
 * @brief Asynchronously transfer 16-bit motion buffer entries to the device.
 *
 * Identical to @ref PK_PEv2_BufferFillAsync but uses the 16-bit buffer
 * command (subcommand @c 0xFE / @c PEV2_CMD_FILL_BUFFER_16BIT).
 *
 * @param device PoKeys device handle.
 * @return Request ID (>= 0) on success, negative error code on failure.
 */
int PK_PEv2_BufferFill_16Async(sPoKeysDevice *device)
{
    if (!device) return PK_ERR_NOT_CONNECTED;

    const uint8_t subcmds[4] = {
        (uint8_t)PEV2_CMD_FILL_BUFFER_16BIT,
        device->PEv2.newMotionBufferEntries,
        (uint8_t)(device->PEv2.PulseEngineEnabled & 0x0F),
        0
    };

    int req = CreateRequestAsyncWithPayload(device, PK_CMD_PULSE_ENGINE_V2,
                                            subcmds, 4,
                                            device->PEv2.MotionBuffer, 56,
                                            PK_PEv2_BufferFillParse);
    if (req < 0) return req;
    return SendRequestAsync(device, req);
}

/**
 * @brief Asynchronously clear the device motion buffer.
 *
 * Mirrors @ref PK_PEv2_BufferClear.  No response data is parsed.
 *
 * @param device PoKeys device handle.
 * @return Request ID (>= 0) on success, negative error code on failure.
 */
int PK_PEv2_BufferClearAsync(sPoKeysDevice *device)
{
    if (!device) return PK_ERR_NOT_CONNECTED;

    const uint8_t subcmds[4] = {PEV2_CMD_CLEAR_BUFFER, 0, 0, 0};
    int req = CreateRequestAsync(device, PK_CMD_PULSE_ENGINE_V2,
                                 subcmds, 4,
                                 NULL, 0, NULL);
    if (req < 0) return req;
    return SendRequestAsync(device, req);
}

int export_pev2_pins(const char *prefix, long comp_id, sPoKeysDevice *device) {
    int r = 0;
    sPoKeysPEv2 *pev2 = &device->PEv2;

    rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: %s.PEv2 [8 axes]\n", __FILE__, __FUNCTION__, prefix);

    // Motion control pins - CRITICAL for LinuxCNC compatibility
    for (int i = 0; i < 8; i++) {
        rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: %s.PEv2.%01d.joint-pos-cmd\n", __FILE__, __FUNCTION__, prefix, i);
        r |= hal_pin_float_newf(HAL_IN, &pev2->pin_joint_pos_cmd[i], comp_id,
                               "%s.PEv2.%01d.joint-pos-cmd", prefix, i);
        rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: %s.PEv2.%01d.joint-vel-cmd\n", __FILE__, __FUNCTION__, prefix, i);
        r |= hal_pin_float_newf(HAL_IN, &pev2->pin_joint_vel_cmd[i], comp_id,
                               "%s.PEv2.%01d.joint-vel-cmd", prefix, i);
        rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: %s.PEv2.%01d.joint-pos-fb\n", __FILE__, __FUNCTION__, prefix, i);
        r |= hal_pin_float_newf(HAL_OUT, &pev2->pin_joint_pos_fb[i], comp_id,
                               "%s.PEv2.%01d.joint-pos-fb", prefix, i);
        rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: %s.PEv2.%01d.joint-in-position\n", __FILE__, __FUNCTION__, prefix, i);
        r |= hal_pin_bit_newf(HAL_OUT, &pev2->pin_joint_in_position[i], comp_id,
                             "%s.PEv2.%01d.joint-in-position", prefix, i);

        if (r != 0) {
            rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: motion pins axis %d failed\n", __FILE__, __FUNCTION__, i);
            return r;
        }
    }

    // State and command pins
    for (int i = 0; i < 8; i++) {
        rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: %s.PEv2.%01d.AxesState\n", __FILE__, __FUNCTION__, prefix, i);
        r |= hal_pin_u32_newf(HAL_OUT, &pev2->pin_AxesState[i], comp_id,
                             "%s.PEv2.%01d.AxesState", prefix, i);
        rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: %s.PEv2.%01d.AxesCommand\n", __FILE__, __FUNCTION__, prefix, i);
        r |= hal_pin_u32_newf(HAL_IN, &pev2->pin_AxesCommand[i], comp_id,
                             "%s.PEv2.%01d.AxesCommand", prefix, i);
        rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: %s.PEv2.%01d.CurrentPosition\n", __FILE__, __FUNCTION__, prefix, i);
        r |= hal_pin_s32_newf(HAL_OUT, &pev2->pin_CurrentPosition[i], comp_id,
                             "%s.PEv2.%01d.CurrentPosition", prefix, i);

        if (r != 0) {
            rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: state pins axis %d failed\n", __FILE__, __FUNCTION__, i);
            return r;
        }
    }

    // Device info pins
    rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: %s.PEv2.nrOfAxes\n", __FILE__, __FUNCTION__, prefix);
    r |= hal_pin_u32_newf(HAL_OUT, &pev2->pin_nrOfAxes, comp_id,
                         "%s.PEv2.nrOfAxes", prefix);
    rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: %s.PEv2.maxPulseFrequency\n", __FILE__, __FUNCTION__, prefix);
    r |= hal_pin_u32_newf(HAL_OUT, &pev2->pin_maxPulseFrequency, comp_id,
                         "%s.PEv2.maxPulseFrequency", prefix);
    rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: %s.PEv2.bufferDepth\n", __FILE__, __FUNCTION__, prefix);
    r |= hal_pin_u32_newf(HAL_OUT, &pev2->pin_bufferDepth, comp_id,
                         "%s.PEv2.bufferDepth", prefix);
    rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: %s.PEv2.slotTiming\n", __FILE__, __FUNCTION__, prefix);
    r |= hal_pin_u32_newf(HAL_OUT, &pev2->pin_slotTiming, comp_id,
                         "%s.PEv2.slotTiming", prefix);

    // Engine state pins
    rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: %s.PEv2.PulseEngineActivated\n", __FILE__, __FUNCTION__, prefix);
    r |= hal_pin_u32_newf(HAL_OUT, &pev2->pin_PulseEngineActivated, comp_id,
                         "%s.PEv2.PulseEngineActivated", prefix);
    rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: %s.PEv2.PulseEngineState\n", __FILE__, __FUNCTION__, prefix);
    r |= hal_pin_u32_newf(HAL_OUT, &pev2->pin_PulseEngineState, comp_id,
                         "%s.PEv2.PulseEngineState", prefix);

    // Emergency and safety pins
    rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: %s.PEv2.digin.Emergency.in\n", __FILE__, __FUNCTION__, prefix);
    r |= hal_pin_bit_newf(HAL_OUT, &pev2->pin_digin_Emergency_in, comp_id,
                         "%s.PEv2.digin.Emergency.in", prefix);
    rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: %s.PEv2.digin.Emergency.in-not\n", __FILE__, __FUNCTION__, prefix);
    r |= hal_pin_bit_newf(HAL_OUT, &pev2->pin_digin_Emergency_in_not, comp_id,
                         "%s.PEv2.digin.Emergency.in-not", prefix);
    rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: %s.PEv2.digout.Emergency.out\n", __FILE__, __FUNCTION__, prefix);
    r |= hal_pin_bit_newf(HAL_IN, &pev2->pin_digout_Emergency_out, comp_id,
                         "%s.PEv2.digout.Emergency.out", prefix);

    // Limit switch pins per axis
    for (int i = 0; i < 8; i++) {
        rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: %s.PEv2.%01d.digin.LimitN.in\n", __FILE__, __FUNCTION__, prefix, i);
        r |= hal_pin_bit_newf(HAL_OUT, &pev2->pin_digin_LimitN_in[i], comp_id,
                             "%s.PEv2.%01d.digin.LimitN.in", prefix, i);
        rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: %s.PEv2.%01d.digin.LimitN.in-not\n", __FILE__, __FUNCTION__, prefix, i);
        r |= hal_pin_bit_newf(HAL_OUT, &pev2->pin_digin_LimitN_in_not[i], comp_id,
                             "%s.PEv2.%01d.digin.LimitN.in-not", prefix, i);
        rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: %s.PEv2.%01d.digin.LimitP.in\n", __FILE__, __FUNCTION__, prefix, i);
        r |= hal_pin_bit_newf(HAL_OUT, &pev2->pin_digin_LimitP_in[i], comp_id,
                             "%s.PEv2.%01d.digin.LimitP.in", prefix, i);
        rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: %s.PEv2.%01d.digin.LimitP.in-not\n", __FILE__, __FUNCTION__, prefix, i);
        r |= hal_pin_bit_newf(HAL_OUT, &pev2->pin_digin_LimitP_in_not[i], comp_id,
                             "%s.PEv2.%01d.digin.LimitP.in-not", prefix, i);
        rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: %s.PEv2.%01d.digin.Home.in\n", __FILE__, __FUNCTION__, prefix, i);
        r |= hal_pin_bit_newf(HAL_OUT, &pev2->pin_digin_Home_in[i], comp_id,
                             "%s.PEv2.%01d.digin.Home.in", prefix, i);
        rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: %s.PEv2.%01d.digin.Home.in-not\n", __FILE__, __FUNCTION__, prefix, i);
        r |= hal_pin_bit_newf(HAL_OUT, &pev2->pin_digin_Home_in_not[i], comp_id,
                             "%s.PEv2.%01d.digin.Home.in-not", prefix, i);

        if (r != 0) {
            rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: limit/home pins axis %d failed\n", __FILE__, __FUNCTION__, i);
            return r;
        }
    }

    // Homing pins
    for (int i = 0; i < 8; i++) {
        rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: %s.PEv2.%01d.HomingStatus\n", __FILE__, __FUNCTION__, prefix, i);
        r |= hal_pin_u32_newf(HAL_IO, &pev2->pin_HomingStatus[i], comp_id,
                             "%s.PEv2.%01d.HomingStatus", prefix, i);
        rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: %s.PEv2.%01d.index-enable\n", __FILE__, __FUNCTION__, prefix, i);
        r |= hal_pin_bit_newf(HAL_IO, &pev2->pin_index_enable[i], comp_id,
                             "%s.PEv2.%01d.index-enable", prefix, i);

        if (r != 0) {
            rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: homing pins axis %d failed\n", __FILE__, __FUNCTION__, i);
            return r;
        }
    }

    // External outputs
    for (int i = 0; i < 4; i++) {
        rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: %s.PEv2.digout.ExternalRelay-%01d.out\n", __FILE__, __FUNCTION__, prefix, i);
        r |= hal_pin_bit_newf(HAL_IN, &pev2->pin_digout_ExternalRelay_out[i], comp_id,
                             "%s.PEv2.digout.ExternalRelay-%01d.out", prefix, i);
        rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: %s.PEv2.digout.ExternalOC-%01d.out\n", __FILE__, __FUNCTION__, prefix, i);
        r |= hal_pin_bit_newf(HAL_IN, &pev2->pin_digout_ExternalOC_out[i], comp_id,
                             "%s.PEv2.digout.ExternalOC-%01d.out", prefix, i);
    }

    rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: %s.PEv2.ExternalRelayOutputs\n", __FILE__, __FUNCTION__, prefix);
    r |= hal_pin_u32_newf(HAL_OUT, &pev2->pin_ExternalRelayOutputs, comp_id,
                         "%s.PEv2.ExternalRelayOutputs", prefix);
    rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: %s.PEv2.ExternalOCOutputs\n", __FILE__, __FUNCTION__, prefix);
    r |= hal_pin_u32_newf(HAL_OUT, &pev2->pin_ExternalOCOutputs, comp_id,
                         "%s.PEv2.ExternalOCOutputs", prefix);

    // Debug and diagnostics pins
    rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: %s.PEv2.debug.test-enable\n", __FILE__, __FUNCTION__, prefix);
    r |= hal_pin_bit_newf(HAL_IN, &pev2->pin_debug_test_enable, comp_id,
                         "%s.PEv2.debug.test-enable", prefix);
    rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: %s.PEv2.debug.cycle-time-ns\n", __FILE__, __FUNCTION__, prefix);
    r |= hal_pin_u32_newf(HAL_OUT, &pev2->pin_debug_cycle_time, comp_id,
                         "%s.PEv2.debug.cycle-time-ns", prefix);
    rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: %s.PEv2.debug.error-count\n", __FILE__, __FUNCTION__, prefix);
    r |= hal_pin_u32_newf(HAL_OUT, &pev2->pin_debug_error_count, comp_id,
                         "%s.PEv2.debug.error-count", prefix);
    rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: %s.PEv2.debug.commands-sent\n", __FILE__, __FUNCTION__, prefix);
    r |= hal_pin_u32_newf(HAL_OUT, &pev2->pin_debug_cmd_sent, comp_id,
                         "%s.PEv2.debug.commands-sent", prefix);
    rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: %s.PEv2.debug.commands-failed\n", __FILE__, __FUNCTION__, prefix);
    r |= hal_pin_u32_newf(HAL_OUT, &pev2->pin_debug_cmd_failed, comp_id,
                         "%s.PEv2.debug.commands-failed", prefix);
    rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: %s.PEv2.debug.communication-ok\n", __FILE__, __FUNCTION__, prefix);
    r |= hal_pin_bit_newf(HAL_OUT, &pev2->pin_debug_comm_ok, comp_id,
                         "%s.PEv2.debug.communication-ok", prefix);
    rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: %s.PEv2.debug.test-frequency\n", __FILE__, __FUNCTION__, prefix);
    r |= hal_pin_float_newf(HAL_IN, &pev2->pin_debug_test_freq, comp_id,
                         "%s.PEv2.debug.test-frequency", prefix);

    // Performance monitoring pins
    rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: %s.PEv2.perf.rt-min-cycle-ns\n", __FILE__, __FUNCTION__, prefix);
    r |= hal_pin_u32_newf(HAL_OUT, &pev2->pin_perf_rt_min_cycle, comp_id,
                         "%s.PEv2.perf.rt-min-cycle-ns", prefix);
    rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: %s.PEv2.perf.rt-max-cycle-ns\n", __FILE__, __FUNCTION__, prefix);
    r |= hal_pin_u32_newf(HAL_OUT, &pev2->pin_perf_rt_max_cycle, comp_id,
                         "%s.PEv2.perf.rt-max-cycle-ns", prefix);
    rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: %s.PEv2.perf.rt-avg-cycle-ns\n", __FILE__, __FUNCTION__, prefix);
    r |= hal_pin_u32_newf(HAL_OUT, &pev2->pin_perf_rt_avg_cycle, comp_id,
                         "%s.PEv2.perf.rt-avg-cycle-ns", prefix);

    // Probing pins
    for (int i = 0; i < 8; i++) {
        rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: %s.PEv2.%01d.ProbePosition\n", __FILE__, __FUNCTION__, prefix, i);
        r |= hal_pin_u32_newf(HAL_IO, &pev2->pin_ProbePosition[i], comp_id,
                             "%s.PEv2.%01d.ProbePosition", prefix, i);
        rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: %s.PEv2.%01d.ProbeMaxPosition\n", __FILE__, __FUNCTION__, prefix, i);
        r |= hal_pin_u32_newf(HAL_IO, &pev2->pin_ProbeMaxPosition[i], comp_id,
                             "%s.PEv2.%01d.ProbeMaxPosition", prefix, i);
    }

    rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: %s.PEv2.ProbeStatus\n", __FILE__, __FUNCTION__, prefix);
    r |= hal_pin_u32_newf(HAL_OUT, &pev2->pin_ProbeStatus, comp_id,
                         "%s.PEv2.ProbeStatus", prefix);
    rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: %s.PEv2.digin.Probed.in\n", __FILE__, __FUNCTION__, prefix);
    r |= hal_pin_bit_newf(HAL_OUT, &pev2->pin_digin_Probed_in, comp_id,
                         "%s.PEv2.digin.Probed.in", prefix);

    // MPG jogging pins
    for (int i = 0; i < 8; i++) {
        rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: %s.PEv2.%01d.joint-kb-jog-active\n", __FILE__, __FUNCTION__, prefix, i);
        r |= hal_pin_bit_newf(HAL_IN, &pev2->pin_joint_kb_jog_active[i], comp_id,
                             "%s.PEv2.%01d.joint-kb-jog-active", prefix, i);
        rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: %s.PEv2.%01d.joint-wheel-jog-active\n", __FILE__, __FUNCTION__, prefix, i);
        r |= hal_pin_bit_newf(HAL_IN, &pev2->pin_joint_wheel_jog_active[i], comp_id,
                             "%s.PEv2.%01d.joint-wheel-jog-active", prefix, i);
    }

    // Motion buffer mode pins
    rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: %s.PEv2.motion-buffer-mode\n", __FILE__, __FUNCTION__, prefix);
    r |= hal_pin_bit_newf(HAL_IN, &pev2->pin_motion_buffer_mode, comp_id,
                         "%s.PEv2.motion-buffer-mode", prefix);
    rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: %s.PEv2.motion-buf-entries\n", __FILE__, __FUNCTION__, prefix);
    r |= hal_pin_s32_newf(HAL_OUT, &pev2->pin_motion_buffer_entries_accepted, comp_id,
                         "%s.PEv2.motion-buf-entries", prefix);

    if (r != 0) {
        rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: Failed to export PEv2 HAL pins\n", __FILE__, __FUNCTION__);
        return r;
    }

    // Initialize parameters with defaults
    for (int i = 0; i < 8; i++) {
        pev2->MaxSpeed[i] = 1000.0;
        pev2->MaxAcceleration[i] = 100.0;
        pev2->home_sequence[i] = -1;
        pev2->stepgen_STEP_SCALE[i] = 1000.0;
        pev2->pos_scale[i] = 1000;
        pev2->pos_offset[i] = 0;
        pev2->step_width[i] = 1e-4f;
    }

    // Initialize motion buffer mode pins
    *(pev2->pin_motion_buffer_mode) = 0;
    *(pev2->pin_motion_buffer_entries_accepted) = 0;

    // Initialize debug and performance monitoring pins
    *(pev2->pin_debug_test_enable) = 0;
    *(pev2->pin_debug_cycle_time) = 0;
    *(pev2->pin_debug_error_count) = 0;
    *(pev2->pin_debug_cmd_sent) = 0;
    *(pev2->pin_debug_cmd_failed) = 0;
    *(pev2->pin_debug_comm_ok) = 0;
    *(pev2->pin_debug_test_freq) = 1.0;

    *(pev2->pin_perf_rt_min_cycle) = 0;
    *(pev2->pin_perf_rt_max_cycle) = 0;
    *(pev2->pin_perf_rt_avg_cycle) = 0;

    rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: PEv2 HAL pins exported successfully\n", __FILE__, __FUNCTION__);
    return 0;
}

