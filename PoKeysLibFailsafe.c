/*

Copyright (C) 2018 Matev� Bo�nak (matevz@poscope.com)

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


/**
 * @file PoKeysLibFailsafe.c
 * @brief Failsafe configuration helpers (command 0x81).
 *
 * The routines in this file read and write the failsafe settings
 * as defined in the PoKeys protocol specification.  They map the
 * device state to the ::sPK_FailsafeSettings structure.
 */


/**
 * @brief Read failsafe configuration from the device (command 0x81).
 *
 * Populates ::sPK_FailsafeSettings in @p device with the timeout and
 * peripheral states defined by the PoKeys failsafe command.
 *
 * @param device Pointer to an opened device structure.
 * @return ::PK_OK on success or a ::PK_ERR code on failure.
 */
int32_t PK_FailsafeSettingsGet(sPoKeysDevice* device)
{
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    CreateRequest(device->request, 0x81, 0, 0, 0, 0);
    if (SendRequest(device) != PK_OK) return PK_ERR_TRANSFER;

    // Parse the settings
    device->failsafeSettings.bFailSafeEnabled = device->response[3];
    device->failsafeSettings.bFailSafePeripherals = device->response[4];

    memcpy(device->failsafeSettings.bFailSafeIO, device->response + 8, 7);
    memcpy(device->failsafeSettings.bFailSafePoExtBus, device->response + 15, 10);
    memcpy(device->failsafeSettings.bFailSafePWM, device->response + 25, 6);

    return PK_OK;
}


/**
 * @brief Write failsafe configuration to the device (command 0x81).
 *
 * The function sends the content of ::sPK_FailsafeSettings stored in
 * @p device to the PoKeys unit using operation code 0x01.
 *
 * @param device Pointer to an opened device structure.
 * @return Result of SendRequest() or ::PK_ERR_NOT_CONNECTED when
 *         @p device is NULL.
 */
int32_t PK_FailsafeSettingsSet(sPoKeysDevice* device)
{
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    CreateRequest(device->request, 0x81, 0x01, 0, 0, 0);

    // Insert the settings
    device->request[3] = device->failsafeSettings.bFailSafeEnabled;
    device->request[4] = device->failsafeSettings.bFailSafePeripherals;

    memcpy(device->request + 8, device->failsafeSettings.bFailSafeIO, 7);
    memcpy(device->request + 15, device->failsafeSettings.bFailSafePoExtBus, 10);
    memcpy(device->request + 25, device->failsafeSettings.bFailSafePWM, 6);

    return SendRequest(device);
}
