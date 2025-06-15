/*

Copyright (C) 2013 Matev� Bo�nak (matevz@poscope.com)

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

*/

#include "PoKeysLibHal.h"
#include "PoKeysLibCore.h"
#include "PoKeysLibAsync.h"


void PK_PEv2_DecodeStatus(sPoKeysDevice * device)
{
    uint8_t * ans;
    if (device == NULL) return;

    ans = device->response;

    device->PEv2.SoftLimitStatus = ans[3];
    device->PEv2.AxisEnabledStatesMask = ans[4];
    device->PEv2.LimitOverride = ans[5];

    // Basic engine states
    device->PEv2.PulseEngineEnabled = ans[8];
    device->PEv2.PulseEngineActivated = ans[9];
    device->PEv2.PulseEngineState = ans[10];
    device->PEv2.ChargePumpEnabled = ans[11];
    device->PEv2.PulseGeneratorType = ans[15];

    // Switch states
    device->PEv2.LimitStatusP = ans[12];
    device->PEv2.LimitStatusN = ans[13];
    device->PEv2.HomeStatus = ans[14];

    memcpy(device->PEv2.AxesState, ans + 16, 8);
    //memcpy(device->PEv2.CurrentPosition, ans + 24, 8*4);
    for (int i = 0; i < 8; i++) {
        device->PEv2.CurrentPosition[i] =
            ((int32_t)ans[24 + i * 4]) |
            ((int32_t)ans[25 + i * 4] << 8) |
            ((int32_t)ans[26 + i * 4] << 16) |
            ((int32_t)ans[27 + i * 4] << 24);
    }
    
    // Engine info
    device->PEv2.info.nrOfAxes = ans[56];
    device->PEv2.info.maxPulseFrequency = ans[57];
    device->PEv2.info.bufferDepth = ans[58];
    device->PEv2.info.slotTiming = ans[59];

    device->PEv2.EmergencySwitchPolarity = ans[60];

    // Other inputs
    device->PEv2.ErrorInputStatus = ans[61];
    device->PEv2.MiscInputStatus = ans[62];
}

/**
 * @brief Retrieve pulse engine status.
 *
 * Sends command `PEV2_CMD_GET_STATUS` (0x00) under
 * `PK_CMD_PULSE_ENGINE_V2`. `param1` is used as a handshake
 * value and the response updates the @ref sPoKeysPEv2 status
 * structure with axis states, current position and engine info.
 *
 * @param device Pointer to the PoKeys device.
 * @return PK_OK when successful, error code otherwise.
 */
int32_t PK_PEv2_StatusGet(sPoKeysDevice * device)
{
    // Do some 'random' magic with numbers
    uint8_t tstB;
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    tstB = (0x10 + device->requestID) % 199;

    // Send request
    CreateRequest(device->request, PK_CMD_PULSE_ENGINE_V2, PEV2_CMD_GET_STATUS, tstB, 0, 0);
    if (SendRequest(device) != PK_OK) return PK_ERR_TRANSFER;

    // Check if response is valid
    if (device->response[63] == (uint8_t)(tstB + 0x5A))
    {
        // Decode status
        PK_PEv2_DecodeStatus(device);
        return PK_OK;
    } else
    {
        device->PEv2.PulseEngineActivated = 0;
        device->PEv2.PulseEngineEnabled = 0;
        return PK_ERR_GENERIC;
    }

}



/**
 * @brief Obtain extended status information.
 *
 * Executes command `PEV2_CMD_GET_STATUS2` (0x08) of
 * `PK_CMD_PULSE_ENGINE_V2`. The response provides the count
 * of dedicated limit and home inputs which is stored in
 * `sPoKeysPEv2`.
 *
 * @param device Target device.
 * @return PK_OK on success or an error code from communication.
 */
int32_t PK_PEv2_Status2Get(sPoKeysDevice * device)
{
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    // Send request
    CreateRequest(device->request, PK_CMD_PULSE_ENGINE_V2, PEV2_CMD_GET_STATUS2, 0, 0, 0);
    if (SendRequest(device) != PK_OK) return PK_ERR_TRANSFER;

	device->PEv2.DedicatedLimitNInputs = device->response[8];
	device->PEv2.DedicatedLimitPInputs = device->response[9];
	device->PEv2.DedicatedHomeInputs = device->response[10];
	return PK_OK;
}

/**
 * @brief Configure the pulse engine parameters.
 *
 * Uses command `PEV2_CMD_SETUP` (0x01). The request payload
 * contains enable flags, buffer size and emergency switch polarity
 * in bytes 8-13.
 *
 * @param device Pointer to the PoKeys device.
 * @return PK_OK on success or an error code on failure.
 */
int32_t PK_PEv2_PulseEngineSetup(sPoKeysDevice * device)
{
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    CreateRequest(device->request, PK_CMD_PULSE_ENGINE_V2, PEV2_CMD_SETUP, 0, 0, 0);

    // Fill the information
    device->request[8] = device->PEv2.PulseEngineEnabled;
    device->request[9] = device->PEv2.ChargePumpEnabled;
    device->request[10] = device->PEv2.PulseGeneratorType;
    device->request[11] = device->PEv2.PulseEngineBufferSize;
    device->request[12] = device->PEv2.EmergencySwitchPolarity;
    device->request[13] = device->PEv2.AxisEnabledStatesMask;

    // Send request
    return SendRequest(device);
}



/**
 * @brief Read miscellaneous pulse engine parameters.
 *
 * Executes `PEV2_CMD_CONFIGURE_MISC` (0x06) with `param3=1`.
 * The emergency input pin is returned in byte 8 of the response
 * and stored in `PEv2.EmergencyInputPin`.
 *
 * @param device Target device.
 * @return PK_OK on success or communication error code.
 */
int32_t PK_PEv2_AdditionalParametersGet(sPoKeysDevice * device)
{
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    // Send request
    CreateRequest(device->request, PK_CMD_PULSE_ENGINE_V2, PEV2_CMD_CONFIGURE_MISC, 0, 0, 1);
    if (SendRequest(device) != PK_OK) return PK_ERR_TRANSFER;

	device->PEv2.EmergencyInputPin = device->response[8];    

    return PK_OK;
}

/**
 * @brief Configure miscellaneous pulse engine options.
 *
 * Uses command `PEV2_CMD_CONFIGURE_MISC` (0x06) with `param1=1`.
 * Byte 8 of the request carries the emergency input pin number.
 *
 * @param device Target device.
 * @return PK_OK on success, otherwise error code.
 */
int32_t PK_PEv2_AdditionalParametersSet(sPoKeysDevice * device)
{
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    // Create request
    CreateRequest(device->request, PK_CMD_PULSE_ENGINE_V2, PEV2_CMD_CONFIGURE_MISC, 1, 0, 0);
	device->request[8] = device->PEv2.EmergencyInputPin;

    // Send request
    return SendRequest(device);
}


