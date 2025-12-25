#include "BVService_Bonjour.hpp"

bool replyError = false;
BVStatus BVService_Bonjour::Register()
{
    BVStatus status = BVStatus::BVSTATUS_IN_PROGRESS;
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

    // This will probably timeout, if it waits for too long
    // If it doesn't timeout, we will have to run this in a separate thread
    // and wait for couple of seconds.
    status = this->ProcessDNSServiceRegisterResult(); // this blocks until it receives the reply from the daemon
    if (status == BVStatus::BVSTATUS_OK)
    {
        this->SetIsRegistered(true);
    }
    return status;
}

// Couldn't this method be abstracted? Or made an interface
// if few more functions will have the same body
// and be used in Service Discovery and Service Registration functionality
BVStatus BVService_Bonjour::ProcessDNSServiceRegisterResult()
{
    DNSServiceErrorType error = DNSServiceProcessResult(this->dnsRef);
    if (error != kDNSServiceErr_NoError) {
        std::cerr << "[DNSServiceBrowseResult] Encountered an error in DNSServiceProcessResult: " << error << std::endl;
        return BVStatus::BVSTATUS_NOK;
    }
    if (replyError) return BVStatus::BVSTATUS_NOK;
    else return BVStatus::BVSTATUS_OK;
}