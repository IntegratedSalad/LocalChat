#ifndef __AVAHI_BRIDGE_API
#define __AVAHI_BRIDGE_API

#include <avahi-common/simple-watch.h>
#include "linked_list.h"

#ifdef __cplusplus
extern "C" {
#endif
void on_service_found(void* obj);
AvahiSimplePoll* on_browser_failure(void* obj);
LinkedList_str* on_service_add(void* obj);
#ifdef __cplusplus
}
#endif

#endif /* __AVAHI_BRIDGE_API */