/**
 * @brief Retrieve configuration for one axis.
 *
 * Issues `PEV2_CMD_GET_AXIS_CONFIGURATION` (0x10). `param1`
 * selects the axis and the response returns a 44 byte block
 * describing limits, speeds and filter options which is copied
 * into the `PEv2` structure.
 *
 * @param device PoKeys device instance (uses `PEv2.param1`).
 * @return PK_OK on success or error code on failure.
 */
int32_t PK_PEv2_AxisConfigurationGet(sPoKeysDevice * device)
{
    sPoKeysPEv2 * pe;
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    if (device->PEv2.param1 >= 8) return PK_ERR_PARAMETER;

    // Send request
    CreateRequest(device->request, PK_CMD_PULSE_ENGINE_V2, PEV2_CMD_GET_AXIS_CONFIGURATION, device->PEv2.param1, 0, 0);
    if (SendRequest(device) != PK_OK) return PK_ERR_TRANSFER;

    // Pointer to PEv2 structure for better code readability
    pe = &device->PEv2;

    // Read the structure
    pe->AxesConfig[pe->param1] = device->response[8];
    pe->AxesSwitchConfig[pe->param1] = device->response[9];

    pe->PinHomeSwitch[pe->param1] = device->response[10];
    pe->PinLimitMSwitch[pe->param1] = device->response[11];
    pe->PinLimitPSwitch[pe->param1] = device->response[12];

    pe->HomingSpeed[pe->param1] = device->response[13];
    pe->HomingReturnSpeed[pe->param1] = device->response[14];

    pe->MPGjogEncoder[pe->param1] = device->response[15];

    // Convert parameters... assume little-endian format
    pe->MaxSpeed[pe->param1] = *(float*)(device->response + 16);
    pe->MaxAcceleration[pe->param1] = *(float*)(device->response + 20);
    pe->MaxDecceleration[pe->param1] = *(float*)(device->response + 24);

    pe->SoftLimitMinimum[pe->param1] = *(int32_t*)(device->response + 28);
    pe->SoftLimitMaximum[pe->param1] = *(int32_t*)(device->response + 32);

    pe->MPGjogMultiplier[pe->param1] = *(int16_t*)(device->response + 36);

    pe->AxisEnableOutputPins[pe->param1] = device->response[38];
    pe->InvertAxisEnable[pe->param1] = device->response[39];

	pe->FilterLimitMSwitch[pe->param1] = device->response[40];
	pe->FilterLimitPSwitch[pe->param1] = device->response[41];
	pe->FilterHomeSwitch[pe->param1] = device->response[42];

	pe->HomingAlgorithm[pe->param1] = device->response[43];
	// MPG 1x mode here
	pe->HomeBackOffDistance[pe->param1] = *(uint32_t*)(device->response + 45);
    pe->MPGjogDivider[pe->param1] = *(uint16_t*)(device->response + 49);
	pe->AxisSignalOptions[pe->param1] = device->response[51];
	pe->FilterProbeInput = device->response[52];
    return PK_OK;
}

/**
 * @brief Send configuration data for one axis.
 *
 * Command `PEV2_CMD_SET_AXIS_CONFIGURATION` (0x11) writes the
 * same structure returned by @ref PK_PEv2_AxisConfigurationGet.
 * `param1` selects the axis and bytes 8 onward contain the
 * configuration fields such as limits and motion parameters.
 *
 * @param device PoKeys device instance.
 * @return PK_OK on success or error code on failure.
 */
int32_t PK_PEv2_AxisConfigurationSet(sPoKeysDevice * device)
{
    sPoKeysPEv2 * pe;
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    if (device->PEv2.param1 >= 8) return PK_ERR_PARAMETER;

    // Create request
    CreateRequest(device->request, PK_CMD_PULSE_ENGINE_V2, PEV2_CMD_SET_AXIS_CONFIGURATION, device->PEv2.param1, 0, 0);

    // Pointer to PEv2 structure for better code readability
    pe = &device->PEv2;

    // Fill the structure
    device->request[8] = pe->AxesConfig[pe->param1];
    device->request[9] = pe->AxesSwitchConfig[pe->param1];

    device->request[10] = pe->PinHomeSwitch[pe->param1];
    device->request[11] = pe->PinLimitMSwitch[pe->param1];
    device->request[12] = pe->PinLimitPSwitch[pe->param1];

    device->request[13] = pe->HomingSpeed[pe->param1];
    device->request[14] = pe->HomingReturnSpeed[pe->param1];

    device->request[15] = pe->MPGjogEncoder[pe->param1];

    *(float*)(device->request + 16) = pe->MaxSpeed[pe->param1];
    *(float*)(device->request + 20) = pe->MaxAcceleration[pe->param1];
    *(float*)(device->request + 24) = pe->MaxDecceleration[pe->param1];

    *(int32_t*)(device->request + 28) = pe->SoftLimitMinimum[pe->param1];
    *(int32_t*)(device->request + 32) = pe->SoftLimitMaximum[pe->param1];
    *(int16_t*)(device->request + 36) = (int16_t)pe->MPGjogMultiplier[pe->param1];

    device->request[38] = pe->AxisEnableOutputPins[pe->param1];
    device->request[39] = pe->InvertAxisEnable[pe->param1];

	device->request[40] = pe->FilterLimitMSwitch[pe->param1];
	device->request[41] = pe->FilterLimitPSwitch[pe->param1];
	device->request[42] = pe->FilterHomeSwitch[pe->param1];
	device->request[43] = pe->HomingAlgorithm[pe->param1];
	device->request[44] = 0;

	*(uint32_t*)(device->request + 45) = pe->HomeBackOffDistance[pe->param1];
    *(uint16_t*)(device->request + 49) = pe->MPGjogDivider[pe->param1];

	device->request[51] = pe->AxisSignalOptions[pe->param1];
	device->request[52] = pe->FilterProbeInput;

    // Send request
    return SendRequest(device);
}

/**
 * @brief Set current axis positions.
 *
 * Command `PEV2_CMD_SET_AXIS_POSITION` (0x03) updates the
 * internal position registers. `param2` is a bit mask selecting
 * which axes are affected and bytes 8-39 carry the new 32-bit
 * positions for all axes.
 *
 * @param device Device containing the new positions in
 *               `PEv2.PositionSetup`.
 * @return PK_OK on success or error code on failure.
 */
