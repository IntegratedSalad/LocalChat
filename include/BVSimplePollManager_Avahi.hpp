#pragma once
#include <iostream>
#include <memory>
#include "BVService.hpp"
#include <avahi-common/simple-watch.h>

// TODO Later
// Redundant?

/* 
 * Class managing access to the SimplePoll.
 * Simple poll should be accessed only by one thread at a time.
 *
*/
class BVSimplePollManager_Avahi
{
private:
    std::unique_ptr<AvahiSimplePoll, AvahiSimplePollDeleter> simple_poll_p;

public:

    BVSimplePollManager_Avahi();
    ~BVSimplePollManager_Avahi();
};

struct AvahiSimplePollDeleter
{
    void operator()(AvahiSimplePoll* p)
    {
        avahi_simple_poll_free(p);
    }
};