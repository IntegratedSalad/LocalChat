#ifndef __BONJOUR_API
#define __BONJOUR_API

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include "dns_sd.h"
#include "linked_list.h" 
#include "const.h"
#include "api_common.h"

extern bool replyError;

typedef struct ResolveCallbackContext
{
    void* discovery_p; // for accessing functions in callback - must point to BVDiscovery_Bonjour
    // void* browseInstance_p;
    char serviceName[N_BYTES_SERVNAME_MAX];
    char regType[N_BYTES_REGTYPE_MAX];
    char replyDomain[N_BYTES_REPLDOMN_MAX];
} ResolveCallbackContext;

// * Service Discovery callbacks * //

/* 
 *
 *
 * 
 * 
 * 
*/
void C_ServiceBrowseReply(
    DNSServiceRef sdRef,
    DNSServiceFlags flags,
    uint32_t interfaceIndex,
    DNSServiceErrorType errorCode,
    const char *serviceName,
    const char *regtype,
    const char *replyDomain,
    void *context);

// * Service Registration callbacks * //

/* 
 *
 *
 * 
 * 
 * 
*/
void C_RegisterReply(
    DNSServiceRef sdRef,
    DNSServiceFlags flags,
    DNSServiceErrorType errorCode,
    const char *name,
    const char *regtype,
    const char *domain,
    void *context);

// * Service Resolution callbacks * //
/* 
 *
 *
 * 
 * 
 * 
*/
void C_ResolveReply(
    DNSServiceRef sdRef,
    DNSServiceFlags flags,
    uint32_t interfaceIndex,
    DNSServiceErrorType errorCode,
    const char *fullname,
    const char *hosttarget,
    uint16_t port,
    uint16_t txtLen,
    const unsigned char *txtRecord,
    void *context);

#ifdef __cplusplus
}
#endif

#endif /* __BONJOUR_API */