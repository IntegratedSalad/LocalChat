#pragma once
#include <string>
#include <chrono>
#include <boost/asio.hpp>

#define DISCOVERY_TIMER_TRIGGER_S 5

class BVDiscovery
{
public:
    virtual void run() = 0;
    void operator()(void)
    {
        this->run();
    }
};