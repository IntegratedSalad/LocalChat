#include "BVDiscovery_Bonjour.hpp"

// TODO: Get reply for service discovery and registration to other file
//       and test it (simulate reply from the daemon)

// This global variable can be removed.
// One solution is to allocate LinkedList
// and pass it into C_ServiceBrowseReply' context pointer.
// Then, ProcessDNSServiceBrowseResult can use it to append discovery results to the queue
unsigned int current_service_num = 0;
extern "C"
{
#include <stdio.h>
#include <string.h>
void C_ServiceBrowseReply(
    DNSServiceRef sdRef,
    DNSServiceFlags flags,
    uint32_t interfaceIndex,
    DNSServiceErrorType errorCode,
    const char *serviceName,
    const char *regtype,
    const char *replyDomain,
    void *context)
{
    setbuf(stdout, NULL);
    if (errorCode == kDNSServiceErr_NoError)
    {
        printf("Found %s.%s in %s!\n", serviceName, regtype, replyDomain);
        if (context != NULL)
        {
            char buff[N_BYTES_SERVICE_STR_TOTAL];
            const size_t servLen = strlen(serviceName);
            const size_t regLen = strlen(regtype);
            const size_t replDmnLen = strlen(replyDomain);
            for (int i = 0; i < N_BYTES_SERVICE_STR_TOTAL; i++)
            {
                buff[i] = 'X';
            }
            if (servLen < N_BYTES_SERVNAME_MAX)
            {
                memcpy(buff, serviceName, servLen);
            }
            if (regLen < N_BYTES_REGTYPE_MAX)
            {
                memcpy(buff + N_BYTES_SERVNAME_MAX, regtype, regLen);
            }
            if (replDmnLen < N_BYTES_REPLDOMN_MAX)
            {
                memcpy(buff + N_BYTES_SERVNAME_MAX + N_BYTES_REGTYPE_MAX, replyDomain, replDmnLen);
            }
            buff[N_BYTES_SERVICE_STR_TOTAL-1] = '\0';
            /* Add element to linked list */
            LinkedList_str* ll_p = (LinkedList_str*)context;
            LinkedListElement_str* lle_p = LinkedListElement_str_Constructor(buff, NULL);
            LinkedList_str_AddElement(ll_p, lle_p);
        }
        // void* context should be a queue or a function pointer?
        // void* context should be a dynamically/statically allocated char array (64 bytes will suffice i think)
        // Then, because DNSServiceProcessResult will block until the daemon replies,
        // and it replies only when there are new services being registered into the network,
        // we add the queue, so discovery acts as a producer of the discovery queue.
        // I think that whenever there are multiple of instance entries,
        // daemon replies over and over until theres no new instance.
        // We have to make an array of char*.

    } else
    {
        fprintf(stderr, "An error occurred while receiving browse reply.");
    }
}
}

BVDiscovery_Bonjour::BVDiscovery_Bonjour(std::shared_ptr<const BVService_Bonjour>& _service_p, std::mutex& _mutex,
                                         boost::asio::io_context& _ioContext) 
    : service_p(_service_p), rwListMutex(_mutex), ioContext(_ioContext), discoveryTimer(_ioContext)
{
    this->discoveryList_p = std::make_shared<std::list<BVServiceBrowseInstance>>();
    this->dnsRef = nullptr;

    // C list alloc
    this->c_ll_p = LinkedList_str_Constructor(NULL);
}

BVDiscovery_Bonjour::~BVDiscovery_Bonjour()
{
    // C list dealloc
    LinkedList_str_Destructor(&this->c_ll_p);
}

void BVDiscovery_Bonjour::StartBrowsing(void)
{   
    // TODO: Pass something, to which when the daemon replies, we can pass data (context)
    if (!this->isBrowsingActive)
    {
        std::cout << "Browsing for: ";
        std::cout << this->service_p->GetRegType() << ".";
        std::cout << this->service_p->GetDomain() << std::endl;
        DNSServiceErrorType error = DNSServiceBrowse(&this->dnsRef,
                                                    0,
                                                    0,
                                                    this->service_p->GetRegType().c_str(),
                                                    this->service_p->GetDomain().c_str(),
                                                    C_ServiceBrowseReply,
                                                    &this->c_ll_p);
        if (!(error == kDNSServiceErr_NoError))
        {
            this->status = BVStatus::BVSTATUS_NOK;
            return;
        }
        this->isBrowsingActive = true;
    } else
    {
        std::cout << "Browsing active..." << std::endl;
    }
}

BVStatus BVDiscovery_Bonjour::ProcessDNSServiceBrowseResult()
{
    DNSServiceErrorType error = DNSServiceProcessResult(this->dnsRef);
    if (error != kDNSServiceErr_NoError) {
        std::cerr << "[ProcessDNSServiceBrowseResult] Encountered an error in DNSServiceBrowseResult: " << error << std::endl;
        return BVStatus::BVSTATUS_NOK; // setup a flag maybe?
    }

    std::cout << "timer scheduled" << std::endl;

    // Process the queue... ( do not block if empty )
    // See if there's something in the array

    LinkedListElement_str* head_copy_p = this->c_ll_p->head_p;

    // TODO: copy this data into queue.

    this->discoveryTimer.expires_after(std::chrono::seconds(DISCOVERY_TIMER_TRIGGER_S));
    this->discoveryTimer.async_wait([this](const boost::system::error_code& /*e*/)
    {
        this->ProcessDNSServiceBrowseResult();
    });

    return BVStatus::BVSTATUS_OK;
}

void BVDiscovery_Bonjour::run()
{
    std::cout << "Scheduling the timer..." << std::endl;
    this->StartBrowsing();

    // if !replyError
    std::cout << "timer scheduled" << std::endl;
    this->discoveryTimer.expires_after(std::chrono::seconds(DISCOVERY_TIMER_TRIGGER_S));
    this->discoveryTimer.async_wait([this](const boost::system::error_code& /*e*/)
    {
        this->ProcessDNSServiceBrowseResult();

    });

    this->ioContext.run();

    // Define a timer (5s)
    // Schedule this timer over and over while performing Discovery
    // In the callback, pass ProcessDNSServiceBrowseResult
    // it will call DNSServiceBrowse, and somehow write back
    // the reply from the daemon - it will be a service name
    // with a regtype and domain

}