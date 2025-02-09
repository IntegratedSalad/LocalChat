#include "BVService_Bonjour.hpp"

bool shouldProcess = true; // TODO: should this be atomic?
bool replyError = false;
extern "C"
{
#include "stdio.h"
void C_RegisterReply(
    DNSServiceRef sdRef,
    DNSServiceFlags flags,
    DNSServiceErrorType errorCode,
    const char *name,
    const char *regtype,
    const char *domain,
    void *context) // maybe pass the pointer to shouldProcess here?
{
    if (errorCode == kDNSServiceErr_NoError) {
        printf("--- Registered %s, as %s, in %s!", name, regtype, domain);
    } else {
        fprintf(stderr, "An error occurred while trying to register %s", name);
        replyError = true;
    }
    shouldProcess = false;
}
}

BVStatus BVService_Bonjour::Register(boost::asio::io_context& ioContext)
{
    BVStatus status = BVStatus::BVSTATUS_OK;
    DNSServiceErrorType error = DNSServiceRegister(&this->dnsRef,
                                                    0,
                                                    0,
                                                    this->GetHostname().c_str(),
                                                    this->GetRegType().c_str(),
                                                    NULL,
                                                    NULL,
                                                    this->GetPort(),
                                                    0,
                                                    NULL,
                                                    C_RegisterReply,
                                                    NULL);

    if (error != kDNSServiceErr_NoError) {
        std::cerr << "Encountered an error: " << error << std::endl;
        return BVStatus::BVSTATUS_NOK;
    }

    // Check results with daemon
    // Create a steady timer for processing DNS-SD results
    boost::asio::steady_timer timer(ioContext);
    timer.expires_after(std::chrono::milliseconds(100));
    /*
        asio::steady_timer::async_wait performs execution of the function
        periodically (for set time).
        When the timer expires, the completion handler (the functor/callback passed) is called
        once.
        
        Schedule a timer to be run 
     */
    timer.async_wait([&timer, &ioContext, this](const boost::system::error_code& /*e*/)
    {
        // Here, this case doesn't use boost::system:error_code& e,
        // but this prototype satisfies the required functor prototype.
        this->ProcessDNSServiceResults(&timer, &ioContext);
    });
    
    return status;
}

BVStatus BVService_Bonjour::ProcessDNSServiceResults(
    boost::asio::steady_timer* timer, boost::asio::io_context* ioContext)
{
    DNSServiceErrorType error = DNSServiceProcessResult(this->dnsRef);
    if (error != kDNSServiceErr_NoError) {
        std::cerr << "Encountered an error in DNSServiceProcessResult: " << error << std::endl;
        return BVStatus::BVSTATUS_NOK;
    }

    if (!shouldProcess)
    {
        if (replyError) return BVStatus::BVSTATUS_NOK;
        else return BVStatus::BVSTATUS_OK;
    }

    // Schedule this again
    timer->expires_after(std::chrono::milliseconds(100));
    timer->async_wait([&timer, &ioContext, this](const boost::system::error_code& /*e*/)
    {
        this->ProcessDNSServiceResults(timer, ioContext);
    });

    return BVStatus::BVSTATUS_OK; 
}