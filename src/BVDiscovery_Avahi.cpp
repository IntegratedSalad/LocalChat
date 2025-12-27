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
                  std::mutex& _discoveryQueueMutex,
                  boost::asio::io_context& _ioContext, // needed?
                  std::shared_ptr<std::queue<BVServiceBrowseInstance>> _discoveryQueue,
                  std::condition_variable& _discoveryQueueCV,
                  bool& _isDiscoveryQueueReady,
                  const BVServiceHostData _hostData,
                  std::shared_ptr<AvahiSimplePoll> _simple_poll_p):
    client_p(std::move(_client_p)),
    simple_poll_p(_simple_poll_p),
    BVDiscovery(_hostData, 
                _discoveryQueueMutex,
                _discoveryQueue,
                _discoveryQueueCV,
                _isDiscoveryQueueReady)
{
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
    // any setup required...
}

void BVDiscovery_Avahi::Shutdown(void)
{

}


void BVDiscovery_Avahi::OnShutdown(void)
{

}


void BVDiscovery_Avahi::Start(void)
{

}


void BVDiscovery_Avahi::OnStart(void)
{

}


BVDiscovery_Avahi::~BVDiscovery_Avahi()
{   
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

void BVDiscovery_Avahi::run()
{
    this->CreateConnectionContext();
    avahi_simple_poll_loop(this->simple_poll_p.get());
}