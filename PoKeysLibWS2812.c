/*

Copyright (C) 2017 Matev� Bo�nak (matevz@poscope.com)

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

int32_t PK_WS2812_Update(sPoKeysDevice* device, uint16_t LEDcount, uint8_t updateFlag)
{
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    // Configure WS2812 driver
    CreateRequest(device->request, PK_CMD_WS2812_CONTROL, 0x00, LEDcount & 0xFF, LEDcount >> 8, updateFlag);
    if (SendRequest(device) != PK_OK) return PK_ERR_TRANSFER;
    return PK_OK;
}

int32_t PK_WS2812_SendLEDdataEx(sPoKeysDevice* device, uint32_t * LEDdata, uint16_t LEDoffset, uint16_t startLED, uint8_t LEDcount)
{
    uint32_t i;
    if (device == NULL) return PK_ERR_NOT_CONNECTED;
    if (LEDcount > 18) return PK_ERR_PARAMETER;

    // Pack LED data
    CreateRequest(device->request, PK_CMD_WS2812_CONTROL, 0x10, startLED & 0xFF, startLED >> 8, LEDcount);
    for (i = 0; i < LEDcount; i++)
    {
        memcpy(device->request + 8 + i*3, &LEDdata[i+LEDoffset], 3);
    }

    // Second checksum covers LED data only (bytes 8-62)
    device->request[63] = 0;
    for (i = 8; i < 63; i++)
    {
        device->request[63] += device->request[i];
    }
    // SendRequest_NoResponse will append packet header and first checksum (bytes 0-6)
    if (SendRequest_NoResponse(device) != PK_OK) return PK_ERR_TRANSFER;
    return PK_OK;
}

int32_t PK_WS2812_SendLEDdata(sPoKeysDevice* device, uint32_t * LEDdata, uint16_t startLED, uint8_t LEDcount)
{
    int32_t errCode = PK_OK;
    uint16_t offset = 0;

    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    while (LEDcount > 0)
    {
        if (LEDcount >= 18)
        {
            errCode = PK_WS2812_SendLEDdataEx(device, LEDdata, offset, startLED + offset, 18);
            if (errCode != PK_OK) return errCode;

            offset += 18;
            LEDcount -= 18;
        }
        else
        {
            errCode = PK_WS2812_SendLEDdataEx(device, LEDdata, offset, startLED + offset, LEDcount);
            if (errCode != PK_OK) return errCode;
            break;
        }
    }
    return PK_OK;
}
