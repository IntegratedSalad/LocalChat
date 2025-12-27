#include "BVService_Avahi.hpp"

bool critical_failure = 0;
bool register_success = 0;
AvahiEntryGroup* group = NULL; // Make sure we free this when deregistering the service!
BVStatus BVService_Avahi::CreateAvahiClient(void)
{
    BVStatus status = BVStatus::BVSTATUS_OK;
    int error;

    const std::string userdata = this->GetHostname() + "," + this->GetRegType();

    this->client_p = std::unique_ptr<AvahiClient, AvahiClientDeleter>(
        avahi_client_new(avahi_simple_poll_get(this->simple_poll_p.get()), 
                        (AvahiClientFlags)0, 
                        client_callback, 
                        (void*)userdata.c_str(), 
                        &error));
    if (this->client_p == nullptr)
    {
        status = BVStatus::BVSTATUS_NOK; // optionally set up an error type
    }
    return status;
}

BVStatus BVService_Avahi::Setup(void)
{
    return this->CreateAvahiClient(); // remember, the pointer client_p is not for the use outside the callback
}

BVStatus BVService_Avahi::Register(void)
{
    BVStatus status = BVStatus::BVSTATUS_NOK;
    avahi_simple_poll_iterate(this->simple_poll_p.get(), 10 * 1000);
    if (register_success)
    {
        status = BVStatus::BVSTATUS_OK;
    } else if (critical_failure)
    {
        status = BVStatus::BVSTATUS_FATAL_ERROR;
    }
    return status;
}

// Deregister
// Deregistering MUST mean that no callbacks will be called!