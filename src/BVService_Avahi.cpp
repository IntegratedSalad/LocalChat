#include "BVService_Avahi.hpp"

#ifndef EXT_FLAGS_C_
#define EXT_FLAGS_C_
bool critical_failure = 0;
bool register_success = 0;
#endif // EXT_FLAGS_C_

extern "C" // TODO: Put these in separate h/c files.
{
#include "avahi-client/client.h"
#include "avahi-common/error.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

static void create_services(AvahiClient* cl_p, const char* hostname_regtype);
// client callback prototype?
// entry_group_callback prototype?
static void entry_group_callback(AvahiEntryGroup* g, AvahiEntryGroupState state, AVAHI_GCC_UNUSED void* userdata)
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

static void client_callback(AvahiClient* cl_p, AvahiClientState state, void* userdata)
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

static void create_services(AvahiClient* cl_p, const char* hostname_regtype)
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
}

BVStatus BVService_Avahi::CreateAvahiClient(void)
{
    BVStatus status = BVStatus::BVSTATUS_OK;
    int error;

    const std::string userdata = this->GetHostname() + "," + this->GetRegType();

    this->client_p = std::unique_ptr<AvahiClient, AvahiClientDeleter>(
        avahi_client_new(avahi_simple_poll_get(this->simple_poll_p.get()), 
                        (AvahiClientFlags)0, 
                        client_callback, 
                        (void*)userdata.c_str(), 
                        &error));
    if (this->client_p == nullptr)
    {
        status = BVStatus::BVSTATUS_NOK; // optionally set up an error type
    }
    return status;
}

BVStatus BVService_Avahi::Setup(void)
{
    return this->CreateAvahiClient(); // remember, the pointer client_p is not for the use outside the callback
}

BVStatus BVService_Avahi::Register(void)
{
    BVStatus status = BVStatus::BVSTATUS_NOK;
    avahi_simple_poll_iterate(this->simple_poll_p.get(), 10 * 1000);
    if (register_success)
    {
        status = BVStatus::BVSTATUS_OK;
    } else if (critical_failure)
    {
        status = BVStatus::BVSTATUS_FATAL_ERROR;
    }
    return status;
}