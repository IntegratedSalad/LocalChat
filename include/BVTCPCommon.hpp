#pragma once
#include <boost/asio.hpp>
#include "threadsafequeue.hpp"
#include "BVMessage.hpp"
#include <memory>
#include "const.h"

#define MAX_MESSAGE_SIZE_BYTES 128

using NodeID = uint8_t;
using SessionID = uint16_t;

typedef enum class BVSessionControlMessage
{
    BVSESSIONCONTROLLMESSAGE_HELLO

} BVSessionControlMessage;

// BVNode is data regarding another host in the network.
struct BVNode // BVNodeData?
{
    NodeID id;
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

struct ChatMessageMetadata
{
    uint64_t      timestamp;
    std::string   sender;
    NodeID        recipient;
};

struct ChatMessage // move to separate?
{
    ChatMessageMetadata metadata;
    std::string         textData;
    // ...
};

struct BVUser
{
    std::string name;
    BVNode hostData;
};

struct BVTCPNodeConnectionSessionData
{
    SessionID sessionID;
    BVNode nodeData;
    // Communication channel with app; outMailbox_p
    std::shared_ptr<threadsafe_queue<BVMessage>> appCommChannel_p;
    std::shared_ptr<threadsafe_queue<BVMessage>> inMailbox_p;

    // flag telling us who initiated it?

    std::shared_ptr<boost::asio::ip::tcp::socket> sock; // shared pointer?
    bool alive = false;

    std::string writeBuf;
    std::unique_ptr<char[]> readBuf;
    std::size_t totalBytesWritten;
    std::size_t totalBytesRead;

    // unique_ptr to thread?

    BVTCPNodeConnectionSessionData(BVNode _nodeData, boost::asio::io_context& _ioContext, SessionID _sid):
    nodeData(_nodeData),
    sock(std::make_shared<boost::asio::ip::tcp::socket>(_ioContext)),
    sessionID(_sid)
    {

    }
};
