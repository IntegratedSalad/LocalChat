#pragma once
#include "BV.hpp"
#include "BVDiscovery.hpp"
#include "BVService_Avahi.hpp"
#include <memory>
#include <mutex>
#include <queue>
#include <condition_variable>
#include "linked_list.h"
#include "avahi_api.h"
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

    void CreateConnectionContext(void) override;
    void Setup(void) override;
    void run() override;

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

    void OnServiceFound(void)
    {
        std::unique_lock lk(this->GetDiscoveryQueueMutex());
        this->GetDiscoveryQueueCV().wait(lk, [this]{return !this->GetIsDiscoveryQueueReady();});
        this->PushBrowsedServicesToQueue();
        this->SetIsDiscoveryQueueReady(true);
        lk.unlock();
        this->GetDiscoveryQueueCV().notify_one();
        LinkedList_str_ClearList(this->GetLinkedList_p());
    }

    AvahiSimplePoll* GetSimplePoll(void)
    {
        return this->simple_poll_p.get();
    }

    void Shutdown() override;
    void OnShutdown() override;
    void Start() override;
    void OnStart() override;
};

// I think now - create a file
// avahi_c_api_bridge.cpp, where we define 