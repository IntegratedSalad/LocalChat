#pragma once
#include <string>
#include <chrono>
#include <boost/asio.hpp>

#define DISCOVERY_TIMER_TRIGGER_S 5
#define N_BYTES_SERVNAME_MAX      24
#define N_BYTES_REGTYPE_MAX       24
#define N_BYTES_REPLDOMN_MAX      16
#define N_BYTES_SERVICE_STR_TOTAL (N_BYTES_SERVNAME_MAX + N_BYTES_REGTYPE_MAX + N_BYTES_REPLDOMN_MAX)
#define N_SERVICES_MAX            32

// TODO: Rewrite this base class
// We do not need it to be a functional object (no operator() overload)
class BVDiscovery
{
public:
    // Function that is called upon running the discovery functionality
    virtual void run() = 0;
    void operator()(void)
    {
        this->run();
    }
};