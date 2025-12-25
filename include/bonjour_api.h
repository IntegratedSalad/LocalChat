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

extern bool replyError;

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

#ifdef __cplusplus
}
#endif

#endif /* __BONJOUR_API */