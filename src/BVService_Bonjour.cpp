#include "BVService_Bonjour.hpp"

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
        printf("--- Registered %s, as %s, in %s!\n", name, regtype, domain);
    } else {
        fprintf(stderr, "An error occurred while trying to register %s\n", name);
        replyError = true;
    }
}
}

BVStatus BVService_Bonjour::Register()
{
    BVStatus status = BVStatus::BV_STATUS_IN_PROGRESS;
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

    // This will probably timeout, if waits for too long
    // If it doesn't timeout, we will have to run this in a separate thread
    // and wait for couple of seconds.
    status = this->ProcessDNSServiceRegisterResult(); // this blocks until it receives the reply from the daemon
    if (status == BVStatus::BVSTATUS_OK)
    {
        this->SetIsRegistered(true);
    }
    return status;
}

BVStatus BVService_Bonjour::ProcessDNSServiceRegisterResult()
{
    DNSServiceErrorType error = DNSServiceProcessResult(this->dnsRef);
    if (error != kDNSServiceErr_NoError) {
        std::cerr << "Encountered an error in DNSServiceProcessResult: " << error << std::endl;
        return BVStatus::BVSTATUS_NOK;
    }
    if (replyError) return BVStatus::BVSTATUS_NOK;
    else return BVStatus::BVSTATUS_OK;
}