int32_t PK_PEv2_PositionSet(sPoKeysDevice * device)
{
    int i;
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    if (device->PEv2.param2 == 0) return PK_ERR_PARAMETER;

    // Create request
    CreateRequest(device->request, PK_CMD_PULSE_ENGINE_V2, PEV2_CMD_SET_AXIS_POSITION, device->PEv2.param2, 0, 0);

    for (i = 0; i < 8; i++)
    {
        *(int32_t*)(&device->request[8+i*4]) = device->PEv2.PositionSetup[i];
    }

    // Send request
    return SendRequest(device);
}

/**
 * @brief Change the operating state of the pulse engine.
 *
 * Sends `PEV2_CMD_SET_STATE` (0x02). Parameters specify the
 * desired engine state, limit override flag and axis enable mask
 * which are placed in `param1`, `param2` and `param3`.
 *
 * @param device Pointer to a PoKeys device with setup values
 *               stored in the `PEv2` structure.
 * @return PK_OK on success or error code.
 */
int32_t PK_PEv2_PulseEngineStateSet(sPoKeysDevice * device)
{
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    // Create request
    CreateRequest(device->request, PK_CMD_PULSE_ENGINE_V2, PEV2_CMD_SET_STATE, device->PEv2.PulseEngineStateSetup, device->PEv2.LimitOverrideSetup, device->PEv2.AxisEnabledMask);

    // Send request
    return SendRequest(device);
}

/**
 * @brief Execute a move using reference positions or speeds.
 *
 * Command `PEV2_CMD_MOVE` (0x20) accepts eight 32-bit position
 * or speed values located in bytes 8-39 of the request.
 * The values are taken from `PEv2.ReferencePositionSpeed`.
 *
 * @param device Active PoKeys device.
 * @return Result of SendRequest.
 */
int32_t PK_PEv2_PulseEngineMove(sPoKeysDevice * device)
{
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    // Create request
    CreateRequest(device->request, PK_CMD_PULSE_ENGINE_V2, PEV2_CMD_MOVE, 0, 0, 0);

    //memcpy(&device->request[8], device->PEv2.ReferencePositionSpeed, 8*4);
    for (int i = 0; i < 8; i++) {
        int32_t val = device->PEv2.ReferencePositionSpeed[i];  // read from volatile pointer
        device->request[8 + i * 4 + 0] = (uint8_t)(val & 0xFF);
        device->request[8 + i * 4 + 1] = (uint8_t)((val >> 8) & 0xFF);
        device->request[8 + i * 4 + 2] = (uint8_t)((val >> 16) & 0xFF);
        device->request[8 + i * 4 + 3] = (uint8_t)((val >> 24) & 0xFF);
    }
    
    // Send request
    return SendRequest(device);
}


/**
 * @brief Perform a move in position/velocity mode.
 *
 * Uses command `PEV2_CMD_MOVE_PV` (0x25). In addition to the
 * 32-bit reference positions, bytes 40-55 of the request include
 * 16-bit velocity ratios derived from `ReferenceVelocityPV`.
 * `param2` carries the enable mask.
 *
 * @param device Active PoKeys device.
 * @return PK_OK or error from SendRequest.
 */
int32_t PK_PEv2_PulseEngineMovePV(sPoKeysDevice * device)
{
    int i;
    uint16_t tmpVelocity;

    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    // Create request
    CreateRequest(device->request, PK_CMD_PULSE_ENGINE_V2, PEV2_CMD_MOVE_PV, device->PEv2.param2, 0, 0);

    //memcpy(&device->request[8], device->PEv2.ReferencePositionSpeed, 8*4);
    for (int i = 0; i < 8; i++) {
        int32_t val = device->PEv2.ReferencePositionSpeed[i];  // Read from volatile pin
        device->request[8 + i * 4 + 0] = (uint8_t)(val & 0xFF);
        device->request[8 + i * 4 + 1] = (uint8_t)((val >> 8) & 0xFF);
        device->request[8 + i * 4 + 2] = (uint8_t)((val >> 16) & 0xFF);
        device->request[8 + i * 4 + 3] = (uint8_t)((val >> 24) & 0xFF);
    }

    for (i = 0; i < 8; i++) {
        tmpVelocity = (uint16_t)(device->PEv2.ReferenceVelocityPV[i] * 65535);
        memcpy(&device->request[40 + i * 2], &tmpVelocity, 2);
    }

    // Send request
    return SendRequest(device);
}

/**
 * @brief Read the state of external outputs.
 *
 * Using `PEV2_CMD_SET_OUTPUTS` (0x04) with `param3=1` the device
 * returns relay and open-collector output states in bytes 3 and 4
 * of the response which are stored in the PEv2 structure.
 *
 * @param device Target device.
 * @return PK_OK when data is read successfully, otherwise error
 *         code from SendRequest.
 */
int32_t PK_PEv2_ExternalOutputsGet(sPoKeysDevice * device)
{
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    // Send request
    CreateRequest(device->request, PK_CMD_PULSE_ENGINE_V2, PEV2_CMD_SET_OUTPUTS, 0, 0, 1);
    if (SendRequest(device) != PK_OK) return PK_ERR_TRANSFER;

    device->PEv2.ExternalRelayOutputs = device->response[3];
    device->PEv2.ExternalOCOutputs = device->response[4];

    return PK_OK;
}

/**
 * @brief Update relay and open-collector outputs.
 *
 * Command `PEV2_CMD_SET_OUTPUTS` (0x04) uses `param1` for relay
 * outputs and `param2` for open-collector outputs.
 *
 * @param device Device with desired output states stored in the
 *               PEv2 structure.
 * @return Result of SendRequest.
 */
int32_t PK_PEv2_ExternalOutputsSet(sPoKeysDevice * device)
{
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    // Create request
    CreateRequest(device->request, PK_CMD_PULSE_ENGINE_V2, PEV2_CMD_SET_OUTPUTS, device->PEv2.ExternalRelayOutputs, device->PEv2.ExternalOCOutputs, 0);
    // Send request
    return SendRequest(device);
}

/**
 * @brief Send a portion of the motion buffer (8-bit variant).
 *
 * Uses command `PEV2_CMD_FILL_BUFFER_8BIT` (0xFF). `param1` holds
 * the number of entries to copy and `param2` carries the enable
 * mask. Up to 56 bytes of motion data are placed in bytes 8-63.
 * The response returns the number of accepted entries in byte 2
 * and refreshes engine status via @ref PK_PEv2_DecodeStatus.
 *
 * @param device PoKeys device containing the new buffer.
 * @return PK_OK when successful or a communication error.
 */
