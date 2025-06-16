/*

Copyright (C) 2020 Matev� Bo�nak (matevz@poscope.com)

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

#include <stdlib.h>
#include "PoKeysLibHal.h"
#include "PoKeysLibCore.h"
#ifdef WIN32
    #include "windows.h"
   // #include "Winsock.h"
    #include "conio.h"
    #include <iphlpapi.h>
#else
    #include <unistd.h>
    #include <stdlib.h>
    #include <sys/socket.h>
    #include <netdb.h>
    #include <netinet/in.h>
    #include <net/if.h>
    #include <arpa/inet.h>
    #include <sys/ioctl.h>
#endif
//#include "PoKeysLibDevicePoKeys57Industrial.h"
#include "PoKeysLibCoreSockets.h"
#include "string.h"
#include "stdio.h"
#include <sys/stat.h>

/**
 * @file PoKeysLibDevicePoKeys57Industrial.c
 * @brief Support for the PoKeys57Industrial device.
 *
 * Contains helper routines for discovery, connection and full I/O
 * exchange using command set 0x3F as defined in the protocol
 * specification.
 */

#ifdef DEBUG_PoKeysLibSockets
    #define debug_printf printf
#else
    #define debug_printf (void)
#endif


/**
 * @brief Search the network for a PoKeys57Industrial board.
 *
 * A broadcast UDP packet is sent to port 20055 and the function waits for a
 * reply.  When a PoKeys57Industrial responds, its network parameters and
 * firmware information are stored into @p device.
 *
 * The discovery mechanism is described in the "Device discovery" section of the
 * PoKeys protocol specification.
 *
 * @param device  Structure that receives the discovered device details.
 * @param timeout Timeout in milliseconds to wait for a response.
 * @return Number of detected boards.
 */
int32_t PK57i_SearchDevice(sPoKeysNetworkDeviceSummary* device, uint32_t timeout);


/**
 * @brief Establish communication with a PoKeys57Industrial device.
 *
 * The helper first performs a UDP discovery (@ref PK57i_SearchDevice). If a
 * board replies, ::PK_ConnectToNetworkDevice is used to open a network
 * connection.  Should that fail, the function falls back to USB enumeration and
 * connects to the first device reporting hardware ID ``45``.
 *
 * On success a newly allocated ::sPoKeys57Industrial handle is returned.  The
 * internal @c deviceStructure field points to the underlying ::sPoKeysDevice
 * instance.
 *
 * @return Pointer to the device structure or @c NULL when no device was found.
 */
sPoKeys57Industrial* PK57i_Connect(void)
{
    sPoKeysDevice* devPtr = NULL;
    sPoKeys57Industrial * devPtr57i;
    sPoKeysNetworkDeviceSummary targetDev;
    uint32_t numDev, i;
    sPoKeysDevice * tmpDevice = NULL;

    // Primarily, use UDP to connect to the device
    if (PK57i_SearchDevice(&targetDev, 100) > 0)
    {
        // Ok, device was detected, connect to it
        devPtr = PK_ConnectToNetworkDevice(&targetDev);
    }

    if (devPtr == NULL)
    {
        // Network connection failed, try USB
        numDev = PK_EnumerateUSBDevices();
        for (i = 0; i < numDev; i++)
        {
            tmpDevice = PK_ConnectToDevice(i);
            if (tmpDevice != NULL && tmpDevice->DeviceData.DeviceType == 45)
            {
                // Correct device type
                devPtr = tmpDevice;
                break;
            } else
            {
                PK_DisconnectDevice(tmpDevice);
            }
        }
    }

    // Device not found
    if (devPtr == NULL)
    {
        return NULL;
    }

    // Allocate the memory for device structure
    devPtr57i = (sPoKeys57Industrial*)malloc(sizeof(sPoKeys57Industrial));
    memset(devPtr57i, 0, sizeof(sPoKeys57Industrial));

    devPtr57i->deviceStructure = (void*)devPtr;
    return devPtr57i;
}

