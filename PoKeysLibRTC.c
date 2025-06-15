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
/**
 * @file PoKeysLibRTC.c
 * @brief Real-time clock access helpers (command 0x83).
 *
 * Functions read and write the RTC registers via
 * PK_CMD_RTC_SETTINGS.
 */
/**
 * @brief Read the real-time clock values (command 0x83/0x00).
 *
 * Fills the RTC structure of @p device with the current date and time.
 *
 * @param device Pointer to an opened PoKeys device.
 * @return PK_OK on success or a PK_ERR code on failure.
 */

int32_t PK_RTCGet(sPoKeysDevice* device)
{
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    CreateRequest(device->request, PK_CMD_RTC_SETTINGS, 0x00, 0, 0, 0);
    if (SendRequest(device) != PK_OK) return PK_ERR_TRANSFER;

    *device->RTC.SEC = device->response[8];
    *device->RTC.MIN = device->response[9];
    *device->RTC.HOUR = device->response[10];
    *device->RTC.DOW = device->response[11];
    *device->RTC.DOM = device->response[12];
    *device->RTC.MONTH = device->response[13];
    *device->RTC.DOY = (uint16_t)device->response[14]
                    + ((uint16_t)device->response[15] << 8);
    *device->RTC.YEAR = (uint16_t)device->response[16]
                    + ((uint16_t)device->response[17] << 8);

    return PK_OK;
}


/**
 * @brief Set the real-time clock (command 0x83/0x10).
 *
 * Writes the values from the device RTC structure to the
 * PoKeys module.
 *
 * @param device Pointer to an opened PoKeys device.
 * @return PK_OK on success or a PK_ERR code on failure.
 */
int32_t PK_RTCSet(sPoKeysDevice* device)
{
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    CreateRequest(device->request, PK_CMD_RTC_SETTINGS, 0x10, 0, 0, 0);
    device->request[8]  = (uint8_t)(*device->RTC.SEC);
    device->request[9]  = (uint8_t)(*device->RTC.MIN);
    device->request[10] = (uint8_t)(*device->RTC.HOUR);
    device->request[11] = (uint8_t)(*device->RTC.DOW);
    device->request[12] = (uint8_t)(*device->RTC.DOM);
    device->request[13] = (uint8_t)(*device->RTC.MONTH);
    device->request[14] = (uint8_t)(*device->RTC.DOY & 0xFF);
    device->request[15] = (uint8_t)((*device->RTC.DOY >> 8) & 0xFF);
    device->request[16] = (uint8_t)(*device->RTC.YEAR & 0xFF);
    device->request[17] = (uint8_t)((*device->RTC.YEAR >> 8) & 0xFF);
    if (SendRequest(device) != PK_OK) return PK_ERR_TRANSFER;

    return PK_OK;
}
