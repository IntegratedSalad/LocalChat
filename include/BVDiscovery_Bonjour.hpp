#pragma once
#include "BV.hpp"
#include "BVDiscovery.hpp"
#include "dns_sd.h"
#include <mutex>
#include <memory>
#include <list>
#include <boost/asio.hpp>

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
    DNSServiceRef dnsRef;
    std::mutex& rwListMutex;
    std::unique_ptr<std::list<BVServiceBrowseInstance>> discoveryList_p; // probably we will have to
    // allocate the memory for this list outside BVDiscovery_Bonjour, because this is another thread,
    // so passing a pointer to memory to another thread might point to not the same thing in this other thread.

    boost::asio::io_context ioContext;

    void Discover();
    BVStatus ProcessDNSServiceBrowseResult();

public:
    BVDiscovery_Bonjour(DNSServiceRef ref, std::mutex& _mutex);
    ~BVDiscovery_Bonjour();
    
    void run() override;

};