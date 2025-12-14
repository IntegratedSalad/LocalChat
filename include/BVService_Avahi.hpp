#pragma once
#include <iostream>
#include <algorithm>
#include <memory>
#include <utility>
#include "BVService.hpp"
#include <avahi-client/client.h>
#include <avahi-client/publish.h>
#include <avahi-common/simple-watch.h>

// Because of the AvahiSimplePoll needed to be accessed from single thread only,
// maybe we create a manager object, that has exclusive ownership of the AvahiSimplePoll?

struct AvahiClientDeleter
{
    void operator()(AvahiClient* p)
    {
        avahi_client_free(p);
    }
};

static AvahiEntryGroup* group = NULL;
class BVService_Avahi : public BVService
{
private:

    /* 
     * Avahi client functions as a connection context (maybe something similar to DNSServiceRef)
     * Calling avahi_client_free() frees entry group object (service)
     * This must not be null until the termination of the service when User closes the application.
     * Later, BVService_Avahi must transfer the ownership of this pointer to BVDiscovery_Avahi
     * But why? When Browsing frees the memory and dies, why should the entire connection context be deleted?
     * But maybe browsing is never deleted until the User quits entire application?
     * Maybe stopping browsing only stops the thread execution of a Browsing thread?
     * Client will not be destroyed because we will not destroy the BVDiscovery_Avahi object.
     * We will only stop the execution. There's no need for the whole object to be destroyed.
    */
    std::unique_ptr<AvahiClient, AvahiClientDeleter> client_p = nullptr;
    std::shared_ptr<AvahiSimplePoll> simple_poll_p = nullptr; // for now, 

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

    std::unique_ptr<AvahiClient, AvahiClientDeleter> TransferClient(void)
    {
        return std::move(this->client_p);
    }

    BVStatus Register() override;
};

