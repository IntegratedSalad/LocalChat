#pragma once
#include <iostream>
#include <boost/asio.hpp>
#include "BV.hpp"

#define SERVICETYPE_STR "_localchathost._tcp"
#define PORT            50001

class BVService
{
private:
    const std::string regtype = SERVICETYPE_STR;
    std::string hostname;
    std::string domain;
    int port;
    bool isRegistered = false;
public:
    BVService(std::string _hostname, std::string _domain, int _port)
    {
        this->hostname = _hostname;
        // TODO foolproof that.
        this->domain = _domain;
        uint16_t portH = static_cast<uint16_t>(_port);
        this->port = static_cast<int>(htons(portH));
    }

    virtual ~BVService() = default;

    virtual BVStatus Register(void) = 0;

    std::string GetHostname(void) const
    {
        return this->hostname;
    }

    std::string GetDomain(void) const
    {
        return this->domain;
    }

    int GetPort(void) const
    {
        return this->port;
    }

    /* Gets service type */
    std::string GetRegType(void) const
    {
        return this->regtype;
    }

    bool GetIsRegistered(void) const
    {
        return this->isRegistered;
    }

    void SetIsRegistered(const bool _isRegistered)
    {
        this->isRegistered = _isRegistered;
    }
};

/*
We have to consider the fact that this class and its functions will be 
executed within a GUI context so the response from Register() reply HAS to be
handled asynchronously.

And on top of that, we have to take into consideration the fact that mDNSResponder is a Windows and macOS
solution only - there will be avahi on Linux.

*/