/**
 * @brief Close the connection opened by PK57i_Connect().
 *
 * The underlying ::sPoKeysDevice is released using ::PK_DisconnectDevice and
 * the memory allocated for the ::sPoKeys57Industrial handle is freed.
 *
 * @param device Handle previously returned by PK57i_Connect().
 */
void PK57i_Disconnect(sPoKeys57Industrial * device)
{
    if (device == NULL) return;
    PK_DisconnectDevice((sPoKeysDevice*)device->deviceStructure);

    // Free the memory
    free(device);
}


/**
 * @brief Exchange I/O data with the PoKeys57Industrial device.
 *
 * This function builds the *Full I/O* command (ID `0x3F`) and transmits it to
 * the device.  Digital and analog outputs from the structure are encoded into
 * the request while input values and fault flags are parsed from the reply.
 *
 * The protocol format of command `0x3F` is documented in the PoKeys protocol
 * specification, see section "Full I/O command (device specific)".
 *
 * @param device      Handle returned by PK57i_Connect().
 * @param resetFaults Set to 1 to clear latched fault flags on the device.
 * @return PK_OK on success or an error code from SendRequest().
 */
int32_t PK57i_Update(sPoKeys57Industrial* device, uint8_t resetFaults)
{
    uint32_t i;
    uint8_t tmpEnable;
    sPoKeysDevice * devPtr;
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    devPtr = (sPoKeysDevice*)device->deviceStructure;

    // Digital outputs enable bits
    tmpEnable = 0;
    for (i = 0; i < 8; i++)
    {
        if (device->digitalOutputsEnable[i] > 0) tmpEnable |= (1<<i);
    }

    // Prepare the request
    CreateRequest(devPtr->request, 0x3F, 0, 0, tmpEnable, resetFaults);

    // Digital outputs first
    devPtr->request[8] = 0;
    for (i = 0; i < 8; i++)
    {
        if (device->digitalOutputs[i] > 0) devPtr->request[8] |= (1<<i);
    }
    // Analog outputs
    for (i = 0; i < 8; i++)
    {
        *((uint16_t*)&devPtr->request[16 + i*2]) = device->analogOutputs[i];
    }
    if (SendRequest(devPtr) != PK_OK) return PK_ERR_TRANSFER;

    // Parse fault statuses
    for (i = 0; i < 8; i++)
    {
        device->digitalOutputsFault[i] = (devPtr->response[4] & (1<<i)) ? 1 : 0;
        device->analogOutputsFault[i] = (devPtr->response[5] & (1<<i)) ? 1 : 0;
    }

    // Get digital inputs
    for (i = 0; i < 8; i++)
    {
        device->digitalInputs[i] = (devPtr->response[8] & (1<<i)) ? 1 : 0;
    }

    // And analog inputs in the end
    for (i = 0; i < 8; i++)
    {
        device->analogInputs[i] = *((uint16_t*)&devPtr->response[16 + i*2]);
    }

    return PK_OK;
}

/**
 * @brief Perform UDP discovery for a PoKeys57Industrial device.
 *
 * An empty broadcast packet is sent to port 20055 and responses are parsed
 * according to the "Discovery packet format" described in the PoKeys protocol
 * specification. Only devices reporting hardware ID 45 are accepted.
 *
 * @param device  Structure that receives the discovered device details.
 * @param timeout Timeout in milliseconds to wait for replies.
 * @return Number of detected boards.
 */
