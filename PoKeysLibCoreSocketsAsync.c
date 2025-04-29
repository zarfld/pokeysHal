#include "PoKeysLibHal.h"
#include "PoKeysLibCoreSocketsAsync.h"
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include "hal.h"  // for hal_malloc


extern int getChecksum(const uint8_t *buffer);
extern void InitializeNewDevice(sPoKeysDevice *device);
extern void debug_printf(const char *fmt, ...);
extern uint64_t get_current_time_us(void);

PoKeysDiscoveryContext discoveryCtx;

uint32_t *GetBroadcastAddressesAsync()
{
    // Reserve room for 100 addresses (99 + terminating zero)
    uint32_t *list = hal_malloc(sizeof(uint32_t) * 100);
    if (!list) return NULL;

    uint32_t *ptr = list;

    struct ifaddrs *ifaddr, *ifa;
    if (getifaddrs(&ifaddr) == -1)
    {
        *ptr = 0;
        return list;
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (!ifa->ifa_addr || !(ifa->ifa_flags & IFF_BROADCAST))
            continue;

        if (ifa->ifa_addr->sa_family == AF_INET && ifa->ifa_broadaddr)
        {
            struct sockaddr_in *broad = (struct sockaddr_in *)ifa->ifa_broadaddr;
            if (broad->sin_addr.s_addr != 0)
            {
                *(ptr++) = broad->sin_addr.s_addr;
            }
        }
    }

    *ptr = 0; // terminate list
    freeifaddrs(ifaddr);
    return list;
}

/**
 * @brief Starts asynchronous search for PoKeys devices on the network.
 *
 * This function sends a UDP broadcast discovery packet to all available broadcast addresses
 * and initializes an asynchronous search context. It must be followed by regular calls
 * to PK_SearchNetworkDevicesAsync_Process() to receive and process incoming responses.
 *
 * @param devices Pointer to an array of sPoKeysNetworkDeviceSummary structures to store found devices.
 * @param timeout_ms Timeout duration for the discovery process in milliseconds.
 * @param serialNumberToFind Optional serial number to stop discovery early if a specific device is found (0 = find all).
 *
 * @return PK_OK on success,
 *         PK_ERR_SOCKET on socket creation failure,
 *         PK_ERR otherwise.
 *
 * @note
 * - Must be called once to initiate discovery.
 * - Does not block. Discovery proceeds asynchronously.
 * - Devices will be filled in the provided array as responses are received.
 *
 * @see PK_SearchNetworkDevicesAsync_Process()
 */
int PK_SearchNetworkDevicesAsync_Start(sPoKeysNetworkDeviceSummary* devices, uint32_t timeout_ms, uint32_t serialNumberToFind)
{
    uint32_t* addr;
    struct sockaddr_in remoteEP;
    int UDPbroadcast = 1;

    memset(&discoveryCtx, 0, sizeof(discoveryCtx));
    discoveryCtx.devices = devices;
    discoveryCtx.timeout_us = timeout_ms * 1000;
    discoveryCtx.nrOfDetectedBoards = 0;
    discoveryCtx.serialNumberToFind = serialNumberToFind;

    discoveryCtx.txSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (discoveryCtx.txSocket < 0) {
        return PK_ERR_SOCKET;
    }

    fcntl(discoveryCtx.txSocket, F_SETFL, O_NONBLOCK); // non-blocking

    setsockopt(discoveryCtx.txSocket, SOL_SOCKET, SO_BROADCAST, &UDPbroadcast, sizeof(UDPbroadcast));

    addr = GetBroadcastAddressesAsync();
    if (!addr) return PK_ERR_SOCKET;

    for (uint32_t* p = addr; *p != 0; p++) {
        memset(&remoteEP, 0, sizeof(remoteEP));
        remoteEP.sin_family = AF_INET;
        remoteEP.sin_port = htons(20055);
        remoteEP.sin_addr.s_addr = *p;

        sendto(discoveryCtx.txSocket, "", 0, 0, (struct sockaddr*)&remoteEP, sizeof(remoteEP));
    }

    // IMPORTANT: do not free addr if hal_malloc was used
    // just let it live until component unload

    discoveryCtx.start_time_us = get_current_time_us();
    return PK_OK;
}

/**
 * @brief Processes incoming responses during asynchronous device search.
 *
 * This function must be called periodically (e.g., once per realtime cycle) after
 * PK_SearchNetworkDevicesAsync_Start() has been called. It checks for incoming UDP packets
 * without blocking and parses valid device discovery responses.
 *
 * @return PK_OK if no device yet or still waiting,
 *         PK_OK_FOUND if a device matching serialNumberToFind was found,
 *         PK_ERR_TIMEOUT if the discovery timeout expired,
 *         PK_ERR_TRANSFER on receive errors.
 *
 * @note
 * - This function is realtime-safe (no blocking, no dynamic memory).
 * - Must be called repeatedly until PK_OK_FOUND or PK_ERR_TIMEOUT is returned.
 * - Detected devices are appended to the devices array provided at Start().
 * - After timeout or finding a specific device, the discovery socket is closed automatically.
 *
 * @see PK_SearchNetworkDevicesAsync_Start()
 */
