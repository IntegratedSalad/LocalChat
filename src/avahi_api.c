#include "avahi_api.h"

void resolve_callback(
    AvahiServiceResolver *r,
    AVAHI_GCC_UNUSED AvahiIfIndex interface,
    AVAHI_GCC_UNUSED AvahiProtocol protocol,
    AvahiResolverEvent event,
    const char *name,
    const char *type,
    const char *domain,
    const char *host_name,
    const AvahiAddress *address,
    uint16_t port,
    AvahiStringList *txt,
    AvahiLookupResultFlags flags,
    void* userdata) 
{
    assert(r);
    assert(userdata);

    switch (event) 
    {
        case AVAHI_RESOLVER_FAILURE:
            fprintf(stderr, "(Resolver) Failed to resolve service '%s' of type '%s' in domain '%s': %s\n", name, type, domain, avahi_strerror(avahi_client_errno(avahi_service_resolver_get_client(r))));
            break;
        case AVAHI_RESOLVER_FOUND: {
            char a[AVAHI_ADDRESS_STR_MAX], *t;
            // fprintf(stderr, "Service '%s' of type '%s' in domain '%s':\n", name, type, domain);
            avahi_address_snprint(a, sizeof(a), address);
            t = avahi_string_list_to_string(txt);
            // fprintf(stderr,
            //         "\t%s:%u (%s)\n"
            //         "\tTXT=%s\n"
            //         "\tcookie is %u\n"
            //         "\tis_local: %i\n"
            //         "\tour_own: %i\n"
            //         "\twide_area: %i\n"
            //         "\tmulticast: %i\n"
            //         "\tcached: %i\n",
            //         host_name, port, a,
            //         t,
            //         avahi_string_list_get_service_cookie(txt),
            //         !!(flags & AVAHI_LOOKUP_RESULT_LOCAL),
            //         !!(flags & AVAHI_LOOKUP_RESULT_OUR_OWN),
            //         !!(flags & AVAHI_LOOKUP_RESULT_WIDE_AREA),
            //         !!(flags & AVAHI_LOOKUP_RESULT_MULTICAST),
            //         !!(flags & AVAHI_LOOKUP_RESULT_CACHED));

            /* Prepare data */
            DNSResolutionResult* resolution_result_p = (DNSResolutionResult*)malloc(sizeof(DNSResolutionResult));
            memcpy(resolution_result_p->serviceName, name, N_BYTES_SERVNAME_MAX);
            memcpy(resolution_result_p->regType, type, N_BYTES_REGTYPE_MAX);
            memcpy(resolution_result_p->replyDomain, domain, N_BYTES_REPLDOMN_MAX);
            memcpy(resolution_result_p->hosttarget, host_name, strlen(host_name));
            memcpy(resolution_result_p->fullname, a, strlen(a));
            memcpy(resolution_result_p->txtRecord, t, strlen(t));
            resolution_result_p->port = port;
            resolution_result_p->txtLen = strlen(t);
            on_service_resolved(userdata, resolution_result_p); // Sends Message
        }
    }
}

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
            // If not paused: add.

            // Add discovered services to the linked list
            setbuf(stdout, NULL);
            printf("Found %s.%s in %s!\n", name, type, domain);

            printf("IF: %d PROTO: %d\n", interface, protocol);

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

            on_service_found(userdata); // calls ReturnListFromBrowseResults

            // Service Resolution
            /*
             * In case of Avahi, resolution is done immediately after service was found
            */
            // Do not resolve this machine
            fprintf(stderr, "DEBUG\n");
            fprintf(stderr, on_this_machine_host_name_request(userdata));
            fprintf(stderr, "\n");
            fprintf(stderr, name);
            fprintf(stderr, "\n");
            if (strncmp(on_this_machine_host_name_request(userdata), name, strlen(name)) == 0)
            {
                break;
            }
            if (!(avahi_service_resolver_new(avahi_service_browser_get_client(sb), 
                                             interface, 
                                             protocol, 
                                             name, 
                                             type, 
                                             domain, 
                                             AVAHI_PROTO_UNSPEC, 
                                             0, 
                                             resolve_callback, 
                                             userdata)))
            {
                fprintf(stderr, "Failed to resolve service '%s': %s\n", name, avahi_strerror(avahi_client_errno(avahi_service_browser_get_client(sb))));
            }
            break;
        }
        case AVAHI_BROWSER_REMOVE:
        {
            setbuf(stdout, NULL);
            printf("Found %s.%s in %s!\n", name, type, domain);

            // This is just a copy of what's above.
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

            on_service_removed(userdata);

            fprintf(stdout, "Service was removed... TODO \n");
            break;
        }
        case AVAHI_BROWSER_ALL_FOR_NOW:
        case AVAHI_BROWSER_CACHE_EXHAUSTED:
        {
            fprintf(stdout, "Cache exhuast or all for now...");
            break;
        }
    }
}

