#pragma once
#include "BV.hpp"
#include <boost/asio.hpp>
#include "threadsafequeue.hpp"
#include "BVMessage.hpp"
#include "BVLoggable.hpp"
#include "BVTCPCommon.hpp"

/*
 * Each and every connection we want to handle separately.
 * This class should implement and define functionality
 * that handles the connection with one service, started by accepting that connection.
 * What class should be responsible when we are trying to initiate (connect) to another service?
 * This should handle both incoming and outgoing traffic
 * 
 * This class communicates with App to update its data regarding communication with other nodes
*/

// Sessions can call manager's callbacks
class BVTCPConnectionManager;
class BVTCPSession : public BVLoggable, public std::enable_shared_from_this<BVTCPSession>
{
private:
    boost::asio::io_context& ioContext;
    std::shared_ptr<BVTCPNodeConnectionSessionData> sessionData_p;
    BVSessionState state = BVSessionState::BVSESSIONSTATE_UNPREPARED;
    BVSessionOrigin origin;
    // raw, unowning pointer just for reference
    // session dies first, so it's ok.
    BVTCPConnectionManager* manager_p; 
    // std::thread worker_thread;

    void Read(void);
    void WriteSome(void); // data? or is it written in session Data
    void WriteSomeCallback(const boost::system::error_code& ec,
                           std::size_t bytes_transferred)
    {
        if (ec)
        {
            LogError("Session [{}]: Error while writing to a socket! {}, {}. Message: {}",  
                this->GetSessionData()->sessionID, ec.value(), ec.message(), sessionData_p->writeBuf);
            return;
        }
        this->sessionData_p->totalBytesWritten += bytes_transferred;

        if (this->sessionData_p->totalBytesWritten == this->sessionData_p->writeBuf.length())
        {
            // And of writing. Just return
            ClearWriteBuffer();
            return;
        }

        this->sessionData_p->sock->async_write_some(
            boost::asio::buffer(sessionData_p->writeBuf),
            std::bind(&BVTCPSession::WriteSomeCallback, shared_from_this(), std::placeholders::_1, std::placeholders::_2)
        );
    }

    void ReadSomeCallback(const boost::system::error_code& ec,
                          std::size_t bytes_transferred)
    {
        if (ec)
        {
            LogError("Session [{}]: Error in ReadSomeCallback callback: {}, {}",  
                this->GetSessionData()->sessionID, ec.value(), ec.message());
                return;
        }
        this->sessionData_p->totalBytesRead += bytes_transferred;
        if (this->sessionData_p->totalBytesRead == MAX_MESSAGE_SIZE_BYTES)
        {
            ClearReadBuffer();
            return;
        }
        this->sessionData_p->sock->async_read_some(
            boost::asio::buffer(this->sessionData_p->readBuf.get() + this->sessionData_p->totalBytesRead,
                MAX_MESSAGE_SIZE_BYTES - this->sessionData_p->totalBytesRead),
            std::bind(&BVTCPSession::ReadSomeCallback, this, std::placeholders::_1, std::placeholders::_2)
        );
    }

