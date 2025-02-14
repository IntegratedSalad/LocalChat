#include "BVDiscovery_Bonjour.hpp"

// How to return the value serviceName?
// Maybe pass something to context?
extern "C"
{
#include <stdio.h>
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
    if (errorCode == kDNSServiceErr_NoError)
    {
        printf("Found %s.%s in %s!", serviceName, regtype, replyDomain);
    } else
    {
        fprintf(stderr, "An error occurred while receiving browse reply.");
    }
}
}

BVDiscovery_Bonjour::BVDiscovery_Bonjour(DNSServiceRef ref, std::mutex& _mutex) : dnsRef(ref), rwListMutex(_mutex)
{
    this->discoveryList_p = std::make_unique<std::list<BVServiceBrowseInstance>>();
}

BVDiscovery_Bonjour::~BVDiscovery_Bonjour()
{
}

BVStatus BVDiscovery_Bonjour::ProcessDNSServiceBrowseResult(void)
{

}

void BVDiscovery_Bonjour::Discover(void)
{
}
    
void BVDiscovery_Bonjour::run()
{
    // Define a timer (5s)
    // Schedule this timer over and over while performing Discovery
    // In the callback, pass ProcessDNSServiceBrowseResult
    // it will call DNSServiceBrowse, and somehow write back
    // the reply from the daemon - it will be a service name
    // with a regtype and domain

}