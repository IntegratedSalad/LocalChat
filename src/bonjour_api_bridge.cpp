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

void CopyBrowseInstanceFields(void* bI_p, char serviceName[N_BYTES_SERVNAME_MAX], 
                                         char regType[N_BYTES_REGTYPE_MAX],
                                         char replyDomain[N_BYTES_REPLDOMN_MAX])
{
    BVServiceBrowseInstance* bI = static_cast<BVServiceBrowseInstance*>(bI_p);
    memcpy(serviceName, bI->serviceName.c_str(), N_BYTES_SERVNAME_MAX);
    memcpy(regType,     bI->regType.c_str(), N_BYTES_REGTYPE_MAX);
    memcpy(replyDomain, bI->replyDomain.c_str(), N_BYTES_REPLDOMN_MAX);
}

void SendServiceResolvedMessageToApp(DNSResolutionResult* res_p, void* obj)
{
    // static_cast<BVDiscovery_Bonjour*>(obj)->SendMessage(
    //     BVMessage(
    //         BVEventType::BVEVENTTYPE_APP_DISCOVERY_SERVICE_RESOLVED,
    //             std::make_unique<std::any>(
    //                 std::make_any<ResolvePayload*>(payload_p));
    //     )
    // );
    static_cast<BVDiscovery_Bonjour*>(obj)->SendMessage_API_INTERFACE(
        BVEventType::BVEVENTTYPE_APP_DISCOVERY_SERVICE_RESOLVED,
        std::make_any<DNSResolutionResult*>(res_p)
    );
}