int32_t PK57i_SearchDevice(sPoKeysNetworkDeviceSummary* device, uint32_t timeout)
{
    //Broadcast the message
    int32_t t; // 100 ms timeout
#ifdef WIN32
    struct sockaddr_in remoteEP;
    SOCKET txSocket;
#else
    int txSocket;
    struct sockaddr_in remoteEP;
    fd_set fds;
    struct timeval stimeout;
#endif
    int32_t UDPbroadcast = 1;
    int32_t status = 0;
    int32_t BufLen = 0;
    char SendBuf[1];
    uint32_t * addr;
    uint32_t * addrPtr;
    uint32_t a;
    uint8_t rcvbuf[500];
    uint32_t nrOfDetectedBoards = 0;

    t = timeout;
#ifdef WIN32
    if (InitWinsock() != 0)
    {
        debug_printf("InitWinsock error!");
        return 0;
    }
#endif

    // Create socket for discovery packet
    if ((txSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        debug_printf("Error creating socket\n");
        return 0;
    }
    if (setsockopt( txSocket, SOL_SOCKET, SO_BROADCAST, (char *)&UDPbroadcast, sizeof UDPbroadcast) == -1)
    {
        debug_printf("Error setting broadcast option\n");
        return 0;
    }

#ifdef WIN32
    if (setsockopt( txSocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&t, sizeof(t)) == -1)
    {
        debug_printf("Error setting SO_RCVTIMEO option\n");
        return 0;
    }
    if (setsockopt( txSocket, SOL_SOCKET, SO_SNDTIMEO, (char *)&t, sizeof(t)) == -1)
    {
        debug_printf("Error setting SO_SNDTIMEO option\n");
        return 0;
    }
#endif

    addr = GetBroadcastAddresses();
    addrPtr = addr;

    while(*addr)
    {
        a = *addr;

        // Send discovery request...
        remoteEP.sin_family = AF_INET;
        remoteEP.sin_port = htons(20055);
        remoteEP.sin_addr.s_addr = a; // inet_addr("255.255.255.255");

#ifdef WIN32
            if (sendto(txSocket, SendBuf, BufLen, 0, (SOCKADDR *)&remoteEP, sizeof(remoteEP)) == -1)
#else
            if (sendto(txSocket, SendBuf, BufLen, 0, (struct sockaddr *)&remoteEP, sizeof(remoteEP)) == -1)
#endif
        {
            debug_printf("Error sending discovery request to %s\n", inet_ntoa(remoteEP.sin_addr));
            free(addrPtr);
        }
#ifdef WIN32
            closesocket(txSocket);
            WSACleanup();
#else
            close(txSocket);
#endif
            return 0;
        }
        addr++;
    }
   // free(addrPtr);

#ifdef WIN32

#else
    // Wait for discovery response...
    stimeout.tv_sec = 0;
    stimeout.tv_usec = 1000 * timeout;

    FD_ZERO(&fds);
    FD_SET(txSocket, &fds);
#endif


    while(1)
    {
        // Receive response from devices
#ifdef WIN32
#else
        if (select(txSocket + 1, &fds, NULL, NULL, &stimeout) < 0)
        {
            debug_printf("Error in select...\n");
            close(txSocket);
            return 0;
        }

        if (FD_ISSET(txSocket, &fds) == 0) break;
#endif

        status = recv(txSocket, (char *)rcvbuf, sizeof(rcvbuf), 0);

        if (status < 0)
        {
            break;
        }

        // Get IP address and receive message
        if (status > 0)
        {
            if (status == 19 && rcvbuf[18] == 45)
            {
                // Parse the response
                debug_printf("Received response from the PoKeys57Industrial device...\n");

                // Save the device info
                device->SerialNumber = (int)rcvbuf[14] + ((int)rcvbuf[15] << 8) + ((int)rcvbuf[16] << 16) + ((int)rcvbuf[17] << 24);
                device->FirmwareVersionMajor = rcvbuf[3];
                device->FirmwareVersionMinor = rcvbuf[4];
                memcpy(device->IPaddress, rcvbuf + 5, 4);
                device->DHCP = rcvbuf[9];
                memcpy(device->hostIP, rcvbuf + 10, 4);
                device->HWtype = rcvbuf[18];
                nrOfDetectedBoards++;
                status = 0;
                break;
            }
        }
    }

#ifdef WIN32
    closesocket(txSocket);
#else
    close(txSocket);
#endif

    return nrOfDetectedBoards;
}
