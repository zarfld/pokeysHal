#include "PoKeysLibHal.h"
#include "PoKeysLibAsync.h"
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include "hal.h"  // for hal_malloc

uint32_t *GetBroadcastAddresses()
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