int PK_SearchNetworkDevicesAsync_Process(void)
{
    if (discoveryCtx.txSocket < 0)
        return PK_ERR_SOCKET;

    // Check timeout
    uint64_t now = get_current_time_us();
    if ((now - discoveryCtx.start_time_us) > discoveryCtx.timeout_us)
    {
        close(discoveryCtx.txSocket);
        discoveryCtx.txSocket = -1;
        return PK_ERR_TIMEOUT;
    }

    unsigned char rcvbuf[500];
    struct sockaddr_in srcaddr;
    socklen_t addrlen = sizeof(srcaddr);

    int status = recvfrom(discoveryCtx.txSocket, rcvbuf, sizeof(rcvbuf), 0, (struct sockaddr*)&srcaddr, &addrlen);
    if (status < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return PK_OK; // No data yet, normal
        else
            return PK_ERR_TRANSFER; // Real error
    }

    if (status == 14 || status == 19)
    {
        sPoKeysNetworkDeviceSummary* device = &discoveryCtx.devices[discoveryCtx.nrOfDetectedBoards];

        if (status == 14) {
            device->SerialNumber = (int)(256 * (int)rcvbuf[1] + rcvbuf[2]);
            device->FirmwareVersionMajor = rcvbuf[3];
            device->FirmwareVersionMinor = rcvbuf[4];
            memcpy(device->IPaddress, rcvbuf + 5, 4);
            device->DHCP = rcvbuf[9];
            memcpy(device->hostIP, rcvbuf + 10, 4);
            device->HWtype = 0;
        } else if (status == 19) {
            device->SerialNumber = (int)rcvbuf[14] + ((int)rcvbuf[15] << 8) + ((int)rcvbuf[16] << 16) + ((int)rcvbuf[17] << 24);
            device->FirmwareVersionMajor = rcvbuf[3];
            device->FirmwareVersionMinor = rcvbuf[4];
            memcpy(device->IPaddress, rcvbuf + 5, 4);
            device->DHCP = rcvbuf[9];
            memcpy(device->hostIP, rcvbuf + 10, 4);
            device->HWtype = rcvbuf[18];
        }

        discoveryCtx.nrOfDetectedBoards++;

        // If specific serial matched, finish early
        if (device->SerialNumber == discoveryCtx.serialNumberToFind)
        {
            close(discoveryCtx.txSocket);
            discoveryCtx.txSocket = -1;
            return PK_OK_FOUND;
        }
    }

    return PK_OK;
}


/**
 * @brief Connects to a PoKeys network device asynchronously (UDP only, Linux-only).
 *
 * This function creates a non-blocking UDP socket and prepares the connection 
 * structure for communicating with the PoKeys device. 
 * No blocking calls (like connect or select) are used, ensuring realtime safety.
 *
 * @param device Pointer to the sPoKeysNetworkDeviceSummary structure containing 
 *               the device IP address and connection parameters.
 *
 * @return Pointer to an initialized sPoKeysDevice structure on success, 
 *         or NULL on failure (e.g., memory allocation failure or socket creation error).
 *
 * @note
 * - Only UDP connections are supported (TCP not implemented here).
 * - Socket is set to non-blocking mode immediately after creation.
 * - All memory allocations use hal_malloc() for realtime safety.
 * - The resulting sPoKeysDevice must be managed and eventually released properly.
 *
 * @usage
 * After calling PK_SearchNetworkDevicesAsync and finding a device, 
 * use this function to establish a realtime-safe UDP connection to it.
 *
 * @see PK_SearchNetworkDevicesAsync_Start()
 * @see PK_SearchNetworkDevicesAsync_Process()
 */
 sPoKeysDevice* PK_ConnectToNetworkDeviceAsync(const sPoKeysNetworkDeviceSummary* device)
 {
     if (device == NULL)
         return NULL;
 
     sPoKeysDevice* tmpDevice = (sPoKeysDevice*)hal_malloc(sizeof(sPoKeysDevice));
     if (!tmpDevice)
         return NULL;
 
     memset(tmpDevice, 0, sizeof(sPoKeysDevice));
 
     tmpDevice->devHandle2 = hal_malloc(sizeof(struct sockaddr_in));
     if (!tmpDevice->devHandle2)
         return NULL;
 
     tmpDevice->connectionType = PK_DeviceType_NetworkDevice; // Network device
     tmpDevice->connectionParam = device->useUDP;
 
     uint32_t addr = (uint32_t)device->IPaddress[0]
                   + ((uint32_t)device->IPaddress[1] << 8)
                   + ((uint32_t)device->IPaddress[2] << 16)
                   + ((uint32_t)device->IPaddress[3] << 24);
 
     struct sockaddr_in* remoteEP = (struct sockaddr_in*)tmpDevice->devHandle2;
     memset(remoteEP, 0, sizeof(struct sockaddr_in));
     remoteEP->sin_family = AF_INET;
     remoteEP->sin_port = htons(20055);
     remoteEP->sin_addr.s_addr = addr;
 
     tmpDevice->devHandle = hal_malloc(sizeof(int));
     if (!tmpDevice->devHandle)
         return NULL;
 
     *(int*)tmpDevice->devHandle = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); // Always UDP
     if (*(int*)tmpDevice->devHandle == -1)
         return NULL;
 
     // Set non-blocking immediately
     fcntl(*(int*)tmpDevice->devHandle, F_SETFL, O_NONBLOCK);
 
     debug_printf("Socket created for device. Non-blocking UDP mode.\n");
 
     InitializeNewDevice(tmpDevice);
 
     return tmpDevice;
 }

 /**
 * @brief Sends a PoKeys request packet asynchronously (non-blocking, realtime-safe).
 *
 * Prepares the request buffer (sets command byte, request ID, checksum) and sends it 
 * via UDP to the PoKeys device using a non-blocking socket.
 *
 * @param device Pointer to the sPoKeysDevice structure.
 *
 * @return PK_OK if the request was sent successfully (64 bytes),
 *         PK_ERR_TRANSFER if sending failed,
 *         PK_ERR_GENERIC on invalid device handle.
 *
 * @note
 * - This function only performs the sending operation.
 * - Response must be checked separately via PK_RecvEthResponseAsync().
 * - Device must have been previously connected with PK_ConnectToNetworkDeviceAsync().
 *
 * @see PK_RecvEthResponseAsync()
 */
 int PK_SendEthRequestAsync(sPoKeysDevice* device)
 {
     if (device == NULL || device->devHandle == NULL)
         return PK_ERR_GENERIC;
 
     device->requestID++;
     device->request[0] = 0xBB;
     device->request[6] = device->requestID;
     device->request[7] = getChecksum(device->request);
 
     ssize_t sent = sendto(*(int*)device->devHandle, device->request, 64, 0,
                           (struct sockaddr*)device->devHandle2, sizeof(struct sockaddr_in));
 
     if (sent != 64)
         return PK_ERR_TRANSFER;
 
     return PK_OK;
 }

