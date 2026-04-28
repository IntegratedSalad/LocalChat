#pragma once
#include <boost/asio.hpp>
#include "threadsafequeue.hpp"
#include "BVMessage.hpp"
#include <memory>
#include "const.h"

#define MAX_MESSAGE_SIZE_BYTES   256
#define MESSAGE_FRAME_SIZE_BYTES 138
#define HEADER_SIZE_BYTES        10
#define PAYLOAD_SIZE_BYTES       128
static_assert((PAYLOAD_SIZE_BYTES) + (HEADER_SIZE_BYTES) == (MESSAGE_FRAME_SIZE_BYTES));

using NodeID = uint8_t;
using SessionID = uint16_t;

/*
    BVTCPMessage structure
    | HEADER | PAYLOAD
        10      128

    HEADER:     10 bytes
     DataLen:   1  byte 
     MsgType:   1  byte
     timestamp: 8  bytes

    TOTAL: 138 BYTES

    Are MSG START and MSG END byte control characters needed?
*/


enum class BVSessionState
{
    BVSESSIONSTATE_UNPREPARED,
    BVSESSIONSTATE_ESTABLISHED,
    BVSESSIONSTATE_CLOSED
};

enum class BVSessionOrigin
{
    BVSESSIONORIGIN_INGOING,
    BVSESSIONORIGIN_OUTGOING
};

// We can send longer messages:
// e.g. _TYPE_AGGREGATE | _TYPE_CHATMESSAGE

namespace BVTCPMessageType
{
    const uint8_t BVSESSIONCONTROLLMESSAGETYPE_HELLO     = 0; // handshake
    const uint8_t BVSESSIONCONTROLLMESSAGETYPE_HELLOBACK = 1; // handshake reply
}


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

struct BVTCPMessageHeader
{
    uint8_t  dataLen;
    uint64_t timestamp;
    uint8_t  msgType;
};

template<typename PayloadType>
struct BVTCPMessage
{
    BVTCPMessageHeader header;
    PayloadType        payload;  
};

struct BVChatMessage // payload of a certain type
{
    std::string textData;
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

inline BVTCPMessageHeader ConstructHeader(const uint8_t msgType)
{
    BVTCPMessageHeader header;
    std::chrono::milliseconds ts = 
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch());
    header.timestamp = ts.count();
    header.msgType = msgType;
    return header;
}

template<typename PayloadType>
inline BVTCPMessage<PayloadType> ConstructMessage(BVTCPMessageHeader header, PayloadType payload)
{
    BVTCPMessage<PayloadType> msg;
    header.dataLen = sizeof(msg.payload);
    msg.header = header;
    msg.payload = payload;
    return msg;
}
