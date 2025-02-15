#pragma once
#include <string>
#include <chrono>
#include <boost/asio.hpp>

#define DISCOVERY_TIMER_TRIGGER_S 5

struct BVServiceBrowseInstance
{
    std::string serviceName;
    std::string regType;
    std::string replyDomain;
};

class BVDiscovery
{
public:
    virtual void run() = 0;
    void operator()(void)
    {
        this->run();
    }
};