/**
 * @brief Receives a PoKeys response packet asynchronously (non-blocking, realtime-safe).
 *
 * Attempts to read a single UDP packet from the PoKeys device without blocking.
 * Validates the received packet against the expected request ID and checksum.
 * If successful, the response is stored in device->response.
 *
 * @param device Pointer to the sPoKeysDevice structure.
 *
 * @return PK_OK if a valid response was received,
 *         PK_ERR_AGAIN if no data is currently available (non-fatal, try again later),
 *         PK_ERR_TRANSFER if an invalid or corrupted response was received,
 *         PK_ERR_GENERIC on invalid device handle.
 *
 * @note
 * - This function should be called periodically after sending a request.
 * - It is non-blocking and safe for use inside a realtime loop.
 * - If PK_ERR_AGAIN is returned, simply continue polling.
 *
 * @see PK_SendEthRequestAsync()
 */
int PK_RecvEthResponseAsync(sPoKeysDevice* device)
{
    if (device == NULL || device->devHandle == NULL)
        return PK_ERR_GENERIC;

    unsigned char tmpbuf[64];
    ssize_t received = recv(*(int*)device->devHandle, tmpbuf, sizeof(tmpbuf), MSG_DONTWAIT);

    if (received < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return PK_ERR_AGAIN; // No data available now
        else
            return PK_ERR_TRANSFER;
    }

    if (received != 64)
        return PK_ERR_TRANSFER;

    // Validate response
    if (tmpbuf[0] != 0xAA || tmpbuf[6] != device->requestID)
        return PK_ERR_TRANSFER;

    if (tmpbuf[7] != getChecksum(tmpbuf))
        return PK_ERR_TRANSFER;

    // Copy response safely
    memcpy(device->response, tmpbuf, 64);

    return PK_OK;
}

/**
 * @brief Sends a PoKeys request packet asynchronously without expecting a response (fire-and-forget).
 *
 * Prepares a UDP packet with correct formatting (start byte, request ID, checksum) and 
 * sends it once over the network to the PoKeys device. No response is waited for.
 *
 * This is typically used for operations where the device executes the command
 * without sending a confirmation (e.g., certain control or update commands).
 *
 * @param device Pointer to the sPoKeysDevice structure.
 *
 * @return PK_OK if the packet was sent successfully (64 bytes transmitted),
 *         PK_ERR_TRANSFER if sending failed,
 *         PK_ERR_GENERIC if device pointer or connection is invalid.
 *
 * @note
 * - The socket must be pre-configured and connected via PK_ConnectToNetworkDeviceAsync().
 * - No retries or waiting are done inside this function (caller must handle retries if needed).
 * - Works in realtime-safe LinuxCNC HAL loops (non-blocking sendto()).
 *
 * @usage
 * - Prepare your device connection first.
 * - Then call PK_SendEthRequestNoResponseAsync() whenever you need to trigger an action without awaiting feedback.
 *
 * @see PK_ConnectToNetworkDeviceAsync()
 * @see PK_SendEthRequestAsync()
 */
 int PK_SendEthRequestNoResponseAsync(sPoKeysDevice* device)
 {
     if (device == NULL || device->devHandle == NULL)
         return PK_ERR_GENERIC;
 
     device->requestID++;
 
     device->request[0] = 0xBB;
     device->request[6] = device->requestID;
     device->request[7] = getChecksum(device->request);
 
     ssize_t sent = sendto(*(int*)device->devHandle,
                           device->request, 64, 0,
                           (struct sockaddr*)device->devHandle2,
                           sizeof(struct sockaddr_in));
 
     if (sent != 64)
         return PK_ERR_TRANSFER;
 
     return PK_OK;
 }

 
 
/**
 * @brief Sends a large (multi-part) PoKeys request asynchronously (non-blocking).
 *
 * This function splits a prepared PoKeys request into 8 × 64-byte segments and
 * sends them as a single 512-byte UDP packet to the target device.
 *
 * This is typically used for sending larger payloads like motion buffers or extended data.
 *
 * @param device Pointer to the sPoKeysDevice structure, with valid multiPartData populated.
 *
 * @return PK_OK if the multi-part packet was sent successfully (512 bytes transmitted),
 *         PK_ERR_TRANSFER if sending failed,
 *         PK_ERR_GENERIC if device or buffers are invalid.
 *
 * @note
 * - No retries or blocking waits are performed inside this function.
 * - Only works with UDP connections (TCP not supported for multi-part transfers).
 * - Make sure `device->multiPartData` is filled with 448 bytes of payload data before calling.
 *
 * @see PK_RecvEthBigResponseAsync()
 */
