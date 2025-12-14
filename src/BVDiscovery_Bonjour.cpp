#include "BVDiscovery_Bonjour.hpp"

// TODO: Get reply for service discovery and registration to other file
//       and test it (simulate reply from the daemon)

// This function should be put in a separate file. It is C DNS-SD API on top of mDNS.
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
                buff[i] = ' ';
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
            LinkedList_str* ll_p = (LinkedList_str*)context;
            LinkedListElement_str* lle_p = LinkedListElement_str_Constructor(buff, NULL);
            LinkedList_str_AddElement(ll_p, lle_p);
        }
    } else
    {
        fprintf(stderr, "An error occurred while receiving browse reply.");
    }
}
}

BVDiscovery_Bonjour::BVDiscovery_Bonjour(std::shared_ptr<const BVService_Bonjour>& _service_p,
                                         std::mutex& _discoveryQueueMutex,
                                         boost::asio::io_context& _ioContext,
                                         std::shared_ptr<std::queue<BVServiceBrowseInstance>> _discoveryQueue,
                                         std::condition_variable& _discoveryQueueCV,
                                         bool& _isDiscoveryQueueReady) :
    service_p(_service_p),
    discoveryQueueMutex(_discoveryQueueMutex),
    discoveryQueue_p(_discoveryQueue),
    ioContext(_ioContext),
    discoveryTimer(_ioContext),
    discoveryQueueCV(_discoveryQueueCV),
    isDiscoveryQueueReady(_isDiscoveryQueueReady)
{
    this->dnsRef = nullptr;
    this->c_ll_p = LinkedList_str_Constructor(NULL); // this is also utilized in BVDiscovery_Avahi
}

BVDiscovery_Bonjour::~BVDiscovery_Bonjour()
{
    LinkedList_str_Destructor(&this->c_ll_p);
    // When dnsRef is deallocated, service is no longer discoverable and browsing stops.
    DNSServiceRefDeallocate(this->dnsRef);
}

void BVDiscovery_Bonjour::StartBrowsing(void)
{
    /*
        DNSServiceBrowse is needed to be called exactly once.
        Browsing goes indefinitely, until the DNSServiceRef is passed to
        DNSServiceRefDeallocate.
    */
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

// StopBrowsing?

BVStatus BVDiscovery_Bonjour::ProcessDNSServiceBrowseResult()
{
    // DNSServiceProcessResult will block until there are no new services discovered.
    DNSServiceErrorType error = DNSServiceProcessResult(this->dnsRef); // blocks
    if (error != kDNSServiceErr_NoError) {
        std::cerr << "[ProcessDNSServiceBrowseResult] Encountered an error in DNSServiceBrowseResult: " << error << std::endl;
        return BVStatus::BVSTATUS_NOK; // setup a flag maybe?
    }

    std::cout << "timer scheduled" << std::endl;

    // TODO: How to stop this?

    std::unique_lock lk(discoveryQueueMutex);
    discoveryQueueCV.wait(lk, [this]{return !this->isDiscoveryQueueReady;});

    this->PushBrowsedServicesToQueue(); // critical section

    this->isDiscoveryQueueReady = true;
    lk.unlock();
    this->discoveryQueueCV.notify_one();
    // We could send a message to the thread now.
    // Upon receiving the message, the application could consume the queue and update its data.

    // Clear list after appending to queue.
    LinkedList_str_ClearList(this->c_ll_p);

    // if active => maybe check the message queue
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

// Will this function be used also in avahi?
void BVDiscovery_Bonjour::PushBrowsedServicesToQueue(void)
{
    // std::lock_guard<std::mutex> lock(this->discoveryQueueMutex);
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