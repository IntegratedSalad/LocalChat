#pragma once
#include "BVDiscovery.hpp"
#include "dns_sd.h"
#include <thread>
#include <memory>

/*
    This class will utilize an array of records.
    Access R/W has to be synchronized by a mutex!
 */
class BVDiscovery_Bonjour : public BVDiscovery
{
private:
    DNSServiceRef dnsRef;
    std::mutex& rwListMutex;
    std::unique_ptr<std::list> discoveryList_p;

public:
    BVDiscovery_Bonjour(std::mutex& _mutex, DNSServiceRef ref);// : rwListMutex(_mutex)
    ~BVDiscovery_Bonjour();
    
    void run() override;

};