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

#include "hidapi.h"
#include <stdio.h>
#include <string.h>
#include "PoKeysLibHal.h"

#ifndef __POKEYSLIBCORE
	#define __POKEYSLIBCORE

	void InitializeNewDevice(sPoKeysDevice* device);
	void CleanDevice(sPoKeysDevice* device);
	void PK_DisconnectNetworkDevice(sPoKeysDevice* device);

        /**
         * Compose a command packet.
         *
         * @param request 64 byte buffer to populate.
         * @param type    Command ID (PK_CMD_* constant).
         * @param param1  First parameter.
         * @param param2  Second parameter.
         * @param param3  Third parameter.
         * @param param4  Fourth parameter.
         * @return PK_OK on success or PK_ERR_NOT_CONNECTED when @p request is NULL.
         */
        int32_t CreateRequest(uint8_t * request, uint8_t type, uint8_t param1, uint8_t param2, uint8_t param3, uint8_t param4);
        uint8_t getChecksum(uint8_t * data);
        /** Send a prepared request and wait for the response. */
        int32_t SendRequest(sPoKeysDevice * device);
        /** Send a multipart (::PK_CMD_MULTIPART_PACKET) request. */
        int32_t SendRequest_multiPart(sPoKeysDevice* device);
        /** Send a request without waiting for a reply. */
    int32_t SendRequest_NoResponse(sPoKeysDevice* device);


	int32_t PK_FillPWMPinNumbers(sPoKeysDevice * device);


	#ifdef POKEYSLIB_USE_LIBUSB
		void * ConnectToFastUSBInterface(int32_t serial);
		void DisconnectFromFastUSBInterface(void * device);
		int32_t SendRequestFastUSB(sPoKeysDevice* device);
		int32_t SendRequestFastUSB_NoResponse(sPoKeysDevice* device);
		int32_t SendRequestFastUSB_multiPart(sPoKeysDevice* device);
	#else	
		#define ConnectToFastUSBInterface(device) NULL;
		#define DisconnectFromFastUSBInterface(device) ;
		#define SendRequestFastUSB(device) ;
		#define SendRequestFastUSB_multiPart(device) ;
	#endif
#endif
