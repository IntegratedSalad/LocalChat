#include "bonjour_api.h"

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
        printf("Found %s.%s in %s!\n", serviceName, regtype, replyDomain);
        if (context != NULL)
        {
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
            LinkedList_str* ll_p = (LinkedList_str*)context;
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
