#pragma once
#include <boost/asio.hpp>
#include "threadsafequeue.hpp"
#include "BVMessage.hpp"
#include "BVEvent.hpp"
#include <memory>
#include "const.h"

#define MAX_MESSAGE_SIZE_BYTES   256
#define MESSAGE_FRAME_SIZE_BYTES 138
#define HEADER_SIZE_BYTES        10
#define PAYLOAD_SIZE_BYTES       128
static_assert((PAYLOAD_SIZE_BYTES) + (HEADER_SIZE_BYTES) == (MESSAGE_FRAME_SIZE_BYTES));

#define TEN_LAST_MESSAGES        10

using NodeID = uint8_t;
using SessionID = uint16_t;
using CharBufferArray128B = std::array<char, MAX_TEXT_DATA_MSG_BYTES>;

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
    /* CONTROL MESSAGES */
    const uint8_t BVSESSIONCONTROLMESSAGETYPE_HELLO               = 0; // handshake
    const uint8_t BVSESSIONCONTROLMESSAGETYPE_HELLOBACK           = 1; // handshake reply
    // const uint8_t BVSESSIONCONTROLMESSAGETYPE_NODESESSION_GOODBYE = 3; // service is deregistering <- T
    const uint8_t BVSESSIONCONTROLMESSAGETYPE_CONFIRM_ESTABLISHED = 3;

    /* REGULAR MESSAGES */
    const uint8_t BVSESSIONREGULARMESSAGETYPE_CHATMESSAGE         = 4;
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

struct BVChatMessagePayload // payload of a certain type sent over the network.
{
    CharBufferArray128B textData;
};

// Maybe templated, in case of exchanging files.
// This is a structure that is used to represent message sent/received
struct BVChatMessage
{
    std::string textData;
    std::string sender;
    uint64_t timestamp;
    BVChatMessage() = default;
    BVChatMessage(const std::string& _textData, uint64_t _timestamp, const std::string& _sender):
    textData(_textData), timestamp(_timestamp), sender(_sender)
    {}
};

struct BVChatMessageLog
{
    std::string serviceName;
    std::vector<BVChatMessage> logV;
    BVChatMessageLog() = default;
    BVChatMessageLog(const std::string& _serviceName, const BVChatMessage msg) :
    serviceName(_serviceName)
    {
        AddMessage(msg);
    }
    
    void AddMessage(const BVChatMessage msg)
    {
        logV.push_back(msg);
    }

    std::vector<BVChatMessage> ReturnNLastMessages(const std::size_t n)
    {
       const std::size_t count = std::min(n, logV.size());
       return std::vector<BVChatMessage>(logV.end() - count, logV.end());
    }

    void PrintNLastMessages(const std::size_t n)
    {
        std::vector<BVChatMessage> messages = ReturnNLastMessages(n);
        std::cout << "+------+" << std::endl;
        for (const auto& msg : messages)
        {
            time_t timestamp = static_cast<time_t>(msg.timestamp); // downcast!
            time(&timestamp);
            const std::string datetime(ctime(&timestamp));
            std::cout << FormatDate(datetime);
            std::cout << " [" << msg.sender << "]: " << msg.textData << std::endl;
        }
        std::cout << "+------+" << std::endl;
    }

    void Clear(void)
    {
        logV.clear();
    }

    std::string FormatDate(const std::string& dateStr)
    {
        return dateStr.substr(4, dateStr.length() - 8 - 5);
    }
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

    const std::string thisMachineServiceName;
    // unique_ptr to thread?

    BVTCPNodeConnectionSessionData(BVNode _nodeData, boost::asio::io_context& _ioContext, SessionID _sid,
        const std::string& _thisMachineServiceName):
    nodeData(_nodeData),
    sock(std::make_shared<boost::asio::ip::tcp::socket>(_ioContext)),
    sessionID(_sid),
    thisMachineServiceName(_thisMachineServiceName)
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
    header.dataLen = sizeof(msg.payload); // wrong for string
    msg.header = header;
    msg.payload = payload;
    return msg;
}
