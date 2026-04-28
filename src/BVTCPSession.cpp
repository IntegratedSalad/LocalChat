#include "BVTCPSession.hpp"

BVTCPSession::BVTCPSession(std::shared_ptr<BVTCPNodeConnectionSessionData> _sessionData_p,
                           boost::asio::io_context& _ioContext) :
sessionData_p(_sessionData_p),
ioContext(_ioContext)
{
    this->sessionData_p->alive = true;
    // async read
    // maybe read 138 bytes
    // this->StartReadingFrames();
}

void BVTCPSession::Start(void)
{

}

void BVTCPSession::Shutdown(void)
{

}

void BVTCPSession::Read(void)
{


}

void BVTCPSession::WriteSome(void)
{
    this->sessionData_p->sock->async_write_some(
        boost::asio::buffer(this->sessionData_p->writeBuf),
        std::bind(&BVTCPSession::WriteSomeCallback, this, std::placeholders::_1, std::placeholders::_2)
    );
}

void BVTCPSession::RequestSomeWrite(const std::string& data)
{
    this->sessionData_p->writeBuf = data;
    this->WriteSome();
}

void BVTCPSession::OnReceiveHelloFrame(void)
{
    using CharPayload128B = std::array<char, 128>;
    using HelloMsg = BVTCPMessage<CharPayload128B>;
    const auto* msg = reinterpret_cast<const HelloMsg*>(this->sessionData_p->readBuf.get());

    if (msg->header.msgType != static_cast<uint8_t>(BVTCPMessageType::BVSESSIONCONTROLLMESSAGETYPE_HELLO))
    {
        LogError("Session [{}]: OnReceiveHelloFrame called for wrong msgType={}", this->GetSessionData()->sessionID,
            static_cast<int>(msg->header.msgType));
        return;
    }

    LogTrace("Session [{}]: Received BVSESSIONCONTROLLMESSAGETYPE_HELLO. Sending _HELLOBACK", 
        this->GetSessionData()->sessionID);
    
    const std::string& serviceNameToCopy = this->sessionData_p->nodeData.serviceName;
    CharPayload128B payloadRaw;
    std::copy(serviceNameToCopy.begin(), serviceNameToCopy.end(), payloadRaw.data());
    BVTCPMessageHeader replyHeader = ConstructHeader(BVTCPMessageType::BVSESSIONCONTROLLMESSAGETYPE_HELLOBACK);
    BVTCPMessage<CharPayload128B> replyMsg = ConstructMessage(replyHeader, payloadRaw);
    WriteMessageFrame(replyMsg);
    this->sessionData_p->writeBuf.erase();
    StartReadingFrames();
}

void BVTCPSession::OnReceiveHelloBackFrame(void)
{
    using CharPayload128B = std::array<char, 128>;
    using HelloBackMsg = BVTCPMessage<CharPayload128B>;
    const auto* msg = reinterpret_cast<const HelloBackMsg*>(this->sessionData_p->readBuf.get());

    // call manager's function
    // TODO: ...

}

void BVTCPSession::OnReceiveStandardFrame(void)
{
    // Parse
    // copy to buffer 10 bytes and read message type
    // needed?
}