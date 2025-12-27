#pragma once
#include <avahi-client/client.h>

struct AvahiClientDeleter
{
    void operator()(AvahiClient* p)
    {
        avahi_client_free(p);
    }
};
