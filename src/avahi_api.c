#include "avahi_api.h"

/*
 * Browse callback
 * void* userdata should contain a pointer to the discovery implementation
 * which has member functions defined called by functions corresponding to them.
 * 
*/
void browse_callback(
    AvahiServiceBrowser* sb,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiBrowserEvent event,
    const char* name,
    const char* type,
    const char* domain,
    AVAHI_GCC_UNUSED AvahiLookupResultFlags flags, 
    void* userdata) 
{
    // If this will be problematic, just pass intermediate structure
    // implements member function handlers

    switch (event)
    {
        case AVAHI_BROWSER_FAILURE:
        {
            fprintf(stderr, "(Browser) %s\n", 
                avahi_strerror(avahi_client_errno(avahi_service_browser_get_client(sb))));

                // avahi_simple_poll_quit(discovery_p->GetSimplePoll());
                avahi_simple_poll_quit((AvahiSimplePoll*)on_browser_failure(userdata));
            return;
        }
        case AVAHI_BROWSER_NEW:
        {
            // Add discovered services to the linked list
            setbuf(stdout, NULL);
            printf("Found %s.%s in %s!\n", name, type, domain);

            // TODO: put this in a separate function
            char buff[N_BYTES_SERVICE_STR_TOTAL];
            const size_t servLen = strlen(name);
            const size_t regLen = strlen(type);
            const size_t replDmnLen = strlen(domain);
            for (int i = 0; i < N_BYTES_SERVICE_STR_TOTAL; i++)
            {
                buff[i] = ' ';
            }
            if (servLen < N_BYTES_SERVNAME_MAX)
            {
                memcpy(buff, name, servLen);
            }
            if (regLen < N_BYTES_REGTYPE_MAX)
            {
                memcpy(buff + N_BYTES_SERVNAME_MAX, type, regLen);
            }
            if (replDmnLen < N_BYTES_REPLDOMN_MAX)
            {
                memcpy(buff + N_BYTES_SERVNAME_MAX + N_BYTES_REGTYPE_MAX, domain, replDmnLen);
            }
            buff[N_BYTES_SERVICE_STR_TOTAL-1] = '\0';
            LinkedListElement_str* lle_p = LinkedListElement_str_Constructor(buff, NULL);
            LinkedList_str_AddElement((LinkedList_str*)on_service_add(userdata), lle_p);

            on_service_found(userdata); // calls PushBrowsedServicesToQueue

            // We must call the member function that is responsible for enqueing the results
            // discovery_p->OnServiceFound(); // calls PushBrowsedServicesToQueue
            // or notify
            break;
        }
        case AVAHI_BROWSER_REMOVE:
        {
            fprintf(stdout, "Service was removed... TODO \n");
            break;
        }
        case AVAHI_BROWSER_ALL_FOR_NOW:
        case AVAHI_BROWSER_CACHE_EXHAUSTED:
        {
            fprintf(stdout, "Cache exhuast or all for now... TODO");
            break;
        }
    }
}
