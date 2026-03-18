#pragma once
#include "BV.hpp"
#include "BVDiscovery.hpp"
#include "dns_sd.h"
#include <mutex>
#include <memory>
#include <queue>
#include <condition_variable>
#include "linked_list.h"
#include "bonjour_api.h"

/*
 *   This class is a Bonjour implementation of BV Discovery Component.
 *   It utilizes a LL of records that is copied to the shared queue.
 */

/* Excerpt from dns_sd.h
 * Clients explicitly wishing to discover *only* LocalOnly services during a
 * browse may do this, without flags, by inspecting the interfaceIndex of each
 * service reported to a DNSServiceBrowseReply() callback function, and
 * discarding those answers where the interface index is not set to
 * kDNSServiceInterfaceIndexLocalOnly.
*/
class BVDiscovery_Bonjour : public BVDiscovery
{
private:
    // Is this really necessary to hold a shared pointer to service_p and not just a structure of needed params?
    // BVService_Bonjour component is alive in the main thread...
    // Probably not. Some manager class will have a pointer to the active service.
    DNSServiceRef dnsRef;
    BVStatus ProcessDNSServiceBrowseResult(void);

    void CreateConnectionContext(void) override; // private member function which actually starts the Discovery service
    void Setup(void) override;
    void run() override;

public:
    BVDiscovery_Bonjour(const BVServiceHostData _hostData);
    ~BVDiscovery_Bonjour() override;
};