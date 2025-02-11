#pragma once
#include <iostream>
#include <boost/asio.hpp>

#define SERVICETYPE_STR "_localchathost._tcp"
#define PORT            50001

typedef enum class BVStatus
{
    BVSTATUS_OK,
    BVSTATUS_NOK,
    BV_STATUS_IN_PROGRESS

} BVStatus;

class BVService
{
private:
    const std::string regtype = SERVICETYPE_STR;
    std::string hostname;
    std::string domain;
    int port;
    bool isRegistered = false;

    virtual BVStatus Register(boost::asio::io_context& ioContext) = 0;
public:
    BVService(std::string _hostname, std::string _domain, int _port)
    {
        this->hostname = boost::asio::ip::host_name();
        // TODO foolproof that.
        this->domain = _domain;
        uint16_t portH = static_cast<uint16_t>(_port);
        this->port = static_cast<int>(htons(portH));
    }

    std::string GetHostname(void)
    {
        return this->hostname;
    }

    std::string GetDomain(void)
    {
        return this->domain;
    }

    int GetPort(void)
    {
        return this->port;
    }

    /* Gets service type */
    std::string GetRegType(void)
    {
        return this->regtype;
    }

    bool GetRegistered(void)
    {
        return this->isRegistered;
    }

    ~BVService()
    {
        
    }
};

/*
We have to consider the fact that this class and its functions will be 
executed within a GUI context so the response from Register() reply HAS to be
handled asynchronously.

And on top of that, we have to take into consideration the fact that mDNSResponder is a Windows and macOS
solution only - there will be avahi on Linux.

*/