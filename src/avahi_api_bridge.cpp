#include "avahi_api_bridge.h"
#include "BVDiscovery_Avahi.hpp"

void on_service_found(void* obj)
{
    static_cast<BVDiscovery_Avahi*>(obj)->OnServiceFound();
}

AvahiSimplePoll* on_browser_failure(void* obj)
{
    return static_cast<BVDiscovery_Avahi*>(obj)->GetSimplePoll();
}

LinkedList_str* on_service_add(void* obj)
{
    return static_cast<BVDiscovery_Avahi*>(obj)->GetLinkedList_p();
}
