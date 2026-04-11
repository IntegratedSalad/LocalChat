#ifndef __BONJOUR_API_BRIDGE
#define __BONJOUR_API_BRIDGE

#ifdef __cplusplus
extern "C" {
#endif

#include "api_common.h"
#include "bonjour_api.h"
#include "const.h"

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

/*
 *
 *
 * 
 *
*/
void CopyBrowseInstanceFields(void* bI_p, char serviceName[N_BYTES_SERVNAME_MAX], 
                                          char regType[N_BYTES_REGTYPE_MAX],
                                          char replyDomain[N_BYTES_REPLDOMN_MAX]);

#ifdef __cplusplus
}
#endif

#endif /* __BONJOUR_API_BRIDGE */