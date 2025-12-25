#pragma once
#include "BVService.hpp"
#include "bonjour_api.h"
#include "dns_sd.h"
#include <atomic>
#include <iostream>
#include <algorithm>

class BVService_Bonjour : public BVService
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

    ~BVService_Bonjour() override
    {
        DNSServiceRefDeallocate(this->dnsRef);
    }

    BVStatus Register() override;
    DNSServiceRef GetDNSRef(void) const
    {
        return this->dnsRef;
    }
};