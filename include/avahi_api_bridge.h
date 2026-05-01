#ifndef __AVAHI_BRIDGE_API
#define __AVAHI_BRIDGE_API

#include <avahi-common/simple-watch.h>
#include "api_common.h"
#include "linked_list.h"

#ifdef __cplusplus
extern "C" {
#endif
void on_service_found(void* obj);
void on_service_removed(void* obj);
const char* on_this_machine_host_name_request(void* obj);
AvahiSimplePoll* on_browser_failure(void* obj);
LinkedList_str* on_service_add(void* obj);
void on_service_resolved(void* obj, DNSResolutionResult* res_p);
#ifdef __cplusplus
}
#endif

#endif /* __AVAHI_BRIDGE_API */
