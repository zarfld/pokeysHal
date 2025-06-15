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
 * @brief Enable or disable the I\2C interface.
 *
 * The PoKeys device keeps the I\2C engine enabled at all times.  This helper
 * only sends the activation flag for compatibility with older firmware.
 *
 * @param device     Handle to an opened PoKeys device.
 * @param activated  Non-zero to enable the bus, zero to disable it.
 * @return ::PK_OK on success or ::PK_ERR_NOT_CONNECTED if @p device is NULL.
 */
int32_t PK_I2CSetStatus(sPoKeysDevice* device, uint8_t activated)
{
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    CreateRequest(device->request, PK_CMD_I2C_COMMUNICATION, activated, 0, 0, 0);
    return SendRequest(device);
}

/**
 * @brief Query whether the I\2C bus is enabled.
 *
 * @param device     Handle to an opened PoKeys device.
 * @param activated  Pointer that receives the activation state.
 * @return ::PK_OK on success, ::PK_ERR_NOT_CONNECTED if @p device is NULL or
 *         ::PK_ERR_TRANSFER on communication failure.
 */
int32_t PK_I2CGetStatus(sPoKeysDevice* device, uint8_t* activated)
{
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    CreateRequest(device->request, PK_CMD_I2C_COMMUNICATION, 0x02, 0, 0, 0);
    if (SendRequest(device) != PK_OK) return PK_ERR_TRANSFER;

    *activated = device->response[3];
    return PK_OK;
}

/**
 * @brief Initiate an I\2C write transaction.
 *
 * Up to 32 bytes from @p buffer are copied and sent to the device using
 * ::PK_CMD_I2C_COMMUNICATION with subcommand @c 0x10.
 *
 * @param device      Handle to an opened PoKeys device.
 * @param address     7-bit I\2C slave address.
 * @param buffer      Data to transmit.
 * @param iDataLength Number of bytes to send (0–32).
 * @return ::PK_OK on command acceptance or ::PK_ERR_NOT_CONNECTED when
 *         @p device is NULL.
 */
int32_t PK_I2CWriteStart(sPoKeysDevice* device, uint8_t address, uint8_t* buffer, uint8_t iDataLength)
{
    uint32_t i;
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    if (iDataLength > 32) iDataLength = 32;

    CreateRequest(device->request, PK_CMD_I2C_COMMUNICATION, 0x10, address, iDataLength, 0);
    for (i = 0; i < iDataLength; i++)
    {
        device->request[8+i] = buffer[i];
    }
    return SendRequest(device);
}

/**
 * @brief Write data and queue a subsequent read from the same address.
 *
 * This combines a write and read phase.  Up to 32 bytes are written from
 * @p buffer and the device is instructed to read @p iDataLengthRead bytes
 * afterwards. The read data is obtained later with ::PK_I2CReadStatusGet().
 *
 * @param device            Handle to an opened PoKeys device.
 * @param address           I\2C slave address.
 * @param buffer            Data to write.
 * @param iDataLengthWrite  Number of bytes to write (0–32).
 * @param iDataLengthRead   Number of bytes expected to be read back.
 * @return ::PK_OK on command acceptance or ::PK_ERR_NOT_CONNECTED when
 *         @p device is NULL.
 */
int32_t PK_I2CWriteAndReadStart(sPoKeysDevice* device, uint8_t address, uint8_t* buffer, uint8_t iDataLengthWrite, uint8_t iDataLengthRead)
{
    uint32_t i;
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    if (iDataLengthWrite > 32) iDataLengthWrite = 32;

    CreateRequest(device->request, PK_CMD_I2C_COMMUNICATION, 0x10, address, iDataLengthWrite, iDataLengthRead);
    for (i = 0; i < iDataLengthWrite; i++)
    {
        device->request[8+i] = buffer[i];
    }
    return SendRequest(device);
}

/**
 * @brief Retrieve the status of the last I\2C write operation.
 *
 * @param device Handle to an opened PoKeys device.
 * @param status Receives the result (see ::ePK_I2C_STATUS).
 * @return ::PK_OK on success, ::PK_ERR_NOT_CONNECTED if @p device is NULL or
 *         ::PK_ERR_TRANSFER on communication failure.
 */
int32_t PK_I2CWriteStatusGet(sPoKeysDevice* device, uint8_t* status)
{
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    CreateRequest(device->request, PK_CMD_I2C_COMMUNICATION, 0x11, 0, 0, 0);
    if (SendRequest(device) != PK_OK) return PK_ERR_TRANSFER;

    *status = device->response[3];
    return PK_OK;
}