int PK_SendEthRequestBigAsync(sPoKeysDevice* device)
{
    if (device == NULL || device->multiPartBuffer == NULL || device->devHandle == NULL)
        return PK_ERR_GENERIC;

    // Fill 8 × 64-byte parts
    for (uint32_t i = 0; i < 8; i++)
    {
        uint8_t* p = &device->multiPartBuffer[i * 64];

        memcpy(p, device->request, 8);
        p[0] = 0xBB;
        p[2] = i;
        if (i == 0) p[2] |= (1 << 3); // First
        if (i == 7) p[2] |= (1 << 4); // Last

        p[6] = ++device->requestID;
        p[7] = getChecksum(p);

        memcpy(p + 8, device->multiPartData + (i * 56), 56);
    }

    // Send entire 512-byte buffer
    ssize_t sent = sendto(*(int*)device->devHandle,
                          device->multiPartBuffer, 512, 0,
                          (struct sockaddr*)device->devHandle2,
                          sizeof(struct sockaddr_in));

    return (sent == 512) ? PK_OK : PK_ERR_TRANSFER;
}

/**
 * @brief Receives the response to a large (multi-part) PoKeys request asynchronously.
 *
 * After sending a multi-part request with PK_SendEthRequestBigAsync(), this function
 * checks (non-blocking) if a valid response packet from the device is available.
 *
 * Verifies packet start byte, request ID match, and checksum integrity.
 *
 * @param device Pointer to the sPoKeysDevice structure.
 *
 * @return PK_OK if a valid 64-byte response packet was received,
 *         PK_ERR_AGAIN if no packet is available yet (socket would block),
 *         PK_ERR_TRANSFER if packet received is invalid or corrupted.
 *
 * @note
 * - Intended for use inside a polling loop after a big request was sent.
 * - No retries or waiting are performed internally (caller should retry if necessary).
 * - This method is realtime-safe and non-blocking.
 *
 * @see PK_SendEthRequestBigAsync()
 */
 int PK_RecvEthBigResponseAsync(sPoKeysDevice* device)
 {
     if (device == NULL || device->devHandle == NULL)
         return PK_ERR_GENERIC;
 
     unsigned char tmpbuf[64];
     ssize_t received = recv(*(int*)device->devHandle, tmpbuf, sizeof(tmpbuf), MSG_DONTWAIT);
 
     if (received < 0)
     {
         if (errno == EAGAIN || errno == EWOULDBLOCK)
             return PK_ERR_AGAIN;
         else
             return PK_ERR_TRANSFER;
     }
 
     if (received != 64)
         return PK_ERR_TRANSFER;
 
     if (tmpbuf[0] != 0xAA || tmpbuf[6] != device->requestID || tmpbuf[7] != getChecksum(tmpbuf))
         return PK_ERR_TRANSFER;
 
     memcpy(device->response, tmpbuf, 64);
     return PK_OK;
 }




 
// Get a list of interfaces - source: Jeremy @ http://developerweb.net/viewtopic.php?id=5085
typedef unsigned long uint32;

#if defined(__FreeBSD__) || defined(BSD) || defined(__APPLE__) || defined(__linux__)
    #define USE_GETIFADDRS 1
    #define SOCKADDR struct sockaddr

    #include <ifaddrs.h>

    uint32 SockAddrToUint32(struct sockaddr * a)
    {
       return ((a)&&(a->sa_family == AF_INET)) ? ntohl(((struct sockaddr_in *)a)->sin_addr.s_addr) : 0;
    }
#endif

uint32 * GetBroadcastAddresses()
{
    uint32 * list = malloc(sizeof(uint32) * 100);
    uint32* ptr = list;
#if defined(WIN32)
    uint32 i;
    MIB_IPADDRTABLE * ipTable = NULL;
    IP_ADAPTER_INFO * pAdapterInfo = NULL;
    ULONG bufLen = 0;
    uint32 ipRet;
    uint32 apRet;

    MIB_IPADDRROW * row;

    uint32 ipAddr;
    uint32 netmask;
    uint32 baddr;

#endif
#if defined(USE_GETIFADDRS)
    uint32 tmp;
#endif

    *ptr = 0;
    

#if defined(USE_GETIFADDRS)
    // BSD-style implementation
    struct ifaddrs * ifap;
    if (getifaddrs(&ifap) == 0)
    {
        struct ifaddrs * p = ifap;
        while(p)
        {
            uint32 ifaAddr  = SockAddrToUint32(p->ifa_addr);

            if (ifaAddr > 0)
            {
				tmp = ((struct sockaddr_in *)p->ifa_broadaddr)->sin_addr.s_addr;
				if (tmp) *(ptr++) = tmp;
			}
            p = p->ifa_next;
        }
        freeifaddrs(ifap);
    }
#elif defined(WIN32)
    // Windows XP style implementation

    // Adapted from example code at http://msdn2.microsoft.com/en-us/library/aa365917.aspx
    // Now get Windows' IPv4 addresses table.  Once again, we gotta call GetIpAddrTable()
    // multiple times in order to deal with potential race conditions properly.
    {
      for (i=0; i<5; i++)
      {
         ipRet = GetIpAddrTable(ipTable, &bufLen, 0);
         if (ipRet == ERROR_INSUFFICIENT_BUFFER)
         {
            free(ipTable);  // in case we had previously allocated it
            ipTable = (MIB_IPADDRTABLE *) malloc(bufLen);
         }
         else if (ipRet == NO_ERROR) break;
         else
         {
            free(ipTable);
            ipTable = NULL;
            break;
         }
     }
   }

   if (ipTable)
   {
      // Try to get the Adapters-info table, so we can given useful names to the IP
      // addresses we are returning.  Gotta call GetAdaptersInfo() up to 5 times to handle
      // the potential race condition between the size-query call and the get-data call.
      // I love a well-designed API :^P
      {
         for (i=0; i<5; i++)
         {
            apRet = GetAdaptersInfo(pAdapterInfo, &bufLen);
            if (apRet == ERROR_BUFFER_OVERFLOW)
            {
               free(pAdapterInfo);  // in case we had previously allocated it
               pAdapterInfo = (IP_ADAPTER_INFO *) malloc(bufLen);
            }
            else if (apRet == ERROR_SUCCESS) break;
            else
            {
               free(pAdapterInfo);
               pAdapterInfo = NULL;
               break;
            }
         }
      }

      for (i=0; i < ipTable->dwNumEntries; i++)
      {
          // -----------------------------------------------------------------------------------------
         row = &ipTable->table[i];
         // -----------------------------------------------------------------------------------------

         ipAddr  = (row->dwAddr);
         netmask = (row->dwMask);
         baddr   = ipAddr & netmask;
         if (row->dwBCastAddr) baddr |= ~netmask;

		 if (baddr) *(ptr++) = baddr;
      }

      free(pAdapterInfo);
      free(ipTable);
   }
#else
   // Dunno what we're running on here!
#  error "Don't know how to implement GetBroadcastAddresses() on this OS!"
#endif

	*ptr = 0;
    return list;
}

