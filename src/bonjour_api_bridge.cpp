#include "bonjour_api_bridge.h"
#include "BVDiscovery_Bonjour.hpp"
#include "BVMessage.hpp"
#include "BVEvent.hpp"
#include "BV.hpp"
#include <any>

ResolveCallbackContext* GetResolveCallbackContext(void* ctx)
{
    return static_cast<ResolveCallbackContext*>(ctx);
}

void SendServiceResolvedMessageToApp(DNSResolutionResult* res_p, void* obj)
{
    static_cast<BVDiscovery_Bonjour*>(obj)->SendMessage_API_INTERFACE(
        BVEventType::BVEVENTTYPE_APP_DISCOVERY_SERVICE_RESOLVED,
        std::make_any<DNSResolutionResult*>(res_p)
    );
}