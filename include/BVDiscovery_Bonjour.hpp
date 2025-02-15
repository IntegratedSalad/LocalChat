#pragma once
#include "BV.hpp"
#include "BVDiscovery.hpp"
#include "BVService_Bonjour.hpp"
#include "dns_sd.h"
#include <mutex>
#include <memory>
#include <list>

/*
    This class will utilize an array of records.
    Access R/W has to be synchronized by a mutex!
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
    std::shared_ptr<const BVService_Bonjour> service_p;
    DNSServiceRef dnsRef;
    std::mutex& rwListMutex;
    std::shared_ptr<std::list<BVServiceBrowseInstance>> discoveryList_p; // probably we will have to
    // allocate the memory for this list outside BVDiscovery_Bonjour, because this is another thread,
    // so passing a pointer to memory to another thread might point to not the same thing in this other thread.

    boost::asio::io_context& ioContext;
    boost::asio::steady_timer discoveryTimer;
    void StartBrowsing(void);

    BVStatus ProcessDNSServiceBrowseResult(void); // this method should update the list

    BVStatus status = BVStatus::BV_STATUS_IN_PROGRESS;
    bool isBrowsingActive = false;

public:
    BVDiscovery_Bonjour(std::shared_ptr<const BVService_Bonjour>& _service_p, std::mutex& _mutex,
                        boost::asio::io_context& _ioContext);
    ~BVDiscovery_Bonjour();

    void run() override;
};