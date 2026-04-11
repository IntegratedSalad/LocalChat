#ifndef __BONJOUR_API_BRIDGE
#define __BONJOUR_API_BRIDGE

#ifdef __cplusplus
extern "C" {
#endif

#include "api_common.h"
#include "bonjour_api.h"

/*
 *
 *
 * 
 *
*/
ResolveCallbackContext* GetResolveCallbackContext(void* ctx);

/*
 *
 *
 * 
 *
*/
void SendServiceResolvedMessageToApp(DNSResolutionResult* res_p, void* obj);

#ifdef __cplusplus
}
#endif

#endif /* __BONJOUR_API_BRIDGE */