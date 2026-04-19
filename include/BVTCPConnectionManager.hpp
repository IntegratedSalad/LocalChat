#pragma once
#include <string>
#include <iostream>
#include <boost/asio.hpp>
#include "BV.hpp"
#include "BVLoggable.hpp"
#include "BVService.hpp"
#include <arpa/inet.h>
#include <map>
#include <mutex>

// These are messages sent between hosts
typedef enum class BVTCPMessageType
{
    BVTCPMESSAGETYPE_HANDSHAKE,
    BVTCPMESSAGETYPE_DEREGISTRATION, // ? 
    BVTCPMESSAGETYPE_TEXT_MESSAGE,
    BVTCPMESSAGETYPE_FILE,

} BVTCPMessageType;

// BVNode is data regarding another host in the network.
struct BVNode // BVNodeData?
{
    std::string serviceName;
    std::string hostname;
    int port;
    boost::asio::ip::tcp::endpoint ep;
    boost::asio::ip::address address;
    boost::asio::ip::basic_resolver_results<boost::asio::ip::tcp> results;

    BVNode(const std::string& _serviceName,
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
    BVNode(const std::string& _serviceName,
           const std::string& _hostname,
           int      _port,
           boost::asio::ip::address _address)
    {
        serviceName = _serviceName;
        hostname    = _hostname;
        port        = _port;
        address     = _address;
    }
    BVNode(const std::string& _serviceName,
           const std::string& _hostname,
           int      _port,
           boost::asio::ip::basic_resolver_results<boost::asio::ip::tcp> _results)
    {
        serviceName = _serviceName;
        hostname    = _hostname;
        port        = _port;
        results     = _results;
    }

    BVNode() = default;
};

struct BVUser
{
    std::string name;
    BVNode hostData;
};

// TODO: Move this to BVTCPSession.hpp
struct BVTCPNodeConnectionSessionData // needed?
{
    BVNode nodeData; // node data
    boost::asio::ip::tcp::socket sock;
    bool alive = false;

    // unique_ptr to thread?

    BVTCPNodeConnectionSessionData(BVNode _nodeData, boost::asio::io_context& _ioContext):
    nodeData(_nodeData),
    sock(_ioContext)
    {

    }
};

/*
 * BVTCPConnectionManager
 * As part of the BVTCP Suite.
 * This class manages all connections - these coming in (we accept them)
 * and those that we initiate (we connect to other).
 * 
 * 
*/

// Remember to open a different file to log the connections logs.
class BVTCPConnectionManager : public BVLoggable // BVComponent?
{
private:

    // This machine is this Node in the network.
    const BVServiceData thisMachineServiceData;
    BVNode              thisMachineHostData;
    boost::asio::io_context& ioContext;

    boost::asio::ip::tcp::acceptor acceptorSocket;

    // Map of active connections to other Nodes.
    // Session can be created in one thread,
    // But removed in the other.
    // Maybe key should be NodeID.
    std::map<std::string, std::shared_ptr<BVTCPNodeConnectionSessionData>> sessions_m;
    std::mutex session_m_mutex;

    // Outwards communication queue with App.
    // App just pushes to one of these, doesn't listen to them.
    // Incoming messages from sessions are put right into App's inMailbox_p
    // std::map<NodeID, threadsafe_queue>
    
public:
    BVTCPConnectionManager(boost::asio::io_context& _ioContext,
                           const BVServiceData _thisMachineServiceData);

    // Upon construction of objects other than for test purposes, instantiate acceptor socket and initialize connection.
    BVStatus StartAcceptingConnections(void);
    BVStatus InitiateSessionWithNode(const BVNode hostData); // initialize and save in map
    
    ~BVTCPConnectionManager();
};