int32_t PK_PEv2_BufferFill(sPoKeysDevice * device)
{
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    // Create request
    CreateRequest(device->request, PK_CMD_PULSE_ENGINE_V2, PEV2_CMD_FILL_BUFFER_8BIT, device->PEv2.newMotionBufferEntries, device->PEv2.PulseEngineEnabled & 0x0F, 0);

    // Copy buffer
    memcpy(&device->request[8], device->PEv2.MotionBuffer, 56);

    // Send request
    if (SendRequest(device) != PK_OK) return PK_ERR_TRANSFER;

    device->PEv2.motionBufferEntriesAccepted = device->response[2];

    // Decode status
    PK_PEv2_DecodeStatus(device);

    return PK_OK;
}

/**
 * @brief Send 16-bit motion buffer entries.
 *
 * Identical to @ref PK_PEv2_BufferFill but uses the 16-bit buffer
 * command ID `0xFE`. Payload layout and response are the same as
 * in the 8-bit version.
 *
 * @param device PoKeys device containing the new data.
 * @return PK_OK on success or a transfer error.
 */
int32_t PK_PEv2_BufferFill_16(sPoKeysDevice * device)
{
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    // Create request
    CreateRequest(device->request, PK_CMD_PULSE_ENGINE_V2, 0xFE, device->PEv2.newMotionBufferEntries, device->PEv2.PulseEngineEnabled & 0x0F, 0);

    // Copy buffer
    memcpy(&device->request[8], device->PEv2.MotionBuffer, 56);

    // Send request
    if (SendRequest(device) != PK_OK) return PK_ERR_TRANSFER;

    device->PEv2.motionBufferEntriesAccepted = device->response[2];

    // Decode status
    PK_PEv2_DecodeStatus(device);

    return PK_OK;
}

/**
 * @brief Transfer a large block of 8-bit motion data.
 *
 * Uses the multipart packet command `PK_CMD_MULTIPART_PACKET`
 * with subcommand `0xFF`. `param2` specifies entry count and
 * `param3` the axis enable mask. The payload contains 448 bytes
 * of buffer data stored in `multiPartData`.
 *
 * @param device Target device.
 * @return PK_OK on success or error code from transfer.
 */
int32_t PK_PEv2_BufferFillLarge(sPoKeysDevice * device)
{
    if (device == NULL) return PK_ERR_NOT_CONNECTED;	

    // Create request
    CreateRequest(device->request, 0xB0, 0, 0xFF, device->PEv2.newMotionBufferEntries, device->PEv2.PulseEngineEnabled & 0x0F);

    // Copy buffer
    memcpy(device->multiPartData, device->PEv2.MotionBuffer, 448);

    // Send request
	if (SendRequest_multiPart(device) != PK_OK) return PK_ERR_TRANSFER;

    device->PEv2.motionBufferEntriesAccepted = device->response[2];

    // Decode status
    PK_PEv2_DecodeStatus(device);

    return PK_OK;
}

/**
 * @brief Large transfer of 16-bit motion data.
 *
 * Same as @ref PK_PEv2_BufferFillLarge but using subcommand
 * `0xFE` for 16-bit entries.
 *
 * @param device Target device.
 * @return PK_OK on success or communication error.
 */
int32_t PK_PEv2_BufferFillLarge_16(sPoKeysDevice * device)
{
    if (device == NULL) return PK_ERR_NOT_CONNECTED;	

    // Create request
    CreateRequest(device->request, 0xB0, 0, 0xFE, device->PEv2.newMotionBufferEntries, device->PEv2.PulseEngineEnabled & 0x0F);

    // Copy buffer
    memcpy(device->multiPartData, device->PEv2.MotionBuffer, 448);

    // Send request
	if (SendRequest_multiPart(device) != PK_OK) return PK_ERR_TRANSFER;

    device->PEv2.motionBufferEntriesAccepted = device->response[2];

    // Decode status
    PK_PEv2_DecodeStatus(device);

    return PK_OK;
}

/**
 * @brief Clear the motion buffer in the device.
 *
 * Command `PEV2_CMD_CLEAR_BUFFER` (0xF0) resets the internal
 * motion queue.
 *
 * @param device Target device.
 * @return Result of SendRequest.
 */
int32_t PK_PEv2_BufferClear(sPoKeysDevice * device)
{
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    // Create request
    CreateRequest(device->request, PK_CMD_PULSE_ENGINE_V2, PEV2_CMD_CLEAR_BUFFER, 0, 0, 0);
    // Send request
    return SendRequest(device);
}

/**
 * @brief Reboot the pulse engine firmware.
 *
 * Issues `PEV2_CMD_REBOOT` (0x05).
 *
 * @param device Target device.
 * @return Result of SendRequest.
 */
int32_t PK_PEv2_PulseEngineReboot(sPoKeysDevice * device)
{
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    // Create request
    CreateRequest(device->request, PK_CMD_PULSE_ENGINE_V2, PEV2_CMD_REBOOT, 0, 0, 0);
    // Send request
    return SendRequest(device);
}

/**
 * @brief Begin the homing sequence.
 *
 * Command `PEV2_CMD_START_HOMING` (0x21). `param1` holds the axis
 * mask and bytes 8-39 carry home offset values for each axis.
 *
 * @param device Target device with offsets stored in
 *               `PEv2.HomeOffsets`.
 * @return Result of SendRequest.
 */
int32_t PK_PEv2_HomingStart(sPoKeysDevice * device)
{
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    // Create request
    CreateRequest(device->request, PK_CMD_PULSE_ENGINE_V2, PEV2_CMD_START_HOMING, device->PEv2.HomingStartMaskSetup, 0, 0);

    //memcpy(&device->request[8], device->PEv2.HomeOffsets, 8 * 4);
    for (int i = 0; i < 8; i++) {
        int32_t val = device->PEv2.HomeOffsets[i];  // Read from volatile pin
        device->request[8 + i * 4 + 0] = (uint8_t)(val & 0xFF);
        device->request[8 + i * 4 + 1] = (uint8_t)((val >> 8) & 0xFF);
        device->request[8 + i * 4 + 2] = (uint8_t)((val >> 16) & 0xFF);
        device->request[8 + i * 4 + 3] = (uint8_t)((val >> 24) & 0xFF);
    }
    // Send request
    return SendRequest(device);
}

/**
 * @brief Finish the homing procedure.
 *
 * Sends `PEV2_CMD_FINISH_HOMING` (0x22). `param1` sets the new
 * engine state after homing and `param2` should be 1 as required
 * by the protocol.
 *
 * @param device PoKeys device instance.
 * @return Result of SendRequest.
 */
int32_t PK_PEv2_HomingFinish(sPoKeysDevice * device)
{
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    // Create request
    CreateRequest(device->request, PK_CMD_PULSE_ENGINE_V2, PEV2_CMD_FINISH_HOMING, device->PEv2.PulseEngineStateSetup, 1, 0);
    // Send request
    return SendRequest(device);
}

