#include "BVService_Avahi.hpp"

extern "C"
{
#include "avahi-client/client.h"
static void client_callback(AvahiClient* cl_p, AvahiClientState state, void* userdata)
{
    switch (state)
    {
        case AVAHI_CLIENT_S_RUNNING:
        {
            // create services...
            break;
        }
    }
}
}

BVStatus BVService_Avahi::CreateAvahiClient(void)
{
    BVStatus status = BVStatus::BVSTATUS_OK;
    this->client_p = std::make_unique<AvahiClient>(
        avahi_client_new(avahi_simple_poll_get(this->simple_poll_p.get(), 0)));
}

BVStatus BVService_Avahi::Setup(void)
{

}

BVStatus BVService_Avahi::Register(void)
{

}