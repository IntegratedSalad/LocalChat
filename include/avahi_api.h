#ifndef __AVAHI_API
#define __AVAHI_API

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <avahi-client/client.h>
#include <avahi-client/lookup.h>
#include <avahi-common/error.h>
#include <avahi-common/simple-watch.h>
#include "const.h"
#include "avahi_api_bridge.h"

extern bool critical_failure;
extern bool register_success;

/* Wrapper for Discovery object and its methods
*/

//client callback? client is not created here, but the daemon might call this function
// when there's problem with the client/client's state change
// typedef struct BrowseData
// {
//     AvahiClient* client_p;
//     AvahiSimplePoll* simplepoll_p;
//     BVDiscovery_Avahi* d_p;
// } BrowseData;

// * Service Discovery callbacks * //
/* 
 *
 * 
 * 
*/
void browse_callback(
    AvahiServiceBrowser* sb,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiBrowserEvent event,
    const char* name,
    const char* type,
    const char* domain,
    AVAHI_GCC_UNUSED AvahiLookupResultFlags flags,
    void* userdata);

#ifdef __cplusplus
}
#endif

#endif /* __AVAHI_API */