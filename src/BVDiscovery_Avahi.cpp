#include "BVDiscovery_Avahi.hpp"

extern "C" // These functions should be put in a separate file. It is C DNS-SD API on top of mDNS.
{
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

static void browse_callback(
    AvahiServiceBrowser* sb,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiBrowserEvent event,
    const char* name,
    const char* type,
    const char* domain,
    AVAHI_GCC_UNUSED AvahiLookupResultFlags flags,
    void* userdata)
{
    BVDiscovery_Avahi* discovery_p = static_cast<BVDiscovery_Avahi*>(userdata);
    // If this will be problematic, just pass intermediate structure

    switch (event)
    {
        case AVAHI_BROWSER_FAILURE:
        {
            fprintf(stderr, "(Browser) %s\n", 
                avahi_strerror(avahi_client_errno(avahi_service_browser_get_client(sb))));
                avahi_simple_poll_quit(discovery_p->GetSimplePoll());
            return;
        }
        case AVAHI_BROWSER_NEW:
        {
            // Add discovered services to the linked list
            setbuf(stdout, NULL);
            printf("Found %s.%s in %s!\n", name, type, domain);

            // todo: put this in a separate function
            char buff[N_BYTES_SERVICE_STR_TOTAL];
            const size_t servLen = strlen(name);
            const size_t regLen = strlen(type);
            const size_t replDmnLen = strlen(domain);
            for (int i = 0; i < N_BYTES_SERVICE_STR_TOTAL; i++)
            {
                buff[i] = ' ';
            }
                if (servLen < N_BYTES_SERVNAME_MAX)
            {
                memcpy(buff, name, servLen);
            }
            if (regLen < N_BYTES_REGTYPE_MAX)
            {
                memcpy(buff + N_BYTES_SERVNAME_MAX, type, regLen);
            }
            if (replDmnLen < N_BYTES_REPLDOMN_MAX)
            {
                memcpy(buff + N_BYTES_SERVNAME_MAX + N_BYTES_REGTYPE_MAX, domain, replDmnLen);
            }
            buff[N_BYTES_SERVICE_STR_TOTAL-1] = '\0';
            LinkedListElement_str* lle_p = LinkedListElement_str_Constructor(buff, NULL);
            LinkedList_str_AddElement(discovery_p->GetLinkedList(), lle_p);

            // We must call the member function that is responsible for enqueing the results
            discovery_p->AvahiOnServiceNewWrapper(); // calls PushBrowsedServicesToQueue
            break;
        }
        case AVAHI_BROWSER_REMOVE:
        {
            fprintf(stdout, "Service was removed... TODO \n");
            break;
        }
        case AVAHI_BROWSER_ALL_FOR_NOW:
        case AVAHI_BROWSER_CACHE_EXHAUSTED:
        {
            fprintf(stdout, "Cache exhuast or all for now... TODO");
            break;
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
                  const BVServiceHostData _hostData,
                  std::shared_ptr<AvahiSimplePoll> _simple_poll_p):
    client_p(std::move(_client_p)),
    discoveryQueueMutex(_discoveryQueueMutex),
    discoveryQueue_p(_discoveryQueue),
    ioContext(_ioContext),
    discoveryTimer(_ioContext),
    discoveryQueueCV(_discoveryQueueCV),
    isDiscoveryQueueReady(_isDiscoveryQueueReady),
    hostData(_hostData),
    simple_poll_p(_simple_poll_p)
{
    this->c_ll_p = LinkedList_str_Constructor(NULL);
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

BVDiscovery_Avahi::~BVDiscovery_Avahi()
{   
    LinkedList_str_Destructor(&this->c_ll_p);
}
// This essentially creates a connection context
// and sets up the callback
void BVDiscovery_Avahi::CreateConnectionContext(void)
{
    const std::string regtype = this->hostData.regtype;
    const std::string domain  = this->hostData.domain;
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

void BVDiscovery_Avahi::PushBrowsedServicesToQueue(void)
{
    for (const LinkedListElement_str* lle_p = this->c_ll_p->head_p;
        lle_p != NULL;)
    {
        BVServiceBrowseInstance bI; // put on heap? No, STL containers have elements allocated on heap.
        std::string regType(lle_p->data + N_BYTES_SERVNAME_MAX, N_BYTES_REGTYPE_MAX);
        std::string replyDomain(lle_p->data + N_BYTES_SERVNAME_MAX + N_BYTES_REGTYPE_MAX, N_BYTES_REPLDOMN_MAX);
        std::string serviceName(lle_p->data, N_BYTES_SERVNAME_MAX);

        regType.erase(std::remove(regType.begin(), regType.end(), ' '), regType.end());
        replyDomain.erase(std::remove(replyDomain.begin(), replyDomain.end(), ' '), replyDomain.end());
        serviceName.erase(std::remove(serviceName.begin(), serviceName.end(), ' '), serviceName.end());

        bI.regType = regType;
        bI.replyDomain = replyDomain;
        bI.serviceName = serviceName;
        this->discoveryQueue_p->push(bI);
        lle_p = lle_p->next_p;
    }
}

void BVDiscovery_Avahi::run()
{
    this->CreateConnectionContext();
    avahi_simple_poll_loop(this->simple_poll_p.get());
}