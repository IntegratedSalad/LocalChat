#pragma once
#include <string>
#include <iostream>

// BVActorMessage?
// messages sent between actors - someone disconnected, someone sent a message

typedef enum class BVStatus
{
    // General flags
    BVSTATUS_OK,
    BVSTATUS_NOK,
    BVSTATUS_IN_PROGRESS,
    BVSTATUS_FATAL_ERROR,

    BVSTATUS_MAX_COMPONENTS
} BVStatus;

struct BVServiceBrowseInstance
{
    std::string serviceName;
    std::string regType;
    std::string replyDomain;

    bool operator==(const BVServiceBrowseInstance& otherBI)
    {
        return (this->serviceName == otherBI.serviceName &&
                this->regType     == otherBI.regType     &&
                this->replyDomain == otherBI.replyDomain);
    }

    void print(void)
    {
        std::cout << "service name: " << serviceName << std::endl;
        std::cout << "reg type:     " << regType     << std::endl;
        std::cout << "domain:       " << replyDomain << std::endl;
    }
};
