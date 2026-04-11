#ifndef __API_COMMON
#define __API_COMMON

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#define MAX_DNS_RESOLUTION_FIELD_SIZE 256

typedef struct DNSResolutionResult
{
    char fullname[MAX_DNS_RESOLUTION_FIELD_SIZE];
    char hosttarget[MAX_DNS_RESOLUTION_FIELD_SIZE];
    uint16_t port;
    uint16_t txtLen;
    unsigned char txtRecord[MAX_DNS_RESOLUTION_FIELD_SIZE];
    void* browseInstance;
} DNSResolutionResult;

#ifdef __cplusplus
}
#endif

#endif // __API_COMMON