// Registration callbacks

void entry_group_callback(AvahiEntryGroup* g, 
                          AvahiEntryGroupState state, 
                          AVAHI_GCC_UNUSED void* userdata)
{
    assert(g == group || group == NULL);
    group = g;

    switch (state)
    {
        case AVAHI_ENTRY_GROUP_ESTABLISHED:
        {
            fprintf(stdout, "Entry group estabilished.\n");
            break;
        }
        case AVAHI_ENTRY_GROUP_COLLISION:
        {
            fprintf(stderr, "Group collision...\n");
            break;
        }
        case AVAHI_ENTRY_GROUP_FAILURE:
        {
            fprintf(stderr, "Entry group failure: %s\n", avahi_strerror(
                avahi_client_errno(
                    avahi_entry_group_get_client(g))));
            critical_failure = 1;
            break;
        }
        default:
        {
            break;
        }
    }
}

void client_callback(AvahiClient* cl_p, AvahiClientState state, void* userdata)
{
    switch (state)
    {
        case AVAHI_CLIENT_S_RUNNING:
        {
            // create services...
            fprintf(stdout, "Creating a service with userdata %s\n", (const char* )userdata);
            create_services(cl_p, (const char*)userdata);
            break;
        }
        case AVAHI_CLIENT_FAILURE:
        {
            fprintf(stderr, "Client failure: %s\n", avahi_strerror(avahi_client_errno(cl_p)));
            break;
        }
        default:
        {
            break;
        }
    }
}

// TODO: Maybe it's not that great to pass only a hostname_regtype?
// We can pass an uninitialized pointer to AvahiEntryGroup here
// in a structure, from a unique_ptr in the BVService class...
void create_services(AvahiClient* cl_p, const char* hostname_regtype)
{
    assert(cl_p);
    int ret;
    if (!group)
    {
        group = avahi_entry_group_new(cl_p, entry_group_callback, NULL);
        assert(group);
    }
    if (avahi_entry_group_is_empty(group))
    {
        char* dup = strdup(hostname_regtype);
        char* hostname = strtok(dup, ",");
        const char* regtype = strtok(NULL, ",");

        fprintf(stdout, "Hostname: %s \n", hostname);
        fprintf(stdout, "Regtype: %s \n", regtype);

        ret = avahi_entry_group_add_service(group, 
                                            AVAHI_IF_UNSPEC, 
                                            AVAHI_PROTO_UNSPEC, 
                                            (AvahiPublishFlags)0, 
                                            hostname,
                                            regtype,
                                            "local",
                                            0,
                                            PORT,
                                            NULL);
        if (ret == AVAHI_SERVER_COLLISION)
        {
            // handle collision
            fprintf(stderr, "Collision!\n");
            return;
        }
        ret = avahi_entry_group_commit(group);
        if (ret < 0)
        {
            fprintf(stderr, "Couldn't register the service...\n");
        } else
        {
            register_success = 1;
        }
    }
    return;
}