#ifdef WIN32
    WSADATA wsaData;

    int32_t InitWinsock()
    {
            // Initialize Winsock - version 2.2
            if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
            {
                    debug_printf("Winsock startup failed!\n");
                    return -1;
            }
            return 0;
    }

    int32_t TerminateWinsock()
    {
            WSACleanup();
            return 0;
    }
#endif


int32_t PK_EnumerateNetworkDevices(sPoKeysNetworkDeviceSummary * devices, uint32_t timeout)
{
    return PK_SearchNetworkDevices(devices, timeout, 0);
}

int32_t PK_SearchNetworkDevices(sPoKeysNetworkDeviceSummary * devices, uint32_t timeout, uint32_t serialNumberToFind)
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
    int UDPbroadcast = 1;
    int status = 0;
    int BufLen = 0;
    char SendBuf[1];
    uint32 * addr;
    uint32 * addrPtr;
    uint32 a;
    unsigned char rcvbuf[500];
    int nrOfDetectedBoards = 0;
    sPoKeysNetworkDeviceSummary * device;

    t = timeout;

    debug_printf("Enumerating network PoKeys devices for %d ms...\n", timeout);

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


    debug_printf("Sending discovery request...\n");

    addr = GetBroadcastAddresses();
    addrPtr = addr;

    while(*addr)
    {
        a = *addr;
        debug_printf("Sending request to %d.%d.%d.%d... ", (a&0xFF),(a>>8)&0xFF, (a>>16)&0xFF, (a>>24)&0xFF);

        // Send discovery request...
        remoteEP.sin_family = AF_INET;
        remoteEP.sin_port = htons(20055);
        remoteEP.sin_addr.s_addr = a; // inet_addr("255.255.255.255");

        if (sendto(txSocket, SendBuf, BufLen, 0, (SOCKADDR *)&remoteEP, sizeof(remoteEP)) == -1)
        {
            debug_printf("Failed\n");
#ifdef WIN32
            closesocket(txSocket);
            WSACleanup();
#else
            close(txSocket);
#endif
            return 0;
        }
        debug_printf(" done\n");
        addr++;
    }
    free(addrPtr);


    debug_printf("Waiting for responses...\n");



#ifdef WIN32

#else
    // Wait for discovery response...
    stimeout.tv_sec = 0;
    stimeout.tv_usec = 1000 * timeout;

    FD_ZERO(&fds);
    FD_SET(txSocket, &fds);
