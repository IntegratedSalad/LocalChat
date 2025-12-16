#include "BV.hpp"
#include "BVDiscovery.hpp"
#include "BVService_Avahi.hpp"
#include <memory>
#include <mutex>
#include <queue>
#include <condition_variable>
#include "linked_list.h"
#include <avahi-client/client.h>
#include <avahi-client/lookup.h>
#include <avahi-common/error.h>
#include <avahi-common/simple-watch.h>

struct AvahiServiceBrowserDeleter
{
    void operator()(AvahiServiceBrowser* p)
    {
        avahi_service_browser_free(p);
    }
};

class BVDiscovery_Avahi : public BVDiscovery
{
private:
    std::unique_ptr<AvahiClient, AvahiClientDeleter> client_p = nullptr;
    std::unique_ptr<AvahiServiceBrowser, AvahiServiceBrowserDeleter> serviceBrowser_p = nullptr;
    std::shared_ptr<AvahiSimplePoll> simple_poll_p;

    const BVServiceHostData hostData;

    std::mutex& discoveryQueueMutex;
    std::condition_variable& discoveryQueueCV;
    std::shared_ptr<std::queue<BVServiceBrowseInstance>> discoveryQueue_p;
    bool& isDiscoveryQueueReady;

    boost::asio::io_context& ioContext;
    boost::asio::steady_timer discoveryTimer;
    void CreateConnectionContext(void);
    // void DestroyConnectionContext
    void PushBrowsedServicesToQueue(void);

    BVStatus status = BVStatus::BVSTATUS_IN_PROGRESS; // TODO: This parameter should be in BVDiscovery parent class.
    bool isBrowsingActive = false;
    LinkedList_str* c_ll_p = NULL; // C linked list, for processing daemon responses
    // TODO: LinkedList_str should be wrapped in a unique ptr.

    void Setup(void);

public:
    BVDiscovery_Avahi(std::unique_ptr<AvahiClient, AvahiClientDeleter> _client_p,
                      std::mutex& _discoveryQueueMutex,
                      boost::asio::io_context& _ioContext, // needed?
                      std::shared_ptr<std::queue<BVServiceBrowseInstance>> _discoveryQueue,
                      std::condition_variable& _discoveryQueueCV,
                      bool& _isDiscoveryQueueReady,
                      const BVServiceHostData _hostData,
                      std::shared_ptr<AvahiSimplePoll> _simple_poll_p);
    ~BVDiscovery_Avahi();

    void AvahiOnServiceNewWrapper(void)
    {
        this->PushBrowsedServicesToQueue();
    }

    LinkedList_str* GetLinkedList(void)
    {
        return this->c_ll_p;
    }

    void run() override;
};