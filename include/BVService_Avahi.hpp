#pragma once
#include <iostream>
#include <algorithm>
#include <memory>
#include "BVService.hpp"
#include <avahi-client/client.h>
#include <avahi-common/simple-watch.h>

class BVService_Avahi : public BVService
{
private:

    /* 
     * Avahi client functions as a connection context (maybe something similar to DNSServiceRef)
     * Calling avahi_client_free() frees entry group object (service)
     * This must not be null until the termination of the service when User closes the application
    */
    std::unique_ptr<AvahiClient> client_p = nullptr;
    std::shared_ptr<AvahiSimplePoll> simple_poll_p = nullptr;

    BVStatus CreateAvahiClient(void);
    BVStatus Setup(void);
    BVStatus WaitForServiceCreation(void);

    /* 
     * Maybe do not enter main loop here.
     * Call avahi_simple_poll_iterate until AVAHI_CLIENT_S_RUNNING
     * Then -> create a service just like in example. Only after this,
     * the AvahiClient pointer is valid.
     * Discovery object should be put on the separate thread
     * Then we can loop indifintely
    */

public:
    BVService_Avahi(std::string& _hostname,  std::string& _domain, int _port, std::shared_ptr<AvahiSimplePoll> _simple_poll_p)
    : BVService(_hostname,  _domain, _port)
    {
        this->simple_poll_p = _simple_poll_p;
        BVStatus status = Setup();
        if (status != BVStatus::BVSTATUS_OK)
        {
            std::exit(-1);
        }
    }

    ~BVService_Avahi()
    {
    }

    static std::shared_ptr<AvahiSimplePoll> MakeSimplePoll(AvahiSimplePoll* sp_p)
    {
        return std::shared_ptr<AvahiSimplePoll>(sp_p, avahi_simple_poll_free);
    }

    BVStatus Register() override;
};