// Start the probing procedure.
// ProbeMaxPosition defines the maximum position in position ticks where probing error will be thrown
// ProbeSpeed defines the probing speed (1 = max speed)
// ProbeInput defines the extenal input (values 1-8) or PoKeys pin (0-based Pin ID + 9)
// ProbeInputPolarity defines the polarity of the probe signal
/**
 * @brief Start a probing move.
 *
 * Command `PEV2_CMD_START_PROBING` (0x23) uses `param1` as axis
 * selection mask. Bytes 8-39 hold maximum probe positions, byte
 * 40 holds probe speed and bytes 44-45 define probe input and
 * polarity.
 *
 * @param device Device configured with probe parameters.
 * @return Result of SendRequest.
 */
int32_t PK_PEv2_ProbingStart(sPoKeysDevice * device)
{
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    // Create request
    CreateRequest(device->request, PK_CMD_PULSE_ENGINE_V2, PEV2_CMD_START_PROBING, device->PEv2.ProbeStartMaskSetup, 0, 0);

    //memcpy(&device->request[8], device->PEv2.ProbeMaxPosition, 8 * 4);
    for (int i = 0; i < 8; i++) {
        int32_t val = device->PEv2.ProbeMaxPosition[i];  // Read from volatile pin
        device->request[8 + i * 4 + 0] = (uint8_t)(val & 0xFF);
        device->request[8 + i * 4 + 1] = (uint8_t)((val >> 8) & 0xFF);
        device->request[8 + i * 4 + 2] = (uint8_t)((val >> 16) & 0xFF);
        device->request[8 + i * 4 + 3] = (uint8_t)((val >> 24) & 0xFF);
    }

    *(float*)(&device->request[40]) = device->PEv2.ProbeSpeed;
    device->request[44] = device->PEv2.ProbeInput;
    device->request[45] = device->PEv2.ProbeInputPolarity;

    // Send request
    return SendRequest(device);
}

/**
 * @brief Start probing with an external controller.
 *
 * Uses `PEV2_CMD_START_PROBING` with `param2=1` for the hybrid
 * probing mode. Only the probe input number and polarity are
 * sent in bytes 44-45.
 *
 * @param device Device configured for probing.
 * @return Result of SendRequest.
 */
int32_t PK_PEv2_ProbingHybridStart(sPoKeysDevice * device)
{
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    // Create request
    CreateRequest(device->request, PK_CMD_PULSE_ENGINE_V2, PEV2_CMD_START_PROBING, 0, 1, 0);

    device->request[44] = device->PEv2.ProbeInput;
    device->request[45] = device->PEv2.ProbeInputPolarity;

    // Send request
    return SendRequest(device);
}

/**
 * @brief Complete the probing cycle and read the result.
 *
 * Command `PEV2_CMD_FINISH_PROBING` (0x24) returns the probe
 * position in bytes 8-39 and status bits in byte 40. The engine
 * state is also reset to STOPPED.
 *
 * @param device Device receiving the probe result.
 * @return PK_OK when the response is valid or an error code.
 */
int32_t PK_PEv2_ProbingFinish(sPoKeysDevice * device)
{
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    // Create request
    CreateRequest(device->request, PK_CMD_PULSE_ENGINE_V2, PEV2_CMD_FINISH_PROBING, 0, 0, 0);

    // Send request
    if (SendRequest(device) != PK_OK) return PK_ERR_TRANSFER;

    // Copy the probe result position
    //memcpy(device->PEv2.ProbePosition, &device->response[8], 8*4);
    for (int i = 0; i < 8; i++) {
        device->PEv2.ProbePosition[i] =
            ((int32_t)device->response[8 + i * 4]) |
            ((int32_t)device->response[8 + i * 4 + 1] << 8) |
            ((int32_t)device->response[8 + i * 4 + 2] << 16) |
            ((int32_t)device->response[8 + i * 4 + 3] << 24);
    }
    
    device->PEv2.ProbeStatus = device->response[40];

    return PK_OK;
}
/**
 * @brief Finish probing without changing engine state.
 *
 * Uses `PEV2_CMD_FINISH_PROBING` (0x24) with `param1=1`.
 * The probe result is returned like in @ref PK_PEv2_ProbingFinish
 * but the engine state is not reset.
 *
 * @param device Device receiving the result.
 * @return PK_OK if successful or error code.
 */
int32_t PK_PEv2_ProbingFinishSimple(sPoKeysDevice * device)
{
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    // Create request
    CreateRequest(device->request, PK_CMD_PULSE_ENGINE_V2, PEV2_CMD_FINISH_PROBING, 1, 0, 0);

    // Send request
    if (SendRequest(device) != PK_OK) return PK_ERR_TRANSFER;

    // Copy the probe result position
   // memcpy(device->PEv2.ProbePosition, &device->response[8], 8*4);
    for (int i = 0; i < 8; i++) {
        device->PEv2.ProbePosition[i] =
            ((int32_t)device->response[8 + i * 4]) |
            ((int32_t)device->response[8 + i * 4 + 1] << 8) |
            ((int32_t)device->response[8 + i * 4 + 2] << 16) |
            ((int32_t)device->response[8 + i * 4 + 3] << 24);
    }
    return PK_OK;
}

/**
 * @brief Prepare the spindle threading trigger.
 *
 * Executes `PEV2_CMD_PREPARE_TRIGGER` (0x30) which readies the
 * device for the next threading operation.
 *
 * @param device Target device.
 * @return PK_OK on success or error code from communication.
 */
int32_t PK_PEv2_ThreadingPrepareForTrigger(sPoKeysDevice * device)
{
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    // Create request
    CreateRequest(device->request, PK_CMD_PULSE_ENGINE_V2, PEV2_CMD_PREPARE_TRIGGER, 0, 0, 0);

    // Send request
    if (SendRequest(device) != PK_OK) return PK_ERR_TRANSFER;

	return PK_OK;
}

/**
 * @brief Force trigger ready state.
 *
 * Command `PEV2_CMD_FORCE_TRIGGER_READY` (0x31) immediately
 * sets the trigger as ready for threading.
 *
 * @param device PoKeys device.
 * @return PK_OK on success or error code.
 */
int32_t PK_PEv2_ThreadingForceTriggerReady(sPoKeysDevice * device)
{
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    // Create request
    CreateRequest(device->request, PK_CMD_PULSE_ENGINE_V2, PEV2_CMD_FORCE_TRIGGER_READY, 0, 0, 0);

    // Send request
    if (SendRequest(device) != PK_OK) return PK_ERR_TRANSFER;

	return PK_OK;
}

/**
 * @brief Arm the threading trigger.
 *
 * Executes `PEV2_CMD_ARM_TRIGGER` (0x32) which waits for the
 * configured spindle index before executing motion.
 *
 * @param device Target device.
 * @return PK_OK if the command is sent successfully.
 */
