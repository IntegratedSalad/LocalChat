#include "bonjour_api.h"
#include "bonjour_api_bridge.h"

/* Excerpt from the dns_sd.h:
 * "or similar problem and has to be deregistered, the callback will
 * be invoked with the kDNSServiceFlagsAdd flag not set. The callback
 * is *not* invoked in the case where the caller explicitly terminates
 * the service registration by calling DNSServiceRefDeallocate(ref);"
 * Deregistration MUST NOT be handled by the Discovery.
 * ^ I read that wrong. This is in case of DNSServiceRegisterReply, not
 * BrowseReply.
 * Excerpt from dns_sd.h:
 * "kDNSServiceFlagsAdd     = 0x2,
 *  kDNSServiceFlagsDefault = 0x4,
 *  Flags for domain enumeration and browse/query reply callbacks.
 * "Default" applies only to enumeration and is only valid in
 * conjunction with "Add". An enumeration callback with the "Add"
 * flag NOT set indicates a "Remove", i.e. the domain is no longer
 * valid."
 */
void C_ServiceBrowseReply(
    DNSServiceRef sdRef,
    DNSServiceFlags flags,
    uint32_t interfaceIndex,
    DNSServiceErrorType errorCode,
    const char *serviceName,
    const char *regtype,
    const char *replyDomain,
    void *context)
{
    setbuf(stdout, NULL);
    if (errorCode == kDNSServiceErr_NoError)
    {
        if (context != NULL)
        {
            LinkedList_str* ll_p = (LinkedList_str*)context;
            // We handle deregistration from the mDNS side.
            if ( (flags & kDNSServiceFlagsAdd ) == 0x0 )
            {
                printf("Service %s.%s in %s has been deregistered\n", serviceName, regtype, replyDomain);
                ll_p->didRegister = 0;
            } else
            {
                printf("Found %s.%s in %s!\n", serviceName, regtype, replyDomain);
                ll_p->didRegister = 1;
            }
            char buff[N_BYTES_SERVICE_STR_TOTAL];
            const size_t servLen = strlen(serviceName);
            const size_t regLen = strlen(regtype);
            const size_t replDmnLen = strlen(replyDomain);
            for (int i = 0; i < N_BYTES_SERVICE_STR_TOTAL; i++)
            {
                buff[i] = ' ';
            }
            if (servLen < N_BYTES_SERVNAME_MAX)
            {
                memcpy(buff, serviceName, servLen);
            }
            if (regLen < N_BYTES_REGTYPE_MAX)
            {
                memcpy(buff + N_BYTES_SERVNAME_MAX, regtype, regLen);
            }
            if (replDmnLen < N_BYTES_REPLDOMN_MAX)
            {
                memcpy(buff + N_BYTES_SERVNAME_MAX + N_BYTES_REGTYPE_MAX, replyDomain, replDmnLen);
            }
            buff[N_BYTES_SERVICE_STR_TOTAL-1] = '\0';
            LinkedListElement_str* lle_p = LinkedListElement_str_Constructor(buff, NULL);
            LinkedList_str_AddElement(ll_p, lle_p);
        }
    } else
    {
        fprintf(stderr, "An error occurred while receiving browse reply.");
    }
}

void C_RegisterReply(
    DNSServiceRef sdRef,
    DNSServiceFlags flags,
    DNSServiceErrorType errorCode,
    const char *name,
    const char *regtype,
    const char *domain,
    void *context) // maybe pass the pointer to shouldProcess here?
{
    if (errorCode == kDNSServiceErr_NoError) {
        printf("--- Registered %s, as %s, in %s!\n", name, regtype, domain);
    } else {
        fprintf(stderr, "An error occurred while trying to register %s\n", name);
        replyError = true;
    }
}

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
    void *context)
{
    (void)sdRef;
    (void)flags;

    if (errorCode != kDNSServiceErr_NoError)
    {
        fprintf(stderr, "An error occurred while trying to resolve %s\n",
                fullname ? fullname : "<null>");
        replyError = true;
        return;
    }

    if (context == NULL)
    {
        fprintf(stderr, "Context not provided to C_ResolveReply callback!\n");
        replyError = true;
        return;
    }

    ResolveCallbackContext* resolve_callback_context_p =
        GetResolveCallbackContext(context);

    DNSResolutionResult* resolution_result_p =
        (DNSResolutionResult*)calloc(1, sizeof(DNSResolutionResult));

    if (resolution_result_p == NULL)
    {
        fprintf(stderr, "Failed to allocate DNSResolutionResult\n");
        replyError = true;
        return;
    }

    if (fullname != NULL)
    {
        size_t len = strlen(fullname);
        if (len >= sizeof(resolution_result_p->fullname))
            len = sizeof(resolution_result_p->fullname) - 1;

        memcpy(resolution_result_p->fullname, fullname, len);
        resolution_result_p->fullname[len] = '\0';
    }

    if (hosttarget != NULL)
    {
        size_t len = strlen(hosttarget);
        if (len >= sizeof(resolution_result_p->hosttarget))
            len = sizeof(resolution_result_p->hosttarget) - 1;

        memcpy(resolution_result_p->hosttarget, hosttarget, len);
        resolution_result_p->hosttarget[len] = '\0';
    }

    if (resolve_callback_context_p->serviceName != NULL)
    {
        size_t len = strlen(resolve_callback_context_p->serviceName);
        if (len >= sizeof(resolution_result_p->serviceName))
            len = sizeof(resolution_result_p->serviceName) - 1;

        memcpy(resolution_result_p->serviceName, resolve_callback_context_p->serviceName, len);
        resolution_result_p->serviceName[len] = '\0';
    }

    if (resolve_callback_context_p->regType != NULL)
    {
        size_t len = strlen(resolve_callback_context_p->regType);
        if (len >= sizeof(resolution_result_p->regType))
            len = sizeof(resolution_result_p->regType) - 1;

        memcpy(resolution_result_p->regType, resolve_callback_context_p->regType, len);
        resolution_result_p->regType[len] = '\0';
    }

    if (resolve_callback_context_p->replyDomain != NULL)
    {
        size_t len = strlen(resolve_callback_context_p->replyDomain);
        if (len >= sizeof(resolution_result_p->replyDomain))
            len = sizeof(resolution_result_p->replyDomain) - 1;

        memcpy(resolution_result_p->replyDomain, resolve_callback_context_p->replyDomain, len);
        resolution_result_p->replyDomain[len] = '\0';
    }

    resolution_result_p->port = ntohs(port);
    resolution_result_p->txtLen = txtLen;

    if (txtRecord != NULL && txtLen > 0)
    {
        size_t copy_len = txtLen;
        if (copy_len > sizeof(resolution_result_p->txtRecord))
            copy_len = sizeof(resolution_result_p->txtRecord);

        memcpy(resolution_result_p->txtRecord, txtRecord, copy_len);
    }

    SendServiceResolvedMessageToApp(
        resolution_result_p,
        resolve_callback_context_p->discovery_p
    );
}