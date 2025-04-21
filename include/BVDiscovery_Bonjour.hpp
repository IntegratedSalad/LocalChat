#pragma once
#include "BV.hpp"
#include "BVDiscovery.hpp"
#include "BVService_Bonjour.hpp"
#include "dns_sd.h"
#include <mutex>
#include <memory>
#include <list>
#include "linked_list.h"

#define N_BYTES_SERVNAME_MAX      24
#define N_BYTES_REGTYPE_MAX       24
#define N_BYTES_REPLDOMN_MAX      16
#define N_BYTES_SERVICE_STR_TOTAL (N_BYTES_SERVNAME_MAX + N_BYTES_REGTYPE_MAX + N_BYTES_REPLDOMN_MAX)
#define N_SERVICES_MAX            32

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
    // allocate the memory for this list outside BVDiscovery_Bonjour in the main thread, because this is another thread,
    // so passing a pointer to memory to another thread might point to not the same thing in this other thread.

    BVServiceBrowseInstance browseInstance;

    boost::asio::io_context& ioContext;
    boost::asio::steady_timer discoveryTimer;
    void StartBrowsing(void);

    BVStatus ProcessDNSServiceBrowseResult(void); // this method should update the list

    BVStatus status = BVStatus::BV_STATUS_IN_PROGRESS;
    bool isBrowsingActive = false;

    // maybe linked list? because now I need to know the current_service_num
    // but it will require allocating it on the heap.
    // But - current_service_num will be used only to create queue.
    // char discoveryResult_carr[N_SERVICES_MAX][N_BYTES_SERVICE_STR_TOTAL];
    // unsigned int current_service_num = 0;

    LinkedList_str* c_ll_p = NULL; // C linked list, for processing daemon response

public:
    BVDiscovery_Bonjour(std::shared_ptr<const BVService_Bonjour>& _service_p, std::mutex& _mutex,
                        boost::asio::io_context& _ioContext);
    ~BVDiscovery_Bonjour();

    void run() override;
};