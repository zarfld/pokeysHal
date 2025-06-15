/*

Copyright (C) 2014 Matev� Bo�nak (matevz@poscope.com)

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
 * @brief Enable or disable the 1-wire bus.
 *
 * Issues command 0xDC with operation code 0x01 to enable or
 * 0x00 to disable the bus as defined in the protocol specification
 * ("1-wire settings and communication").
 *
 * @param device  Pointer to an opened PoKeys device.
 * @param activated  Non-zero to activate the bus, zero to deactivate.
 * @return PK_OK on success, PK_ERR_NOT_CONNECTED when @p device is NULL or
 *         an error returned by SendRequest (for example PK_ERR_TRANSFER).
 */
int32_t PK_1WireStatusSet(sPoKeysDevice* device, uint8_t activated)
{
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    CreateRequest(device->request, PK_CMD_ONEWIRE_COMMUNICATION, activated, 0, 0, 0);
    return SendRequest(device);
}

/**
 * @brief Query the 1-wire activation status.
 *
 * Sends command 0xDC/0x11 which returns the current activation
 * state in the response.
 *
 * @param device      Pointer to an opened PoKeys device.
 * @param activated   Destination for the activation flag (1 when enabled).
 * @return PK_OK on success, PK_ERR_NOT_CONNECTED if @p device is NULL or
 *         PK_ERR_TRANSFER on communication failure.
 */
int32_t PK_1WireStatusGet(sPoKeysDevice* device, uint8_t* activated)
{
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    CreateRequest(device->request, PK_CMD_ONEWIRE_COMMUNICATION, 0x11, 0, 0, 0);
    if (SendRequest(device) != PK_OK) return PK_ERR_TRANSFER;

    *activated = device->response[3];
    return PK_OK;
}

/**
 * @brief Start a reset/write/read transaction on the 1-wire bus.
 *
 * Executes command 0xDC/0x10 ("Start Reset, Write and Read process").
 * Up to 16 bytes may be written and read in a single transaction.
 *
 * @param device     Pointer to an opened PoKeys device.
 * @param WriteCount Number of bytes to write (clamped to 16).
 * @param ReadCount  Number of bytes to read (clamped to 16).
 * @param data       Buffer containing bytes to write.
 * @return PK_OK when the command is sent successfully, PK_ERR_NOT_CONNECTED
 *         if @p device is NULL or an error from SendRequest.
 */
int32_t PK_1WireWriteReadStart(sPoKeysDevice* device, uint8_t WriteCount, uint8_t ReadCount, uint8_t * data)
{
    uint32_t i;
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    if (WriteCount > 16) WriteCount = 16;
    if (ReadCount > 16) ReadCount = 16;

    CreateRequest(device->request, PK_CMD_ONEWIRE_COMMUNICATION, 0x10, WriteCount, ReadCount, 0);
    for (i = 0; i < WriteCount; i++)
    {
        device->request[8+i] = data[i];
    }
    return SendRequest(device);
}

/**
 * @brief Retrieve data from the previous 1-wire read operation.
 *
 * Sends command 0xDC/0x11 to obtain the read status and any
 * returned bytes.
 *
 * @param device      Pointer to an opened PoKeys device.
 * @param readStatus  Operation status: 1 when data is available.
 * @param ReadCount   Receives the number of bytes returned.
 * @param data        Buffer that receives the read bytes (up to 16).
 * @return PK_OK on success, PK_ERR_NOT_CONNECTED if @p device is NULL,
 *         PK_ERR_TRANSFER on communication failure or PK_ERR_PARAMETER
 *         if the returned byte count is invalid.
 */
int32_t PK_1WireReadStatusGet(sPoKeysDevice* device, uint8_t * readStatus, uint8_t * ReadCount, uint8_t * data)
{
    uint32_t i;
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    CreateRequest(device->request, PK_CMD_ONEWIRE_COMMUNICATION, 0x11, 0, 0, 0);
    if (SendRequest(device) != PK_OK) return PK_ERR_TRANSFER;

    *readStatus = device->response[8];
    if (*readStatus == 1)
    {
        *ReadCount = device->response[9];

        if (*ReadCount > 16)
        {
            *readStatus = 0;
            return PK_ERR_PARAMETER;
        }

        for (i = 0; i < *ReadCount; i++)
        {
            data[i] = device->response[10 + i];
        }
    }
    return PK_OK;
}



