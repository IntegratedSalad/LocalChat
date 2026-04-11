#pragma once
#include <string>
#include <iostream>
#include <boost/asio/ip/tcp.hpp>

typedef enum class BVStatus
{
    // General flags
    BVSTATUS_OK,
    BVSTATUS_NOK,
    BVSTATUS_IN_PROGRESS,
    BVSTATUS_FATAL_ERROR,

    BVSTATUS_MAX_COMPONENTS
} BVStatus;

struct BVHost
{
    std::string serviceDomainName;
    std::string hostname;
    boost::asio::ip::address address;
};

struct BVServiceBrowseInstance
{
    std::string serviceName;
    std::string regType;
    std::string replyDomain;

    BVServiceBrowseInstance(const BVServiceBrowseInstance& other) :
    serviceName(other.serviceName),
    regType(other.regType),
    replyDomain(other.replyDomain)
    {}

    BVServiceBrowseInstance() = default;

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