int32_t PK_PEv2_ThreadingTrigger(sPoKeysDevice * device)
{
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    // Create request
    CreateRequest(device->request, PK_CMD_PULSE_ENGINE_V2, PEV2_CMD_ARM_TRIGGER, 0, 0, 0);

    // Send request
    if (SendRequest(device) != PK_OK) return PK_ERR_TRANSFER;

	return PK_OK;
}

/**
 * @brief Release the threading trigger.
 *
 * Sends `PEV2_CMD_RELEASE_TRIGGER` (0x33) which aborts waiting
 * for the spindle index.
 *
 * @param device Target device.
 * @return Result of SendRequest.
 */
int32_t PK_PEv2_ThreadingRelease(sPoKeysDevice * device)
{
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    // Create request
    CreateRequest(device->request, PK_CMD_PULSE_ENGINE_V2, PEV2_CMD_RELEASE_TRIGGER, 0, 0, 0);

    // Send request
    if (SendRequest(device) != PK_OK) return PK_ERR_TRANSFER;

	return PK_OK;
}

/**
 * @brief Cancel a prepared threading operation.
 *
 * Command `PEV2_CMD_CANCEL_THREADING` (0x34) clears any
 * pending trigger preparation.
 *
 * @param device PoKeys device.
 * @return Result of SendRequest.
 */
int32_t PK_PEv2_ThreadingCancel(sPoKeysDevice * device)
{
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    // Create request
    CreateRequest(device->request, PK_CMD_PULSE_ENGINE_V2, PEV2_CMD_CANCEL_THREADING, 0, 0, 0);

    // Send request
    if (SendRequest(device) != PK_OK) return PK_ERR_TRANSFER;

	return PK_OK;
}

/**
 * @brief Query the status of a threading operation.
 *
 * Sends `PEV2_CMD_GET_THREADING_STATUS` (0x35) which returns
 * various trigger flags along with spindle diagnostic data in
 * the response bytes 8-63.
 *
 * @param device Target device.
 * @return PK_OK on success or error code.
 */
int32_t PK_PEv2_ThreadingStatusGet(sPoKeysDevice * device)
{
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    // Create request
    CreateRequest(device->request, PK_CMD_PULSE_ENGINE_V2, PEV2_CMD_GET_THREADING_STATUS, 0, 0, 0);

    // Send request
    if (SendRequest(device) != PK_OK) return PK_ERR_TRANSFER;


	device->PEv2.TriggerPreparing = device->response[8];
	device->PEv2.TriggerPrepared = device->response[9];
	device->PEv2.TriggerPending = device->response[10];
	device->PEv2.TriggerActive = device->response[11];

	device->PEv2.SpindleSpeedEstimate = *(int32_t*)(device->response + 12);
	device->PEv2.SpindlePositionError = *(int32_t*)(device->response + 16);
	device->PEv2.SpindleRPM =			*(int32_t*)(device->response + 20);

	device->PEv2.TriggerIngnoredAxisMask = device->response[24];

	device->PEv2.spindleIndexCounter  = *(int32_t*)(device->response + 25);

	//memcpy(device->PEv2.DebugValues, device->response + 29, 35);
    for (int i = 0; i < 35; i++) {
        device->PEv2.DebugValues[i] = device->response[29 + i];
    }
    
	return PK_OK;

}

/**
 * @brief Configure spindle threading parameters.
 *
 * Uses `PEV2_CMD_SET_THREADING_PARAMS` (0x36). The request
 * provides sensor mode and filter settings in the payload while
 * `TriggerIngnoredAxisMask` is stored in byte 20.
 *
 * @param device Target device.
 * @param sensorMode Sensor mode selector.
 * @param ticksPerRevolution Encoder ticks per spindle revolution.
 * @param tagetSpindleRPM Desired spindle speed.
 * @param filterGainSpeed Speed filter gain.
 * @param filterGainPosition Position filter gain.
 * @return PK_OK on success or error code.
 */
int32_t PK_PEv2_ThreadingSetup(sPoKeysDevice * device, uint8_t sensorMode, uint16_t ticksPerRevolution, uint16_t tagetSpindleRPM, uint16_t filterGainSpeed, uint16_t filterGainPosition)
{
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    // Create request
    CreateRequest(device->request, PK_CMD_PULSE_ENGINE_V2, PEV2_CMD_SET_THREADING_PARAMS, 0, 0, 0);

	device->request[8] = sensorMode;
	*(uint16_t*)(device->request + 12) = ticksPerRevolution;
	*(uint16_t*)(device->request + 14) = tagetSpindleRPM;
	*(uint16_t*)(device->request + 16) = filterGainSpeed;
	*(uint16_t*)(device->request + 18) = filterGainPosition;
	device->request[20] = device->PEv2.TriggerIngnoredAxisMask;

    // Send request
    if (SendRequest(device) != PK_OK) return PK_ERR_TRANSFER;

	return PK_OK;
}

/**
 * @brief Retrieve backlash compensation parameters.
 *
 * Sends `PEV2_CMD_GET_BACKLASH_SETTINGS` (0x40) and parses the
 * backlash width, acceleration and register values for each axis
 * from the response bytes.
 *
 * @param device Target device.
 * @return PK_OK on success or communication error.
 */
int32_t PK_PEv2_BacklashCompensationSettings_Get(sPoKeysDevice * device)
{
	int32_t i;
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    // Create request
    CreateRequest(device->request, PK_CMD_PULSE_ENGINE_V2, PEV2_CMD_GET_BACKLASH_SETTINGS, 0, 0, 0);

	// Send request
    if (SendRequest(device) != PK_OK) return PK_ERR_TRANSFER;

	for (i = 0; i < 8; i++)
	{
		device->PEv2.BacklashWidth[i] = *(uint16_t*)(device->response + 8 + i * 4);
		device->PEv2.BacklashAcceleration[i] = device->response[10 + i * 4];
		device->PEv2.BacklashRegister[i] = *(int16_t*)(device->response + 40 + i * 2);
	}
	device->PEv2.BacklashCompensationEnabled = device->response[3];
	device->PEv2.BacklashCompensationMaxSpeed = device->response[4];
			
	return PK_OK;
}

/**
 * @brief Write backlash compensation parameters.
 *
 * Command `PEV2_CMD_SET_BACKLASH_SETTINGS` (0x41) sends the
 * same structure as returned by the getter to configure
 * backlash for each axis.
 *
 * @param device Target device with values stored in the
 *               PEv2 structure.
 * @return PK_OK on success or error code.
 */
