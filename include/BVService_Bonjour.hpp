#pragma once
#include "BVService.hpp"
#include "dns_sd.h"
#include <atomic>

class BVService_Bonjour : private BVService
{
private:
    DNSServiceRef dnsRef;
    /*
        Do we need an abstraction to process result?
        Or will this be needlessly complex.     
    */
    BVStatus ProcessDNSServiceRegisterResult(void);

public:
    BVService_Bonjour(std::string& _hostname,  std::string& _domain, int _port) 
    : BVService(_hostname,  _domain, _port)
    {
    }

    BVStatus Register() override;

    ~BVService_Bonjour()
    {
        DNSServiceRefDeallocate(this->dnsRef);
    }
};