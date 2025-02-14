#include "BVDiscovery_Bonjour.hpp"

BVDiscovery_Bonjour(std::mutex& _mutex, DNSServiceRef ref) : rwListMutex(_mutex), dnsRef(ref)
{
    this->discoveryList_p = std::make_unique<std::list>();
}

~BVDiscovery_Bonjour()
{
}
    
void run() override
{
}