    void ReadMessageFrameCallback(const boost::system::error_code& ec,
                                  std::size_t bytes_transferred)
    {
        if (ec)
        {
            LogError("Session [{}]: Error in ReadMessageFrame callback: {}, {}",  
                this->GetSessionData()->sessionID, ec.value(), ec.message());
            
            LogDebug("Read buffer: {} Read buffer is a nullpointer: {} Bytes read: {} Bytes transferred: {} Address in nodeData: {} Endpoint address: {} State: {}", 
                this->sessionData_p->readBuf.get(), this->sessionData_p->readBuf == nullptr, this->sessionData_p->totalBytesRead, bytes_transferred, 
                    this->sessionData_p->nodeData.address.to_string(), this->sessionData_p->nodeData.ep.address().to_string(), static_cast<int>(this->state));
            return;
        }
        this->sessionData_p->totalBytesRead += bytes_transferred;
        LogTrace("Session [{}]: Read {} bytes", this->sessionData_p->sessionID, bytes_transferred);
        if (this->sessionData_p->totalBytesRead == MESSAGE_FRAME_SIZE_BYTES)
        {   
            // assert maybe?
            // Reset
            this->sessionData_p->totalBytesRead = 0;

            BVTCPMessageHeader header = GetMsgHeader();
            if (state == BVSessionState::BVSESSIONSTATE_UNPREPARED)
            {
                if (header.msgType == BVTCPMessageType::BVSESSIONCONTROLLMESSAGETYPE_HELLO)
                    OnReceiveHelloFrame();
                if (header.msgType == BVTCPMessageType::BVSESSIONCONTROLLMESSAGETYPE_HELLOBACK)
                    OnReceiveHelloBackFrame();
            } else
            {
                OnReceiveStandardFrame();
            }
            LogTrace("Session [{}]: Read all bytes {}", this->sessionData_p->sessionID, bytes_transferred);
            ClearReadBuffer();
            return;
        }
        boost::asio::async_read(*this->sessionData_p->sock, 
            boost::asio::buffer(this->sessionData_p->readBuf.get() + this->sessionData_p->totalBytesRead,
                MESSAGE_FRAME_SIZE_BYTES - this->sessionData_p->totalBytesRead), 
            std::bind(&BVTCPSession::ReadMessageFrameCallback, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
    }

    void StartReadingFrames(void)
    {
        boost::asio::async_read(*this->sessionData_p->sock, 
            boost::asio::buffer(this->sessionData_p->readBuf.get() + this->sessionData_p->totalBytesRead,
                MESSAGE_FRAME_SIZE_BYTES - this->sessionData_p->totalBytesRead), 
                  std::bind(&BVTCPSession::ReadMessageFrameCallback, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
    }

    void WriteMessageFrameCallback(const boost::system::error_code& ec,
                                   std::size_t bytes_transferred)
    {
        if (ec)
        {
            LogError("Session [{}]: Error while writing frame to a socket! {}, {}. Message: {}",  
                this->GetSessionData()->sessionID, ec.value(), ec.message(), sessionData_p->writeBuf);
                return;
            return;
        }
        this->sessionData_p->totalBytesWritten += bytes_transferred;
        LogDebug("Session [{}]: Writebuffer: {}", this->sessionData_p->sessionID, this->sessionData_p->writeBuf);
        LogTrace("Session [{}]: Written {} bytes", this->sessionData_p->sessionID, bytes_transferred);
        if (this->sessionData_p->totalBytesWritten == this->sessionData_p->writeBuf.length())
        {
            LogTrace("Session [{}]: Written all bytes", this->sessionData_p->sessionID, bytes_transferred);
            ClearWriteBuffer();
            return;
        }
        boost::asio::async_write(*this->sessionData_p->sock,
            boost::asio::buffer(sessionData_p->writeBuf),
                std::bind(&BVTCPSession::WriteMessageFrameCallback, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
    }

    BVTCPMessageHeader GetMsgHeader(void)
    {
        BVTCPMessageHeader header{};
        const char* buf = this->sessionData_p->readBuf.get();
        header.dataLen = static_cast<uint8_t>(buf[0]);
        std::memcpy(&header.timestamp, buf+1, sizeof(header.timestamp));
        header.msgType = static_cast<uint8_t>(buf[9]);
        return header;
    }

public:
    BVTCPSession(std::shared_ptr<BVTCPNodeConnectionSessionData> _sessionData_p,
                 boost::asio::io_context& _ioContext);

    void Start(void);
    void Shutdown(void);

    BVTCPNodeConnectionSessionData* GetSessionData(void)
    {
        return this->sessionData_p.get();
    }

    void SetOrigin(const BVSessionOrigin& _origin)
    {
        this->origin = _origin;
    }

    BVSessionOrigin GetOrigin(void)
    {
        return this->origin;
    }

    void SetState(const BVSessionState& _state)
    {
        this->state = _state;
    }

    BVSessionState GetState(void)
    {
        return this->state;
    }

    // for now only text
    void RequestSomeWrite(const std::string& data);
    void OnReceiveHelloFrame(void);
    void OnReceiveHelloBackFrame(void);
    void OnReceiveStandardFrame(void);
    // Upon receiving chat message,
    // call BVTCPConnectionManager function
    // or, construct BVMessage and directly put it in appCommChannel_p.

    void RequestReadingFrames(void)
    {
        StartReadingFrames();
    }

    template<typename PayloadType>
    void WriteMessageFrame(const BVTCPMessage<PayloadType>& message)
    {
        static_assert(std::is_trivially_copyable_v<BVTCPMessage<PayloadType>>,
                    "BVTCPMessage must be trivially copyable to send as raw bytes.");

        this->sessionData_p->totalBytesWritten = 0;
        
        constexpr std::size_t headerSize = 10;
        constexpr std::size_t payloadSize = MESSAGE_FRAME_SIZE_BYTES - headerSize;

        if (sizeof(PayloadType) > payloadSize)
        {
            LogError("Session [{}]: WriteMessageFrame: payload too large. payloadSize={}, max={}",
                this->sessionData_p->sessionID, sizeof(PayloadType), payloadSize);
            return;
        }

        LogDebug("WriteMessageFrame: data size: {}", sizeof(message.payload));
        LogDebug("WriteMessageFrame: dataLen: {}", message.header.dataLen);
        char* buf = this->sessionData_p->writeBuf.data();
        buf[0] = static_cast<char>(message.header.dataLen);
        std::memcpy(buf + 1,
            &message.header.timestamp,
            sizeof(message.header.timestamp));
        buf[9] = static_cast<char>(message.header.msgType);
        std::memcpy(buf + headerSize,
            &message.payload,
            sizeof(PayloadType));

        assert(this->sessionData_p->writeBuf.size() != 0);

        auto self = shared_from_this();
        boost::asio::async_write(
            *this->sessionData_p->sock,
            boost::asio::buffer(self->sessionData_p->writeBuf.data(),
                                self->sessionData_p->writeBuf.size()),
            [self](const boost::system::error_code& ec, std::size_t bytesTransferred)
            {
                self->WriteMessageFrameCallback(ec, bytesTransferred);
            });
    }

    void SetManager_p(BVTCPConnectionManager* p)
    {
        this->manager_p = p;
    }

    const char* GetPayloadPtr(void) const
    {
        if (!this->sessionData_p->readBuf)
        {
            return nullptr;
        }

        return this->sessionData_p->readBuf.get() + HEADER_SIZE_BYTES;
    }

    void ClearReadBuffer(void)
    {
        this->sessionData_p->readBuf.reset();
        this->sessionData_p->totalBytesRead = 0;
    }

    void ClearWriteBuffer(void)
    {
        this->sessionData_p->writeBuf.clear();
        this->sessionData_p->totalBytesWritten = 0;
        this->sessionData_p->writeBuf.assign(MESSAGE_FRAME_SIZE_BYTES, '\0');
    }

    void Close(void)
    {
        this->state = BVSessionState::BVSESSIONSTATE_CLOSED;
        this->sessionData_p->sock->cancel();
        this->sessionData_p->readBuf.reset();
        this->sessionData_p->writeBuf.erase();
    }

    ~BVTCPSession() {};
};
