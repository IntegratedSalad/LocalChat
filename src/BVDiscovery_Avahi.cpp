#include "BVDiscovery_Avahi.hpp"

//client callback? client is not created here, but the daemon might call this function
// when there's problem with the client/client's state change
#include <stdio.h>
#include <string.h>
// typedef struct BrowseData
// {
//     AvahiClient* client_p;
//     AvahiSimplePoll* simplepoll_p;
//     BVDiscovery_Avahi* d_p;
// } BrowseData;

BVDiscovery_Avahi::BVDiscovery_Avahi(std::unique_ptr<AvahiClient, AvahiClientDeleter> _client_p,
                                     boost::asio::io_context& _ioContext, // probably not needed
                                     const BVServiceHostData _hostData,
                                     std::shared_ptr<AvahiSimplePoll> _simple_poll_p,
                                     std::shared_ptr<threadsafe_queue<BVMessage>> _outMbx,
                                     std::shared_ptr<threadsafe_queue<BVMessage>> _inMbx):
    client_p(std::move(_client_p)),
    simple_poll_p(_simple_poll_p),
    BVDiscovery(_hostData),
    BVComponent(_outMbx, _inMbx)
{
    RegisterCallback(BVEventType::BVEVENTTYPE_DISCOVERY_REQUEST_START,
                     std::bind(&BVDiscovery_Avahi::OnStart, this, std::placeholders::_1));

    RegisterCallback(BVEventType::BVEVENTTYPE_DISCOVERY_REQUEST_PAUSE, 
                     std::bind(&BVDiscovery_Avahi::OnPause, this, std::placeholders::_1));
    
    RegisterCallback(BVEventType::BVEVENTTYPE_DISCOVERY_REQUEST_RESUME, 
                     std::bind(&BVDiscovery_Avahi::OnResume, this, std::placeholders::_1));

    RegisterCallback(BVEventType::BVEVENTTYPE_TERMINATE_ALL,
                     std::bind(&BVDiscovery_Avahi::OnShutdown, this, std::placeholders::_1));

    RegisterCallback(BVEventType::BVEVENTTYPE_DISCOVERY_REQUEST_SHUTDOWN,
                     std::bind(&BVDiscovery_Avahi::OnShutdown, this, std::placeholders::_1));

    RegisterCallback(BVEventType::BVEVENTTYPE_DISCOVERY_REQUEST_RESTART,
                     std::bind(&BVDiscovery_Avahi::OnRestart, this, std::placeholders::_1));
    
    RegisterCallback(BVEventType::BVEVENTTYPE_DISCOVERY_REQUEST_RESOLVE,
                     std::bind(&BVDiscovery_Avahi::OnResolveRequest, this, std::placeholders::_1));

    this->Setup();
}

void BVDiscovery_Avahi::Setup(void)
{
    if (this->client_p == nullptr || this->simple_poll_p == nullptr)
    {
        std::cerr << "(Avahi) contexts are dead\n client_p:\t" << this->client_p.get()
        << "\nsimple_poll_p:\t" << this->simple_poll_p.get()
        << std::endl;
        throw std::bad_alloc();
    }
    this->CreateConnectionContext();
}


BVStatus BVDiscovery_Avahi::OnStart(std::unique_ptr<std::any> dp)
{
    return BVStatus::BVSTATUS_OK;
}

BVStatus BVDiscovery_Avahi::OnPause(std::unique_ptr<std::any> dp)
{
    this->SetIsPaused(true);
    LogTrace("Discovery: Pausing...");
    avahi_simple_poll_wakeup(this->GetSimplePoll());
    return BVStatus::BVSTATUS_OK;
}

BVStatus BVDiscovery_Avahi::OnResume(std::unique_ptr<std::any> dp)
{
    this->SetIsPaused(false);
    LogTrace("Discovery: Resuming...");
    this->pauseCV.notify_all();
    return BVStatus::BVSTATUS_OK;
}

BVStatus BVDiscovery_Avahi::OnRestart(std::unique_ptr<std::any> dp)
{
    return BVStatus::BVSTATUS_OK;
}

BVStatus BVDiscovery_Avahi::OnShutdown(std::unique_ptr<std::any> dp)
{
    this->SetIsBrowsingActive(false);
    LogTrace("Discovery: Shutting down...");
    return BVStatus::BVSTATUS_OK;
}

BVStatus BVDiscovery_Avahi::ResolveService(const BVServiceBrowseInstance& bI)
{
    /*
     * Avahi seems to be different in how to handle resolution.
     * App does not need to ask Discovery - instead, resolver
     * is created in the browse callback.
     *
    */
    return BVStatus::BVSTATUS_OK;
}

// This essentially creates a connection context
// and sets up the callback
void BVDiscovery_Avahi::CreateConnectionContext(void)
{
    const std::string regtype = this->GetHostData().regtype;
    const std::string domain  = this->GetHostData().domain;
    this->serviceBrowser_p = std::unique_ptr<AvahiServiceBrowser, AvahiServiceBrowserDeleter>(
        avahi_service_browser_new(client_p.get(), 
                                  AVAHI_IF_UNSPEC, 
                                  AVAHI_PROTO_UNSPEC, 
                                  regtype.c_str(),
                                  domain.c_str(),
                                  (AvahiLookupFlags)0,
                                  browse_callback,
                                  this)
    );
}

// void DestroyConnectionContext

// This is put on a separate thread
void BVDiscovery_Avahi::Browse()
{
    this->SetIsBrowsingActive(true);
    while (this->GetIsBrowsingActive())
    {
        if (!this->GetIsPaused())
        {
            int status = avahi_simple_poll_iterate(this->GetSimplePoll(),
                                                   AVAHI_POLL_ITERATE_TIMEOUT_MS);
            if (status == -1)
            {
                const std::string err_s("avahi_simple_poll_iterate returned -1");
                throw std::runtime_error(err_s);
            }
        } else
        {
            Pause();
        }
    }
}


BVDiscovery_Avahi::~BVDiscovery_Avahi()
{   
}