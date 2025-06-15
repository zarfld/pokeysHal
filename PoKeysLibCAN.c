/*

Copyright (C) 2016 Matev� Bo�nak (matevz@poscope.com)

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

/**
 * @file PoKeysLibCAN.c
 * @brief Implementation of CAN bus operations (command 0x86).
 *
 * This module provides functions for configuring and using the
 * CAN interface of PoKeys devices via the PK_CMD_CAN_OPERATIONS
 * command set.
 */

/**
 * @brief Configure the CAN interface (subcommand 0x01).
 *
 * Sends PK_CMD_CAN_OPERATIONS with subcommand 0x01 to enable the
 * CAN controller and set the desired bitrate.
 *
 * @param device  Pointer to an opened PoKeys device.
 * @param bitrate CAN bus bitrate in bits per second.
 * @return PK_OK on success or PK_ERR_* on failure.
 */
int32_t PK_CANConfigure(sPoKeysDevice* device, uint32_t bitrate)
{
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    // Configure UART
    CreateRequest(device->request, PK_CMD_CAN_OPERATIONS, POCAN_CMD_ENABLE, 0, 0, 0);
    *(uint32_t*)(device->request + 8) = bitrate;
    if (SendRequest(device) != PK_OK) return PK_ERR_TRANSFER;
    return PK_OK;
}

/**
 * @brief Register a CAN filter (subcommand 0x10).
 *
 * The filter selects which CAN identifiers are received by the
 * device.
 *
 * @param device Pointer to an opened PoKeys device.
 * @param format Identifier format: 0 for standard, 1 for extended.
 * @param CANid  Identifier value to filter for.
 * @return PK_OK on success or PK_ERR_* on failure.
 */
int32_t PK_CANRegisterFilter(sPoKeysDevice* device, uint8_t format, uint32_t CANid)
{
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    // Configure UART
    CreateRequest(device->request, PK_CMD_CAN_OPERATIONS, 0x10, format, 0, 0);
    *(uint32_t*)(device->request + 8) = CANid;
    if (SendRequest(device) != PK_OK) return PK_ERR_TRANSFER;
    return PK_OK;
}

/**
 * @brief Transmit a CAN message (subcommand 0x20).
 *
 * The message contents are taken from @p msg and sent to the device.
 *
 * @param device Pointer to an opened PoKeys device.
 * @param msg    CAN message to send.
 * @return PK_OK on success or PK_ERR_* on failure.
 */
int32_t PK_CANWrite(sPoKeysDevice* device, sPoKeysCANmsg * msg)
{
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    // Configure UART
    CreateRequest(device->request, PK_CMD_CAN_OPERATIONS, 0x20, 0, 0, 0);
    memcpy(device->request + 8, msg, sizeof(sPoKeysCANmsg));
    if (SendRequest(device) != PK_OK) return PK_ERR_TRANSFER;
    return PK_OK;
}

/**
 * @brief Receive one CAN message (subcommand 0x31).
 *
 * The returned status byte indicates message availability.
 * When non-zero, the received message is stored in @p msg.
 *
 * @param device Pointer to an opened PoKeys device.
 * @param msg    Buffer for the received message.
 * @param status Receives 1 if a message was read.
 * @return PK_OK on success or PK_ERR_* on failure.
 */
int32_t PK_CANRead(sPoKeysDevice* device, sPoKeysCANmsg * msg, uint8_t * status)
{
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    // Configure UART
    CreateRequest(device->request, PK_CMD_CAN_OPERATIONS, 0x31, 0, 0, 0);
    if (SendRequest(device) != PK_OK) return PK_ERR_TRANSFER;

    *status = device->response[3];
    if (*status)
        memcpy(msg, device->response + 8, sizeof(sPoKeysCANmsg));
    return PK_OK;
}


/**
 * @brief Flush the CAN receive queue (subcommand 0x32).
 *
 * @param device Pointer to an opened PoKeys device.
 * @return PK_OK on success or PK_ERR_* on failure.
 */
int32_t PK_CANFlush(sPoKeysDevice* device)
{
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    // Configure UART
    CreateRequest(device->request, PK_CMD_CAN_OPERATIONS, 0x32, 0, 0, 0);
    if (SendRequest(device) != PK_OK) return PK_ERR_TRANSFER;
    return PK_OK;
}
