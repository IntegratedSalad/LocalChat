#include "avahi_api_bridge.h"
#include "BVDiscovery_Avahi.hpp"
#include "BVEvent.hpp"
#include "BV.hpp"
#include <any>

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

void on_service_resolved(void* obj, DNSResolutionResult* res_p)
{
    // Send Message
    static_cast<BVDiscovery_Avahi*>(obj)->SendMessage_API_INTERFACE(
        BVEventType::BVEVENTTYPE_APP_DISCOVERY_SERVICE_RESOLVED,
        std::make_any<DNSResolutionResult*>(res_p)
    );
}