int32_t PK_PEv2_BacklashCompensationSettings_Set(sPoKeysDevice * device)
{
	int32_t i;
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

	// Create request
    CreateRequest(device->request, PK_CMD_PULSE_ENGINE_V2, PEV2_CMD_SET_BACKLASH_SETTINGS, device->PEv2.BacklashCompensationEnabled, device->PEv2.BacklashCompensationMaxSpeed, 0);

	for (i = 0; i < 8; i++)
	{
		*(uint16_t*)(device->request + 8 + i * 4) = device->PEv2.BacklashWidth[i];
		device->request[10 + i * 4] = device->PEv2.BacklashAcceleration[i];
	}		

	// Send request
    if (SendRequest(device) != PK_OK) return PK_ERR_TRANSFER;

	return PK_OK;
}


/**
 * @brief Configure synchronized PWM output.
 *
 * Command `PEV2_CMD_SETUP_SYNCED_PWM` (0x0A). The request
 * enables or disables synchronized PWM generation based on
 * an axis position.
 *
 * @param device Target device.
 * @param enabled Non-zero to enable feature.
 * @param srcAxis Source axis index (0-based).
 * @param dstPWMChannel PWM output channel.
 * @return PK_OK on success or error code.
 */
int32_t PK_PEv2_SyncedPWMSetup(sPoKeysDevice * device, uint8_t enabled, uint8_t srcAxis, uint8_t dstPWMChannel)
{    
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    // Create request
    CreateRequest(device->request, PK_CMD_PULSE_ENGINE_V2, PEV2_CMD_SETUP_SYNCED_PWM, enabled, srcAxis, dstPWMChannel);

    // Send request
    if (SendRequest(device) != PK_OK) return PK_ERR_TRANSFER;

    return PK_OK;
}

/**
 * @brief Configure synchronized digital outputs.
 *
 * Command `PEV2_CMD_SETUP_SYNCED_DIGITAL` (0x0B) maps a group of
 * outputs to follow the position of an axis.
 *
 * @param device Device with mapping information stored in
 *               `SyncFastOutputsMapping` and axis ID in
 *               `SyncFastOutputsAxisID`.
 * @return PK_OK on success or error code.
 */
int32_t PK_PEv2_SyncOutputsSetup(sPoKeysDevice * device)
{
	if (device == NULL) return PK_ERR_NOT_CONNECTED;

	// Create request
    CreateRequest(device->request, PK_CMD_PULSE_ENGINE_V2, PEV2_CMD_SETUP_SYNCED_DIGITAL, device->PEv2.SyncFastOutputsAxisID > 0, device->PEv2.SyncFastOutputsAxisID - 1, 0);
	memcpy(device->request + 8, device->PEv2.SyncFastOutputsMapping, 8);

	// Send request
	if (SendRequest(device) != PK_OK) return PK_ERR_TRANSFER;

	return PK_OK;
}

/**
 * @brief Get configuration for external PoStep drivers.
 *
 * Command `PEV2_CMD_SETUP_DRIVER_COMM` (0x50) retrieves driver
 * type, I2C address and update flags for each axis.
 *
 * @param device Target device.
 * @return PK_OK on success or communication error.
 */
int32_t PK_PoStep_ConfigurationGet(sPoKeysDevice * device)
{
	int32_t i;
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    // Create request
    CreateRequest(device->request, PK_CMD_PULSE_ENGINE_V2, PEV2_CMD_SETUP_DRIVER_COMM, 0, 0, 0);

	// Send request
    if (SendRequest(device) != PK_OK) return PK_ERR_TRANSFER;

	// Parse settings for each axis
	for (i = 0; i < 8; i++)
	{
		device->PoSteps.drivers[i].DriverType = device->response[8 + i*3];
		device->PoSteps.drivers[i].AddressI2C = device->response[9 + i*3];
		device->PoSteps.drivers[i].UpdateConfig = device->response[10 + i*3];
	}
	device->PoSteps.EnablePoStepCommunication = device->response[4];
			
	return PK_OK;
}

/**
 * @brief Configure external PoStep drivers.
 *
 * Sends `PEV2_CMD_SETUP_DRIVER_COMM` (0x50) with `param1=0x10`
 * followed by driver type, address and update flags for each
 * axis.
 *
 * @param device Device holding configuration in `PoSteps`.
 * @return PK_OK on success or error code.
 */
int32_t PK_PoStep_ConfigurationSet(sPoKeysDevice * device)
{
	int32_t i;
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

	// Create request
    CreateRequest(device->request, PK_CMD_PULSE_ENGINE_V2, PEV2_CMD_SETUP_DRIVER_COMM, 0x10, device->PoSteps.EnablePoStepCommunication, 0);

	// Insert settings for each axis
	for (i = 0; i < 8; i++)
	{
		device->request[8 + i * 3] = device->PoSteps.drivers[i].DriverType;
		device->request[9 + i * 3] = device->PoSteps.drivers[i].AddressI2C;
		device->request[10 + i * 3] = device->PoSteps.drivers[i].UpdateConfig;
	}

	// Send request
    if (SendRequest(device) != PK_OK) return PK_ERR_TRANSFER;

	// Parse settings for each axis
	for (i = 0; i < 8; i++)
	{
		device->PoSteps.drivers[i].DriverType = device->response[8 + i*3];
		device->PoSteps.drivers[i].AddressI2C = device->response[9 + i*3];
		device->PoSteps.drivers[i].UpdateConfig = device->response[10 + i*3];
	}
	device->PoSteps.EnablePoStepCommunication = device->response[4];
	
	return PK_OK;
}


/**
 * @brief Read status information from PoStep drivers.
 *
 * Executes `PEV2_CMD_GET_DRIVER_STATUS` (0x51) which returns
 * voltage, temperature and diagnostic bits for each driver.
 *
 * @param device Target device.
 * @return PK_OK or error code.
 */
int32_t PK_PoStep_StatusGet(sPoKeysDevice * device)
{
	int32_t i;
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    // Create request
    CreateRequest(device->request, PK_CMD_PULSE_ENGINE_V2, PEV2_CMD_GET_DRIVER_STATUS, 0, 0, 0);

	// Send request
    if (SendRequest(device) != PK_OK) return PK_ERR_TRANSFER;

	// Parse settings for each axis
	for (i = 0; i < 8; i++)
	{
		device->PoSteps.drivers[i].SupplyVoltage = device->response[8 + i*6];
		device->PoSteps.drivers[i].Temperature = device->response[9 + i*6];
		device->PoSteps.drivers[i].InputStatus = device->response[10 + i*6];
		device->PoSteps.drivers[i].DriverStatus = device->response[11 + i*6];
		device->PoSteps.drivers[i].FaultStatus = device->response[12 + i*6];
		device->PoSteps.drivers[i].UpdateState = device->response[13 + i*6];
	}
			
	return PK_OK;
}


