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
 * @file PoKeysLibSPI.c
 * @brief SPI communication helpers (command 0xE5).
 *
 * Provides basic configuration and data transfer routines
 * for the SPI interface on PoKeys devices.
 */

/**
 * @brief Configure the SPI interface of a PoKeys device (command 0xE5/0x01).
 *
 * Issues PK_CMD_SPI_COMMUNICATION with subcommand 0x01 to set
 * the SPI clock prescaler and frame format.
 *
 * @param device      Pointer to an initialised device structure.
 * @param prescaler   SPI clock prescaler value sent to the device.
 * @param frameFormat Frame configuration (mode, bit length etc.).
 *
 * @return ::PK_OK on success or a negative ::PK_ERR code on failure.
 */
int32_t PK_SPIConfigure(sPoKeysDevice * device, uint8_t prescaler, uint8_t frameFormat)
{
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    // Configure SPI
    CreateRequest(device->request, PK_CMD_SPI_COMMUNICATION, 0x01, prescaler, frameFormat, 0);
    if (SendRequest(device) != PK_OK) return PK_ERR_TRANSFER;
    return PK_OK;
}

/**
 * @brief Transmit data over the SPI interface (command 0xE5/0x10).
 *
 * Sends a block of up to 55 bytes to the selected chip on the PoKeys
 * device. The chip select line is asserted automatically by the device.
 *
 * @param device       Pointer to an initialised PoKeys device structure.
 * @param buffer       Data buffer containing bytes to transmit.
 * @param iDataLength  Number of bytes to send (max 55).
 * @param pinCS        Chip select pin identifier on the PoKeys device.
 *
 * @return ::PK_OK on success or a negative ::PK_ERR code when the transfer fails.
 */
int32_t PK_SPIWrite(sPoKeysDevice * device, uint8_t * buffer, uint8_t iDataLength, uint8_t pinCS)
{
    uint8_t i;
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    if (iDataLength > 55) iDataLength = 55;

    CreateRequest(device->request, PK_CMD_SPI_COMMUNICATION, 0x10, iDataLength, pinCS, 0);
    for (i = 0; i < iDataLength; i++)
    {
        device->request[8+i] = buffer[i];
    }
    if (SendRequest(device) != PK_OK) return PK_ERR_TRANSFER;

    if (device->response[3] == 1) return PK_OK; else return PK_ERR_GENERIC;
}

/**
 * @brief Receive data from the SPI interface (command 0xE5/0x20).
 *
 * Issues a read request for the specified number of bytes.
 * The returned values are copied into the provided buffer on success.
 *
 * @param device      Pointer to an initialised PoKeys device structure.
 * @param buffer      Destination buffer for the received bytes.
 * @param iDataLength Number of bytes to read (maximum 55).
 *
 * @return ::PK_OK if the data was read successfully, otherwise a negative
 *         ::PK_ERR code.
 */
int32_t PK_SPIRead(sPoKeysDevice * device, uint8_t * buffer, uint8_t iDataLength)
{
    uint8_t i;
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    if (iDataLength > 55) iDataLength = 55;

    CreateRequest(device->request, PK_CMD_SPI_COMMUNICATION, 0x20, iDataLength, 0, 0);
    if (SendRequest(device) != PK_OK) return PK_ERR_TRANSFER;

    if (device->response[3] == 1)
    {
        for (i = 0; i < iDataLength; i++)
        {
            buffer[i] = device->response[8+i];
        }
        return PK_OK;
    } else return PK_ERR_GENERIC;
}