/**
 * @brief Initiate an I\2C read transaction.
 *
 * The device is commanded to read @p iDataLength bytes from the slave at
 * @p address.  The received data is later obtained with
 * ::PK_I2CReadStatusGet().
 *
 * @param device      Handle to an opened PoKeys device.
 * @param address     I\2C slave address.
 * @param iDataLength Number of bytes to read (0–32).
 * @return ::PK_OK on command acceptance or ::PK_ERR_NOT_CONNECTED when
 *         @p device is NULL.
 */
int32_t PK_I2CReadStart(sPoKeysDevice* device, uint8_t address, uint8_t iDataLength)
{
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    if (iDataLength > 32) iDataLength = 32;

    CreateRequest(device->request, PK_CMD_I2C_COMMUNICATION, 0x20, address, iDataLength, 0);
    return SendRequest(device);
}

/**
 * @brief Obtain the result of an I\2C read command.
 *
 * After a read has been started with PK_I2CReadStart() or
 * PK_I2CWriteAndReadStart(), this function polls the device for completion and
 * copies the returned bytes into @p buffer.
 *
 * @param device           Handle to an opened PoKeys device.
 * @param status           Receives the operation status (see ::ePK_I2C_STATUS).
 * @param iReadBytes       Number of bytes actually read on success.
 * @param buffer           Destination buffer for the received data.
 * @param iMaxBufferLength Size of @p buffer in bytes.
 * @return ::PK_OK on success, ::PK_ERR_NOT_CONNECTED if @p device is NULL,
 *         ::PK_ERR_TRANSFER on communication failure or ::PK_ERR_GENERIC when
 *         the device reports more data than fits into 32 bytes.
 */
int32_t PK_I2CReadStatusGet(sPoKeysDevice* device, uint8_t* status, uint8_t* iReadBytes, uint8_t* buffer, uint8_t iMaxBufferLength)
{
    uint32_t i;

    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    CreateRequest(device->request, PK_CMD_I2C_COMMUNICATION, 0x21, 0, 0, 0);
    if (SendRequest(device) != PK_OK) return PK_ERR_TRANSFER;

    *status = device->response[3];
    *iReadBytes = 0;

    if (*status == PK_I2C_STAT_COMPLETE)
    {
        *iReadBytes = device->response[9];
        if (*iReadBytes > 32)
        {
            *status = PK_I2C_STAT_ERR;
            return PK_ERR_GENERIC;
        }

        for (i = 0; i < iMaxBufferLength && i < *iReadBytes; i++)
        {
            buffer[i] = device->response[10+i];
        }
    }

    return PK_OK;
}

/**
 * @brief Begin scanning the I\2C bus for devices.
 *
 * The device searches all 7-bit addresses. Completion is checked with
 * ::PK_I2CBusScanGetResults().
 *
 * @param device Handle to an opened PoKeys device.
 * @return ::PK_OK on command acceptance or ::PK_ERR_NOT_CONNECTED when
 *         @p device is NULL.
 */
int32_t PK_I2CBusScanStart(sPoKeysDevice* device)
{
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    CreateRequest(device->request, PK_CMD_I2C_COMMUNICATION, 0x30, 0, 0, 0);
    return SendRequest(device);
}

/**
 * @brief Obtain the results of an I\2C bus scan.
 *
 * After PK_I2CBusScanStart() completes, this helper reads the scan bitmap and
 * stores device presence information into @p presentDevices.
 *
 * @param device         Handle to an opened PoKeys device.
 * @param status         Receives the scan status (see ::ePK_I2C_STATUS).
 * @param presentDevices Array that receives one byte per address indicating
 *                       whether a device acknowledged.
 * @param iMaxDevices    Maximum number of addresses to report (up to 128).
 * @return ::PK_OK on success, ::PK_ERR_NOT_CONNECTED if @p device is NULL or
 *         ::PK_ERR_TRANSFER on communication failure.
 */
int32_t PK_I2CBusScanGetResults(sPoKeysDevice* device, uint8_t* status, uint8_t* presentDevices, uint8_t iMaxDevices)
{
    uint32_t i;

    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    if (iMaxDevices > 128) iMaxDevices = 128;

    CreateRequest(device->request, PK_CMD_I2C_COMMUNICATION, 0x31, 0, 0, 0);
    if (SendRequest(device) != PK_OK) return PK_ERR_TRANSFER;

    *status = device->response[3];

    if (*status == PK_I2C_STAT_COMPLETE)
    {
        for (i = 0; i < iMaxDevices; i++)
        {
            presentDevices[i] = ((device->response[9 + i / 8] & (1 << (i % 8))) > 0) ? PK_I2C_STAT_OK : PK_I2C_STAT_ERR;
        }
    }

    return PK_OK;
}