/**
 * @brief Read PoStep driver current and mode settings.
 *
 * Uses commands `PEV2_CMD_DRIVER_CURRENT_PARAMS` (0x52) and
 * `PEV2_CMD_DRIVER_MODE_PARAMS` (0x53) to obtain current limits,
 * driver mode, microstepping and temperature limits.
 *
 * @param device Target device.
 * @return PK_OK on success or communication error.
 */
int32_t PK_PoStep_DriverConfigurationGet(sPoKeysDevice * device)
{
	int32_t i;
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

	// Current settings
    // Create request
    CreateRequest(device->request, PK_CMD_PULSE_ENGINE_V2, PEV2_CMD_DRIVER_CURRENT_PARAMS, 0, 0, 0);

	// Send request
    if (SendRequest(device) != PK_OK) return PK_ERR_TRANSFER;

	// Parse settings for each axis
	for (i = 0; i < 8; i++)
	{
		device->PoSteps.drivers[i].Current_FS = *(uint16_t*)(device->response + 8 + i*6);
		device->PoSteps.drivers[i].Current_Idle = *(uint16_t*)(device->response + 10 + i*6);
		device->PoSteps.drivers[i].Current_Overheat = *(uint16_t*)(device->response + 12 + i*6);
	}

	// Mode settings
    // Create request
    CreateRequest(device->request, PK_CMD_PULSE_ENGINE_V2, PEV2_CMD_DRIVER_MODE_PARAMS, 0, 0, 0);

	// Send request
    if (SendRequest(device) != PK_OK) return PK_ERR_TRANSFER;

	// Parse settings for each axis
	for (i = 0; i < 8; i++)
	{
		device->PoSteps.drivers[i].DriverMode = device->response[8 + i*6];
		device->PoSteps.drivers[i].StepMode = device->response[9 + i*6];
		device->PoSteps.drivers[i].TemperatureLimit = device->response[10 + i*6];
	}
		
	return PK_OK;
}


/**
 * @brief Write PoStep driver current and mode parameters.
 *
 * Sends `PEV2_CMD_DRIVER_CURRENT_PARAMS` and
 * `PEV2_CMD_DRIVER_MODE_PARAMS` with `param1=0x10` to update
 * driver settings for all axes.
 *
 * @param device Device containing new driver parameters.
 * @return PK_OK on success or error code.
 */
int32_t PK_PoStep_DriverConfigurationSet(sPoKeysDevice * device)
{
	int32_t i;
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

	// Current
	// Create request
    CreateRequest(device->request, PK_CMD_PULSE_ENGINE_V2, PEV2_CMD_DRIVER_CURRENT_PARAMS, 0x10, 0, 0);

	// Insert settings for each axis
	for (i = 0; i < 8; i++)
	{
		*(uint16_t*)(device->request + 8 + i*6) = device->PoSteps.drivers[i].Current_FS;
		*(uint16_t*)(device->request + 10 + i*6) = device->PoSteps.drivers[i].Current_Idle;
		*(uint16_t*)(device->request + 12 + i*6) = device->PoSteps.drivers[i].Current_Overheat;
	}

	// Send request
    if (SendRequest(device) != PK_OK) return PK_ERR_TRANSFER;

	// Parse settings for each axis
	for (i = 0; i < 8; i++)
	{
		device->PoSteps.drivers[i].Current_FS = *(uint16_t*)(device->response + 8 + i*6);
		device->PoSteps.drivers[i].Current_Idle = *(uint16_t*)(device->response + 10 + i*6);
		device->PoSteps.drivers[i].Current_Overheat = *(uint16_t*)(device->response + 12 + i*6);
	}

	// Modes
	// Create request
    CreateRequest(device->request, PK_CMD_PULSE_ENGINE_V2, PEV2_CMD_DRIVER_MODE_PARAMS, 0x10, 0, 0);

	// Insert settings for each axis
	for (i = 0; i < 8; i++)
	{
		device->request[8 + i * 6] = device->PoSteps.drivers[i].DriverMode;
		device->request[9 + i * 6] = device->PoSteps.drivers[i].StepMode;
		device->request[10 + i * 6] = device->PoSteps.drivers[i].TemperatureLimit;
		device->request[11 + i * 6] = 0;
		device->request[12 + i * 6] = 0;
		device->request[13 + i * 6] = 0;
	}

	// Send request
    if (SendRequest(device) != PK_OK) return PK_ERR_TRANSFER;

	// Parse settings for each axis
	for (i = 0; i < 8; i++)
	{
		device->PoSteps.drivers[i].DriverMode = device->response[8 + i*6];
		device->PoSteps.drivers[i].StepMode = device->response[9 + i*6];
		device->PoSteps.drivers[i].TemperatureLimit = device->response[10 + i*6];
	}
	
	return PK_OK;
}

/**
 * @brief Read configuration of internal motor drivers.
 *
 * Command `PEV2_CMD_GET_INTERNAL_DRIVERS` (0x18) returns step
 * mode and current settings for four integrated drivers.
 *
 * @param device Target device.
 * @return PK_OK on success or error code.
 */
int32_t PK_PEv2_InternalDriversConfigurationGet(sPoKeysDevice * device)
{
    sPoKeysPEv2 * pe;
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    // Send request
    CreateRequest(device->request, PK_CMD_PULSE_ENGINE_V2, PEV2_CMD_GET_INTERNAL_DRIVERS, 0, 0, 0);
    if (SendRequest(device) != PK_OK) return PK_ERR_TRANSFER;

    // Pointer to PEv2 structure for better code readability
    pe = &device->PEv2;

    for (int32_t i = 0; i < 4; i++) {
        pe->InternalDriverStepConfig[i] = device->response[8 + i*2];
        pe->InternalDriverCurrentConfig[i] = device->response[9 + i*2];
    }
    return PK_OK;
}

/**
 * @brief Set configuration of internal motor drivers.
 *
 * Uses `PEV2_CMD_SET_INTERNAL_DRIVERS` (0x19) with step and
 * current settings for the four integrated drivers placed in the
 * request payload.
 *
 * @param device Device providing new settings via `PEv2` fields.
 * @return Result of SendRequest.
 */
int32_t PK_PEv2_InternalDriversConfigurationSet(sPoKeysDevice * device)
{
    sPoKeysPEv2 * pe;
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    // Create request
    CreateRequest(device->request, PK_CMD_PULSE_ENGINE_V2, PEV2_CMD_SET_INTERNAL_DRIVERS, 0, 0, 0);

    // Pointer to PEv2 structure for better code readability
    pe = &device->PEv2;

    for (int32_t i = 0; i < 4; i++) {
        device->request[8 + i*2] = pe->InternalDriverStepConfig[i];
        device->request[9 + i*2] = pe->InternalDriverCurrentConfig[i];
    }

    // Send request
    return SendRequest(device);
}
