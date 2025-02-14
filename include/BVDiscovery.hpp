#pragma once
#include <string>

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