#pragma once
#include <string>
#include <iostream>
#include <boost/asio.hpp>
#include "BV.hpp"
#include "BVLoggable.hpp"
#include "BVService.hpp"
#include <arpa/inet.h>

// These are messages sent between hosts
typedef enum class BVTCPMessageType
{
    BVTCPMESSAGETYPE_HANDSHAKE,
    BVTCPMESSAGETYPE_DEREGISTRATION, // ? 
    BVTCPMESSAGETYPE_TEXT_MESSAGE,
    BVTCPMESSAGETYPE_FILE,

} BVTCPMessageType;

struct BVHost // TODO: maybe rename that to BVNode, as a connection node
{
    std::string serviceName;
    std::string hostname;
    int port;
    boost::asio::ip::address address;
    boost::asio::ip::basic_resolver_results<boost::asio::ip::tcp> results;

    BVHost(const std::string& _serviceName,
           const std::string& _hostname,
           int      _port,
           boost::asio::ip::address _address,
           boost::asio::ip::basic_resolver_results<boost::asio::ip::tcp> _results)
    {
        serviceName = _serviceName;
        hostname    = _hostname;
        port        = _port;
        address     = _address;
        results     = _results;
    }
    BVHost(const std::string& _serviceName,
           const std::string& _hostname,
           int      _port,
           boost::asio::ip::address _address)
    {
        serviceName = _serviceName;
        hostname    = _hostname;
        port        = _port;
        address     = _address;
    }
    BVHost(const std::string& _serviceName,
           const std::string& _hostname,
           int      _port,
           boost::asio::ip::basic_resolver_results<boost::asio::ip::tcp> _results)
    {
        serviceName = _serviceName;
        hostname    = _hostname;
        port        = _port;
        results     = _results;
    }

    BVHost() = default;
};

struct BVTCPData
{

};

struct BVUser
{
    std::string name;
    BVHost hostData;
};

struct BVTCPConnectionData
{
    BVHost host;
    boost::asio::ip::tcp::socket sock_out;
    // Socket in?
    // Socket out?
    bool alive;
};

// Remember to open a different file to log the connections logs.
class BVTCPConnectionManager : public BVLoggable // BVComponent?
{
private:
    // maybe std::map with connections?
    // key -> BVHost.serviceName?

    const BVServiceData thisMachineServiceData;
    BVHost              thisMachineHostData;
    boost::asio::io_context& ioContext;

    boost::asio::ip::tcp::acceptor acceptorSocket;

public:
    BVTCPConnectionManager(boost::asio::io_context& _ioContext,
                           const BVServiceData _thisMachineServiceData); 

    // Upon construction of objects other than for test purposes, instantiate acceptor socket and initialize connection.
    BVStatus InitializeTCPConnectionForThisMachine(void);

    BVStatus CreateTCPConnection(const BVHost& hostData); // initialize and save in map
    ~BVTCPConnectionManager();
};
