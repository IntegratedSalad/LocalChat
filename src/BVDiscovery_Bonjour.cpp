#include "BVDiscovery_Bonjour.hpp"

// How to return the value serviceName?
// Maybe pass something to context?
extern "C"
{
#include <stdio.h>
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
    if (errorCode == kDNSServiceErr_NoError)
    {
        printf("Found %s.%s in %s!\n", serviceName, regtype, replyDomain);
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
}

BVDiscovery_Bonjour::~BVDiscovery_Bonjour()
{
}

void BVDiscovery_Bonjour::StartBrowsing(void)
{   
    if (this->dnsRef && !this->isBrowsingActive)
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
                                                    NULL);
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
        std::cerr << "Encountered an error in DNSServiceBrowseResult: " << error << std::endl;
        return BVStatus::BVSTATUS_NOK; // setup a flag maybe?
    }

    std::cout << "timer scheduled" << std::endl;
    this->discoveryTimer.expires_after(std::chrono::seconds(DISCOVERY_TIMER_TRIGGER_S));
    this->discoveryTimer.async_wait([this](const boost::system::error_code& /*e*/)
    {
        this->ProcessDNSServiceBrowseResult();
    });
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