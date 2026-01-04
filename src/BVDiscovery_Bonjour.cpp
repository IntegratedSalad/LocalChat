#include "BVDiscovery_Bonjour.hpp"

BVDiscovery_Bonjour::BVDiscovery_Bonjour(const BVServiceHostData _hostData,
                                         std::mutex& _discoveryQueueMutex,
                                         boost::asio::io_context& _ioContext,
                                         std::shared_ptr<std::queue<BVServiceBrowseInstance>> _discoveryQueue,
                                         std::condition_variable& _discoveryQueueCV,
                                         bool& _isDiscoveryQueueReady) :
    ioContext(_ioContext),
    discoveryTimer(_ioContext),
    BVDiscovery(_hostData, 
                _discoveryQueueMutex,
                _discoveryQueue,
                _discoveryQueueCV,
                _isDiscoveryQueueReady)

{
    this->dnsRef = nullptr;
}

void BVDiscovery_Bonjour::Setup(void)
{

}

// void BVDiscovery_Bonjour::OnShutdown(void)
// {

// }

// void BVDiscovery_Bonjour::OnStart(void)
// {

// }


BVDiscovery_Bonjour::~BVDiscovery_Bonjour()
{
    // When dnsRef is deallocated, browsing stops.
    // TODO: Think of it maybe being deallocated in a separate method for control
    DNSServiceRefDeallocate(this->dnsRef); 
}

void BVDiscovery_Bonjour::CreateConnectionContext(void)
{
    /*
        DNSServiceBrowse is needed to be called exactly once.
        Browsing goes indefinitely, until the DNSServiceRef is passed to
        DNSServiceRefDeallocate.
    */
    if (!this->GetIsBrowsingActive())
    {
        const BVServiceHostData hd = this->GetHostData();
        std::cout << "Browsing for: ";
        std::cout << hd.regtype << ".";
        std::cout << hd.domain  << std::endl; // Do not pass the pointer to service
        DNSServiceErrorType error = DNSServiceBrowse(&this->dnsRef,
                                                    0,
                                                    0,
                                                    hd.regtype.c_str(),
                                                    hd.domain.c_str(),
                                                    C_ServiceBrowseReply,
                                                    &this->GetLinkedList_p()); // TODO: how to pass the address of a pointer?
        if (!(error == kDNSServiceErr_NoError))
        {
            this->SetStatus(BVStatus::BVSTATUS_NOK);
            return;
        }
        this->SetIsBrowsingActive(true);
    } else
    {
        std::cout << "Browsing active..." << std::endl;
    }
}

// TODO: StopBrowsing

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
    // TODO: Before pushing onto queue check somehow if the service at the given
    //       servicename was already Registered!

    std::unique_lock lk(this->GetDiscoveryQueueMutex());
    this->GetDiscoveryQueueCV().wait(lk, [this]{return !this->GetIsDiscoveryQueueReady();});

    this->PushBrowsedServicesToQueue(); // critical section

    this->SetIsDiscoveryQueueReady(true);
    lk.unlock();
    this->GetDiscoveryQueueCV().notify_one();
    // We could send a message to the thread now.
    // Upon receiving the message, the application could consume the queue and update its data.

    // Clear list after appending to queue.
    LinkedList_str_ClearList(this->GetLinkedList_p());

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
    this->CreateConnectionContext();

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
    // Do we really need a delay?
}