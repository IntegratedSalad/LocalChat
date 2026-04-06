#pragma once
#include "BV.hpp"
#include "BVDiscovery.hpp"
#include "BVAvahi_Common.hpp"
#include "BVComponent.hpp"
#include "BVLoggable.hpp"
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

class BVDiscovery_Avahi : public BVDiscovery,
                          public BVComponent,
                          public BVLoggable
{
private:
    std::unique_ptr<AvahiClient, AvahiClientDeleter> client_p = nullptr;
    std::unique_ptr<AvahiServiceBrowser, AvahiServiceBrowserDeleter> serviceBrowser_p = nullptr;
    std::shared_ptr<AvahiSimplePoll> simple_poll_p;
    std::atomic<bool> isPaused = false;
    std::condition_variable pauseCV;
    std::mutex pauseMutex;

    void CreateConnectionContext(void) override;
    void Setup(void) override;
    void Browse(void) override;

public:
    BVDiscovery_Avahi(std::unique_ptr<AvahiClient, AvahiClientDeleter> _client_p,
                      boost::asio::io_context& _ioContext, // probably not needed
                      const BVServiceHostData _hostData,
                      std::shared_ptr<AvahiSimplePoll> _simple_poll_p,
                      std::shared_ptr<threadsafe_queue<BVMessage>> _outMbx,
                      std::shared_ptr<threadsafe_queue<BVMessage>> _inMbx);

    void OnServiceFound(void)
    {
        using BVServiceBrowseInstanceList = std::list<BVServiceBrowseInstance>;
        BVServiceBrowseInstanceList browseInstanceList = ReturnListFromBrowseResults();
        SendMessage(BVMessage(
                        BVEventType::BVEVENTTYPE_APP_PUBLISHED_SERVICE, 
                            std::make_unique<std::any>(std::make_any<BVServiceBrowseInstanceList>(browseInstanceList))));
        LinkedList_str_ClearList(this->GetLinkedList_p());
    }

    AvahiSimplePoll* GetSimplePoll(void)
    {
        return this->simple_poll_p.get();
    }

    bool GetIsPaused(void)
    {
        return this->isPaused;
    }

    void SetIsPaused(const bool _isPaused)
    {
        this->isPaused = _isPaused;
    }

    void Pause(void)
    {
        std::unique_lock<std::mutex> lk(pauseMutex);
        pauseCV.wait(lk, [this]{return !this->isPaused;});
    }

    BVStatus OnStart(std::unique_ptr<std::any>) override;
    BVStatus OnPause(std::unique_ptr<std::any>) override;
    BVStatus OnResume(std::unique_ptr<std::any>) override;
    BVStatus OnRestart(std::unique_ptr<std::any>) override;
    BVStatus OnShutdown(std::unique_ptr<std::any>) override;

    ~BVDiscovery_Avahi();
};