/**
 * @brief Start a 1-wire transaction on a specific pin.
 *
 * Same as PK_1WireWriteReadStart() but allows selecting the
 * 1-wire pin. Uses command 0xDC/0x10 with the pin ID in the
 * third parameter byte.
 *
 * @param device     Pointer to an opened PoKeys device.
 * @param pinID      PoKeys pin used for 1-wire communication.
 * @param WriteCount Number of bytes to write (clamped to 16).
 * @param ReadCount  Number of bytes to read (clamped to 16).
 * @param data       Buffer of bytes to send.
 * @return PK_OK when the command is sent, PK_ERR_NOT_CONNECTED if
 *         @p device is NULL or an error from SendRequest.
 */
int32_t PK_1WireWriteReadStartEx(sPoKeysDevice* device, uint8_t pinID, uint8_t WriteCount, uint8_t ReadCount, uint8_t * data)
{
    uint32_t i;
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    if (WriteCount > 16) WriteCount = 16;
    if (ReadCount > 16) ReadCount = 16;

    CreateRequest(device->request, PK_CMD_ONEWIRE_COMMUNICATION, 0x10, WriteCount, ReadCount, pinID);
    for (i = 0; i < WriteCount; i++)
    {
        device->request[8+i] = data[i];
    }
    return SendRequest(device);
}


/**
 * @brief Begin scanning the 1-wire bus for devices.
 *
 * Issues command 0xDC/0x20 with the selected pin ID.
 *
 * @param device  Pointer to an opened PoKeys device.
 * @param pinID   PoKeys pin used for the scan.
 * @return PK_OK on success, PK_ERR_NOT_CONNECTED if @p device is NULL
 *         or an error from SendRequest.
 */
int32_t PK_1WireBusScanStart(sPoKeysDevice* device, uint8_t pinID)
{
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    CreateRequest(device->request, PK_CMD_ONEWIRE_COMMUNICATION, 0x20, pinID, 0, 0);
    return SendRequest(device);
}

/**
 * @brief Obtain status and results of a 1-wire bus scan.
 *
 * Command 0xDC/0x21 returns the current scanning state and the
 * discovered device ROM code when available.
 *
 * @param device          Pointer to an opened PoKeys device.
 * @param operationStatus Receives operation status byte.
 * @param scanResult      Receives scan progress flags.
 * @param deviceROM       Buffer for the 8 byte device ROM.
 * @return PK_OK on success, PK_ERR_NOT_CONNECTED if @p device is NULL or
 *         PK_ERR_TRANSFER on communication failure.
 */
int32_t PK_1WireBusScanGetResults(sPoKeysDevice* device, uint8_t * operationStatus, uint8_t * scanResult, uint8_t * deviceROM)
{
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    CreateRequest(device->request, PK_CMD_ONEWIRE_COMMUNICATION, 0x21, 0, 0, 0);
    if (SendRequest(device) != PK_OK) return PK_ERR_TRANSFER;

    *operationStatus = device->response[8];
    *scanResult = device->response[9];
    memcpy(deviceROM, device->response + 10, 8);

    return PK_OK;
}



/**
 * @brief Continue a previously started 1-wire bus scan.
 *
 * Sends command 0xDC/0x22.
 *
 * @param device Pointer to an opened PoKeys device.
 * @return PK_OK on success, PK_ERR_NOT_CONNECTED if @p device is NULL
 *         or an error from SendRequest.
 */
int32_t PK_1WireBusScanContinue(sPoKeysDevice* device)
{
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    CreateRequest(device->request, PK_CMD_ONEWIRE_COMMUNICATION, 0x22, 0, 0, 0);
    return SendRequest(device);
}


/**
 * @brief Stop an ongoing 1-wire bus scan.
 *
 * Uses command 0xDC/0x23 to terminate the scan procedure.
 *
 * @param device Pointer to an opened PoKeys device.
 * @return PK_OK on success, PK_ERR_NOT_CONNECTED if @p device is NULL
 *         or an error from SendRequest.
 */
int32_t PK_1WireBusScanStop(sPoKeysDevice* device)
{
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    CreateRequest(device->request, PK_CMD_ONEWIRE_COMMUNICATION, 0x23, 0, 0, 0);
    return SendRequest(device);
}