#endif


    while(nrOfDetectedBoards < 16)
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

        debug_printf("Retrieving data...\n");
        status = recv(txSocket, (char *)rcvbuf, sizeof(rcvbuf), 0);

        if (status < 0)
        {
#ifdef WIN32
            int iError = WSAGetLastError();

            if (iError != WSAECONNRESET) {
                debug_printf("Error receiving data %d / %ld...\n", status, iError);
                break;
            }
#else
            break;
#endif
        }

        // Get IP address and receive message
        if (status > 0)
        {
            if (status == 14)
            {
                // Parse the response
                debug_printf("Received response...\n");

                debug_printf("  User ID: %u\n", rcvbuf[0]);
                debug_printf("  Serial: %u\n", (int)(256 * (int)rcvbuf[1] + rcvbuf[2]));
                debug_printf("  Version: %u.%u\n", rcvbuf[3], rcvbuf[4]);
                debug_printf("  IP address: %u.%u.%u.%u\n", rcvbuf[5], rcvbuf[6], rcvbuf[7], rcvbuf[8]);
                debug_printf("  DHCP: %u\n", rcvbuf[9]);
                debug_printf("  Host IP address: %u.%u.%u.%u\n", rcvbuf[10], rcvbuf[11], rcvbuf[12], rcvbuf[13]);

                // Save the device info
                device = &devices[nrOfDetectedBoards];
                //device->userID = rcvbuf[0];
                device->SerialNumber = (int)(256 * (int)rcvbuf[1] + rcvbuf[2]);
                device->FirmwareVersionMajor = rcvbuf[3];
                device->FirmwareVersionMinor = rcvbuf[4];
                memcpy(device->IPaddress, rcvbuf + 5, 4);
                device->DHCP = rcvbuf[9];
                memcpy(device->hostIP, rcvbuf + 10, 4);
                device->HWtype = 0;

                nrOfDetectedBoards++;
                status = 0;

                if (serialNumberToFind == device->SerialNumber) break;
            } else if (status == 19)
            {
                // Parse the response
                debug_printf("Received response from 58 series device...\n");

                // Save the device info
                device = &devices[nrOfDetectedBoards];
                //device->userID = rcvbuf[0];
                device->SerialNumber = (int)rcvbuf[14] + ((int)rcvbuf[15] << 8) + ((int)rcvbuf[16] << 16) + ((int)rcvbuf[17] << 24);
                device->FirmwareVersionMajor = rcvbuf[3];
                device->FirmwareVersionMinor = rcvbuf[4];
                memcpy(device->IPaddress, rcvbuf + 5, 4);
                device->DHCP = rcvbuf[9];
                memcpy(device->hostIP, rcvbuf + 10, 4);
                device->HWtype = rcvbuf[18];

                debug_printf("  User ID: %u\n", rcvbuf[0]);
                debug_printf("  Serial: %u\n", device->SerialNumber);
                debug_printf("  Version: %u.%u\n", rcvbuf[3], rcvbuf[4]);
                debug_printf("  IP address: %u.%u.%u.%u\n", rcvbuf[5], rcvbuf[6], rcvbuf[7], rcvbuf[8]);
                debug_printf("  DHCP: %u\n", rcvbuf[9]);
                debug_printf("  Host IP address: %u.%u.%u.%u\n", rcvbuf[10], rcvbuf[11], rcvbuf[12], rcvbuf[13]);

                nrOfDetectedBoards++;
                status = 0;

                if (serialNumberToFind == device->SerialNumber) break;
            }
        }
		else
		{
			if (nrOfDetectedBoards == 0)
			{
				debug_printf("\n No Boards detected\n");
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

sPoKeysDevice* PK_ConnectToNetworkDevice(sPoKeysNetworkDeviceSummary * device)
{
    int result;
#ifdef WIN32
    int t = 100; // 100 ms timeout
#endif
    // Create target endpoint
    //struct sockaddr_in * remoteEP;
    uint32 addr ;

    // Create temporary device object
    sPoKeysDevice * tmpDevice;

    if (device == NULL) return NULL;

    tmpDevice = (sPoKeysDevice*)malloc(sizeof(sPoKeysDevice));
    tmpDevice->devHandle2 = malloc(sizeof(struct sockaddr_in));

    tmpDevice->connectionType = PK_DeviceType_NetworkDevice; // Network device
    tmpDevice->connectionParam = device->useUDP;

    addr = (uint32)device->IPaddress[0] + ((uint32)device->IPaddress[1] << 8) + ((uint32)device->IPaddress[2] << 16) + ((uint32)device->IPaddress[3] << 24);
    // Set up the RecvAddr structure with the broadcast ip address and correct port number
    ((struct sockaddr_in*)tmpDevice->devHandle2)->sin_family = AF_INET;
    ((struct sockaddr_in*)tmpDevice->devHandle2)->sin_port = htons(20055);
    ((struct sockaddr_in*)tmpDevice->devHandle2)->sin_addr.s_addr = addr;

#ifdef WIN32    
    if (InitWinsock() != 0)
    {
        debug_printf("InitWinsock error!");
        return 0;
    }


    if (tmpDevice->connectionParam == PK_ConnectionParam_UDP)
    {
        tmpDevice->devHandle = (void*)socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    } else
    {
        tmpDevice->devHandle = (void*)socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    }

    if ((SOCKET)tmpDevice->devHandle == -1)
#else
    tmpDevice->devHandle = malloc(sizeof(int));
    if (tmpDevice->devHandle == NULL) return NULL;
    
    if (tmpDevice->connectionParam == PK_ConnectionParam_UDP)
        *(int *)tmpDevice->devHandle = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    else
        *(int *)tmpDevice->devHandle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	
	if ((*(int *)tmpDevice->devHandle) == -1)
#endif
    {
        //CleanDevice(tmpDevice);
#ifndef WIN32
        free(tmpDevice->devHandle);
#endif
        free(tmpDevice);

        return NULL; // Couldn't create the socket
    }

#ifdef WIN32
	// Set socket options
	setsockopt((SOCKET)tmpDevice->devHandle, SOL_SOCKET, SO_RCVTIMEO, (char *)&t, sizeof(t));
    setsockopt((SOCKET)tmpDevice->devHandle, SOL_SOCKET, SO_SNDTIMEO, (char *)&t, sizeof(t));
#endif


	// Connect to target IP
#ifdef WIN32
    if (tmpDevice->connectionParam != PK_ConnectionParam_UDP)
        result = connect((SOCKET)tmpDevice->devHandle, (SOCKADDR *)tmpDevice->devHandle2, sizeof(struct sockaddr_in));
    else
        result = 0;
#else
	result = 0;
    if (tmpDevice->connectionParam != PK_ConnectionParam_UDP)
        result = connect(*(int *)tmpDevice->devHandle, (struct sockaddr *)tmpDevice->devHandle2, sizeof(struct sockaddr_in));
	else
        result = 0;
		
#endif

    if (result == -1)
    {
		debug_printf(" ERROR");
        //CleanDevice(tmpDevice);
#ifdef WIN32
        closesocket((SOCKET)tmpDevice->devHandle);
#else
        close(*(int *)tmpDevice->devHandle);
        free(tmpDevice->devHandle);
#endif
        free(tmpDevice);

        return NULL; // Couldn't connect
    }

#ifdef WIN32
    if (tmpDevice->connectionParam == PK_ConnectionParam_UDP)
    {
        // Setup socket to be unblocking
        u_long iMode = TRUE;
        ioctlsocket((SOCKET)tmpDevice->devHandle, FIONBIO, &iMode);
    }
#endif

	debug_printf(" Connected\n");

    debug_printf("Initializing the device object... ");
    InitializeNewDevice(tmpDevice);
    debug_printf("done\n");


	//printf("Connected to PoKeys device at %s", addrBuf);
	return tmpDevice;
}

void PK_DisconnectNetworkDevice(sPoKeysDevice* device)
{
    if (device == NULL) return;
	if (device->connectionType != PK_DeviceType_NetworkDevice) return;

    debug_printf("\nClosing connection...");

#ifdef WIN32
    if ((SOCKET)device->devHandle)
		closesocket((SOCKET)device->devHandle);    
#else
    close(*(int *)device->devHandle);
    free(device->devHandle);
#endif

    // Release secondary device handle
    if (device->devHandle2)
        free(device->devHandle2);
}


int32_t SendEthRequest(sPoKeysDevice* device)
{    
    uint32_t retries1 = 0;
    uint32_t retries2 = 0;
    int result;

    fd_set fds;
    struct timeval stimeout;

    if (device == NULL) return PK_ERR_GENERIC;
    if (device->connectionType != PK_DeviceType_NetworkDevice) return PK_ERR_GENERIC;
    if (device->devHandle == NULL) return PK_ERR_GENERIC;

    while (1)
    {
        // Form the request packet
        device->requestID++;

        device->request[0] = 0xBB;
        device->request[6] = device->requestID;
        device->request[7] = getChecksum(device->request);

        //memcpy(requestBuffer, device->request, 64);

        debug_printf("\nSending...");
        // Send the data

        if (device->connectionParam == PK_ConnectionParam_UDP)
        {
#ifdef WIN32
            if (sendto((SOCKET)device->devHandle, (char *)device->request, 64, 0, (SOCKADDR *)device->devHandle2, sizeof(struct sockaddr_in)) == -1)
#else
            if (sendto(*(int*)device->devHandle, (char *)device->request, 64, 0, (SOCKADDR *)device->devHandle2, sizeof(struct sockaddr_in)) == -1)
#endif
            {
                debug_printf("Error sending UDP report\nAborting...\n");
                return -1;
            }
        } else
        {
#ifdef WIN32
            if (send((SOCKET)device->devHandle, (char *)device->request, 64, 0) != 64)
#else
            if (send(*(int*)device->devHandle, (char *)device->request, 64, 0) != 64)
#endif
            {
                debug_printf("Error sending TCP report\nAborting...\n");
                return -1;
            }
        }

		// Wait for the response
		while(1)
		{           
#ifdef WIN32
            FD_ZERO(&fds);
            FD_SET((SOCKET)device->devHandle, &fds);

            stimeout.tv_sec = 0;
			stimeout.tv_usec = 1000 * device->socketTimeout;

            result = select((SOCKET)device->devHandle + 1, &fds, NULL, NULL, &stimeout);

            if (result == 0 || result == -1)
            {
                // Timeout...
                debug_printf("Timeout!");
                if (++retries1 > device->readRetries) break;
                continue;
            }

            //device->reserved64 = GetTickCount64();
            result = recv((SOCKET)device->devHandle, (char *)device->response, 64, 0);
            //device->reserved64 = GetTickCount64() - device->reserved64;
            //printf("Time: %u\n", (uint32_t)device->reserved64);
#else
            FD_ZERO(&fds);
            FD_SET(*(int*)device->devHandle, &fds);

            stimeout.tv_sec = 0;
            stimeout.tv_usec = 1000 * device->socketTimeout;

            result = select(*(int*)device->devHandle + 1, &fds, NULL, NULL, &stimeout);

            if (result == 0 || result == -1)
            {
                // Timeout...
                debug_printf("Timeout!");
                if (++retries1 > 10) break;
                continue;
            }

            //if (FD_ISSET(*(int*)device->devHandle, &fds) == 0)

            result = recv(*(int*)device->devHandle, (char *)device->response, 64, 0);
#endif

			// 64 bytes received?
			if (result == 64)
			{
				if (device->response[0] == 0xAA && device->response[6] == device->requestID)
				{
					if (device->response[7] == getChecksum(device->response))
					{
						debug_printf(" Received!");
						return PK_OK;
					} else 
					{
						debug_printf("!! Wrong checksum...");
					}
				} else
				{
					debug_printf("!! Wrong response received!");
					break;
				}
			}
			else if (result == 0)
				debug_printf("Connection closed\n");
			else
#ifdef WIN32
                debug_printf("recv failed: %d\n", WSAGetLastError());
#else
                debug_printf("recv failed\n");
#endif

            
            if (++retries1 > device->readRetries) break;
		}

        if (retries2++ > device->sendRetries) break;
    }

	debug_printf("Error - timeout...");
	return PK_ERR_TRANSFER;

}

int32_t SendEthRequest_NoResponse(sPoKeysDevice* device)
{    
    uint32_t retries1 = 0;
    uint32_t retries2 = 0;
    //int result;

    //fd_set fds;
    //struct timeval stimeout;

    if (device == NULL) return PK_ERR_GENERIC;
    if (device->connectionType != PK_DeviceType_NetworkDevice) return PK_ERR_GENERIC;
    if (device->devHandle == NULL) return PK_ERR_GENERIC;

    while (1)
    {
        // Form the request packet
        device->requestID++;

        device->request[0] = 0xBB;
        device->request[6] = device->requestID;
        device->request[7] = getChecksum(device->request);

        // Send the data
        if (device->connectionParam == PK_ConnectionParam_UDP)
        {
#ifdef WIN32
            if (sendto((SOCKET)device->devHandle, (char *)device->request, 64, 0, (SOCKADDR *)device->devHandle2, sizeof(struct sockaddr_in)) == -1)
#else
            if (sendto(*(int*)device->devHandle, (char *)device->request, 64, 0, (SOCKADDR *)device->devHandle2, sizeof(struct sockaddr_in)) == -1)
#endif
            {
                debug_printf("Error sending UDP report\nAborting...\n");
                return -1;
            }
        } else
        {
#ifdef WIN32
            if (send((SOCKET)device->devHandle, (char *)device->request, 64, 0) != 64)
#else
            if (send(*(int*)device->devHandle, (char *)device->request, 64, 0) != 64)
#endif
            {
                debug_printf("Error sending TCP report\nAborting...\n");
                return -1;
            }
        }
		
        if (retries2++ > device->sendRetries) break;
    }

	debug_printf("Error - timeout...");
	return PK_ERR_TRANSFER;

}

int32_t SendEthRequestBig(sPoKeysDevice* device)
{
    uint32_t retries1 = 0;
    uint32_t retries2 = 0;
    int result;
	uint32_t i;

    fd_set fds;
    struct timeval stimeout;

    uint8_t * requestBuffer;
    uint8_t * requestBufferPtr = 0;

    if (device == NULL) return PK_ERR_GENERIC;
    if (device->connectionType != PK_DeviceType_NetworkDevice) return PK_ERR_GENERIC;
    if (device->devHandle == NULL) return PK_ERR_GENERIC;

    if (device->multiPartBuffer == 0) return PK_ERR_GENERIC;
    requestBuffer = device->multiPartBuffer;

    while (1)
    {
        // Form the request packet
        for (i = 0; i < 8; i++)
        {
            requestBufferPtr = &requestBuffer[i * 64];

            memcpy(requestBufferPtr, device->request, 8);
            requestBufferPtr[0] = 0xBB;

            // Put packet ID and flags here...
            requestBufferPtr[2] = i;
            if (i == 0)
                requestBufferPtr[2] |= (1<<3);
            else if (i == 7)
                requestBufferPtr[2] |= (1<<4);

            requestBufferPtr[6] = ++device->requestID;
            requestBufferPtr[7] = getChecksum(requestBufferPtr);

            memcpy(requestBufferPtr + 8, device->multiPartData + i*56, 56);
        }

        debug_printf("\nSending...");
        // Send the data

        if (device->connectionParam == PK_ConnectionParam_UDP)
        {
#ifdef WIN32
            if (sendto((SOCKET)device->devHandle, (char *)requestBuffer, 512, 0, (SOCKADDR *)device->devHandle2, sizeof(struct sockaddr_in)) == -1)
#else
            if (sendto(*(int*)device->devHandle, (char *)requestBuffer, 512, 0, (SOCKADDR *)device->devHandle2, sizeof(struct sockaddr_in)) == -1)
#endif
            {
                debug_printf("Error sending UDP report\nAborting...\n");
                return -1;
            }
        } else
        {
#ifdef WIN32
            if (send((SOCKET)device->devHandle, (char *)requestBuffer, 512, 0) != 512)
#else
            if (send(*(int*)device->devHandle, (char *)requestBuffer, 512, 0) != 512)
#endif
            {
                debug_printf("Error sending TCP report\nAborting...\n");
                return -1;
            }
        }

        // Wait for the response
        while(1)
        {
#ifdef WIN32
            FD_ZERO(&fds);
            FD_SET((SOCKET)device->devHandle, &fds);

            stimeout.tv_sec = 0;
            stimeout.tv_usec = 1000 * device->socketTimeout;

            result = select((SOCKET)device->devHandle + 1, &fds, NULL, NULL, &stimeout);

            if (result == 0 || result == -1)
            {
                // Timeout...
                debug_printf("Timeout!");
                if (++retries1 > device->readRetries) break;
                continue;
            }

            result = recv((SOCKET)device->devHandle, (char *)device->response, 64, 0);
#else
            FD_ZERO(&fds);
            FD_SET(*(int*)device->devHandle, &fds);

            stimeout.tv_sec = 0;
            stimeout.tv_usec = 1000 * device->socketTimeout;

            result = select(*(int*)device->devHandle + 1, &fds, NULL, NULL, &stimeout);

            if (result == 0 || result == -1)
            {
                // Timeout...
                debug_printf("Timeout!");
                if (++retries1 > 10) break;
                continue;
            }

            result = recv(*(int*)device->devHandle, (char *)device->response, 64, 0);
#endif

            // 64 bytes received?
            if (result == 64)
            {
                if (device->response[0] == 0xAA && device->response[6] == device->requestID)
                {
                    if (device->response[7] == getChecksum(device->response))
                    {
                        debug_printf(" Received!");
                        return PK_OK;
                    } else
                    {
                        debug_printf("!! Wrong checksum...");
                    }
                } else
                {
                    debug_printf("!! Wrong response received!");
                    break;
                }
            }
            else if (result == 0)
                debug_printf("Connection closed\n");
            else
#ifdef WIN32
                debug_printf("recv failed: %d\n", WSAGetLastError());
#else
                debug_printf("recv failed\n");
#endif


            if (++retries1 > device->readRetries) break;
        }

        if (retries2++ > device->sendRetries) break;
    }

    debug_printf("Error - timeout...");
    return PK_ERR_TRANSFER;

}


void PK_SetEthernetRetryCountAndTimeout(sPoKeysDevice * device, uint32_t sendRetries, uint32_t readRetries, uint32_t timeout)
{
    device->sendRetries = sendRetries;
    device->readRetries = readRetries;
    device->socketTimeout = timeout;
}

