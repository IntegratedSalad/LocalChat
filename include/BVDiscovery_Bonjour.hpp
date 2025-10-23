#pragma once
#include "BV.hpp"
#include "BVDiscovery.hpp"
#include "BVService_Bonjour.hpp"
#include "dns_sd.h"
#include <mutex>
#include <memory>
#include <queue>
#include <condition_variable>
#include "linked_list.h"

#define N_BYTES_SERVNAME_MAX      24
#define N_BYTES_REGTYPE_MAX       24
#define N_BYTES_REPLDOMN_MAX      16
#define N_BYTES_SERVICE_STR_TOTAL (N_BYTES_SERVNAME_MAX + N_BYTES_REGTYPE_MAX + N_BYTES_REPLDOMN_MAX)
#define N_SERVICES_MAX            32

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
    // TODO: is this really necessary to hold a shared pointer to service_p and not just a structure of needed params?
    // BVService_Bonjour component is alive in the main thread...
    std::shared_ptr<const BVService_Bonjour> service_p;
    DNSServiceRef dnsRef; // TODO: shouldn't this be in BVDiscovery? DNSServiceRef should be allocated no matter the implementation
    std::mutex& queueMutex;
    std::condition_variable& discoveryQueueCV;

    std::shared_ptr<std::queue<BVServiceBrowseInstance>> discoveryQueue_p;
    boost::asio::io_context& ioContext;
    boost::asio::steady_timer discoveryTimer;
    void StartBrowsing(void);
    BVStatus ProcessDNSServiceBrowseResult(void); // this method should update the list. TODO: This method should also be in the BVDiscovery parent class.
    void PushBrowsedServicesToQueue(void);

    BVStatus status = BVStatus::BVSTATUS_IN_PROGRESS; // TODO: This parameter should be in BVDiscovery parent class.
    bool isBrowsingActive = false;
    LinkedList_str* c_ll_p = NULL; // C linked list, for processing daemon responses

public:
    BVDiscovery_Bonjour(std::shared_ptr<const BVService_Bonjour>& _service_p,
                        std::mutex& _queueMutex,
                        boost::asio::io_context& _ioContext,
                        std::shared_ptr<std::queue<BVServiceBrowseInstance>> _discoveryQueue,
                        std::condition_variable& _discoveryQueueCV);
    ~BVDiscovery_Bonjour();

    void run() override;
};