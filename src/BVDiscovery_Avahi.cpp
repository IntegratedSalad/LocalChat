#include "BVDiscovery_Avahi.hpp"

extern "C" // These functions should be put in a separate file. It is C DNS-SD API on top of mDNS.
{
//client callback? client is not created here, but the daemon might call this function
// when there's problem with the client/client's state change
static void browse_callback(
    AvahiServiceBrowser* sb,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiBrowserEvent event,
    const char* name,
    const char* type,
    const char* domain,
    AVAHI_GCC_UNUSED AvahiLookupResultFlags flags,
    void* userdata) // pass simple poll...
{
    AvahiClient* c = (AvahiClient*)userdata;
    assert(sb);
    assert(c);

    switch (event)
    {
        case AVAHI_BROWSER_FAILURE:
        {
            fprintf(stderr, "(Browser) %s\n", 
                avahi_strerror(avahi_client_errno(avahi_service_browser_get_client(sb))));
                avahi_simple_poll_quit(simple_poll);
            return;
        }
    }
}
}

BVDiscovery_Avahi::BVDiscovery_Avahi(std::unique_ptr<AvahiClient, AvahiClientDeleter> _client_p,
                  std::mutex& _discoveryQueueMutex,
                  boost::asio::io_context& _ioContext, // needed?
                  std::shared_ptr<std::queue<BVServiceBrowseInstance>> _discoveryQueue,
                  std::condition_variable& _discoveryQueueCV,
                  bool& _isDiscoveryQueueReady,
                  const BVServiceHostData _hostData) :
    client_p(std::move(_client_p)),
    discoveryQueueMutex(_discoveryQueueMutex),
    discoveryQueue_p(_discoveryQueue),
    ioContext(_ioContext),
    discoveryTimer(_ioContext),
    discoveryQueueCV(_discoveryQueueCV),
    isDiscoveryQueueReady(_isDiscoveryQueueReady),
    hostData(_hostData)
{
    this->c_ll_p = LinkedList_str_Constructor(NULL);
    this->Setup();
}

void BVDiscovery_Avahi::Setup(void)
{
    /* We have to have the data from BVService here: type, hostname etc... */
    const std::string regtype = this->hostData.regtype;
    const std::string domain  = this->hostData.domain;
    if (client_p == nullptr)
    {
        throw std::bad_alloc();
    }
    this->serviceBrowser_p = std::unique_ptr<AvahiServiceBrowser, AvahiServiceBrowserDeleter>(
        avahi_service_browser_new(client_p.get(), 
                                  AVAHI_IF_UNSPEC, 
                                  AVAHI_PROTO_UNSPEC, 
                                  regtype.c_str(), 
                                  domain.c_str(), 
                                  (AvahiLookupFlags)0,
                                  browse_callback,
                                  client_p.get())
    );
}

BVDiscovery_Avahi::~BVDiscovery_Avahi()
{   
    LinkedList_str_Destructor(&this->c_ll_p);
}

void BVDiscovery_Avahi::StartBrowsing(void)
{

}

BVStatus BVDiscovery_Avahi::ProcessDNSServiceBrowseResult(void)
{

}

void BVDiscovery_Avahi::PushBrowsedServicesToQueue(void)
{

}

void BVDiscovery_Avahi::run()
{

}