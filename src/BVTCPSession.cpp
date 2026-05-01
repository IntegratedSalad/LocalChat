#include "BVTCPSession.hpp"
#include "BVTCPConnectionManager.hpp" // to call manager's functions

BVTCPSession::BVTCPSession(std::shared_ptr<BVTCPNodeConnectionSessionData> _sessionData_p,
                           boost::asio::io_context& _ioContext) :
sessionData_p(_sessionData_p),
ioContext(_ioContext)
{
    this->sessionData_p->alive = true;
    this->sessionData_p->readBuf = std::make_unique<char[]>(MESSAGE_FRAME_SIZE_BYTES);
    this->ClearReadBuffer();
    this->ClearWriteBuffer();
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

    if (msg->header.msgType != static_cast<uint8_t>(BVTCPMessageType::BVSESSIONCONTROLMESSAGETYPE_HELLO))
    {
        LogError("Session [{}]: OnReceiveHelloFrame called for wrong msgType={}", this->GetSessionData()->sessionID,
            static_cast<int>(msg->header.msgType));
        return;
    }

    LogTrace("Session [{}]: Received BVSESSIONCONTROLMESSAGETYPE_HELLO. Sending _HELLOBACK", 
        this->GetSessionData()->sessionID);

    // This is sending text data (from string) -> maybe put into function
    const std::string& serviceNameToCopy = this->sessionData_p->thisMachineServiceName;
    CharPayload128B payloadRaw;
    std::copy(serviceNameToCopy.begin(), serviceNameToCopy.end(), payloadRaw.data());
    BVTCPMessageHeader replyHeader = ConstructHeader(BVTCPMessageType::BVSESSIONCONTROLMESSAGETYPE_HELLOBACK);
    BVTCPMessage<CharPayload128B> replyMsg = ConstructMessage(replyHeader, payloadRaw);
    replyMsg.header.dataLen = serviceNameToCopy.length();
    WriteMessageFrame(replyMsg);
    this->sessionData_p->writeBuf.erase();
    StartReadingFrames();
}

void BVTCPSession::OnReceiveHelloBackFrame()
{
    BVTCPMessageHeader header = GetMsgHeader();

    if (header.msgType != static_cast<uint8_t>(
            BVTCPMessageType::BVSESSIONCONTROLMESSAGETYPE_HELLOBACK))
    {
        LogError("Session [{}]: OnReceiveHelloBackFrame called for wrong msgType={}",
                 this->GetSessionData()->sessionID,
                 static_cast<int>(header.msgType));
        return;
    }

    if (header.dataLen > 128)
    {
        LogError("Session [{}]: Invalid HELLOBACK payload length={}",
                 this->GetSessionData()->sessionID,
                 static_cast<unsigned>(header.dataLen));
        return;
    }

    const char* payloadPtr = GetPayloadPtr();
    if (!payloadPtr)
    {
        LogError("Session [{}]: Payload pointer is null.",
                 this->GetSessionData()->sessionID);
        return;
    }

    // This gets the payload - might be useful to put that into function
    std::string payloadStr(payloadPtr, static_cast<std::size_t>(header.dataLen));

    LogTrace("Session [{}]: Received _HELLOBACK with payload='{}'. Calling Manager handler.",
             this->GetSessionData()->sessionID,
             payloadStr);

    manager_p->HandleSessionIdentification(payloadStr, shared_from_this());
}

// TODO: This is probably not needed,
// as deregistration can be handled from the mDNS side.
void BVTCPSession::OnReceiveNodeGoodbyeFrame(void)
{
    BVTCPMessageHeader header = GetMsgHeader();
    const char* payloadPtr = GetPayloadPtr();
    // std::string payloadStr(payloadPtr, static_cast<std::size_t>(header.dataLen));
    std::string payloadStr("GUUUUUUUUWNO");

    LogTrace("Session [{}]: Received _NODESESSION_GOODBYE from {}", 
        this->GetSessionData()->sessionID, this->GetSessionData()->nodeData.serviceName);

    // Maybe we have the serviceName here in nodeData here?
    // assert(payloadStr == this->GetSessionData()->nodeData.serviceName); // ??? Yes!
    // this will be the endpoint's serviceName? Yes! TODO: We don't need to send serviceName!

    // manager_p->PutMessageIntoAppMailbox(BVEventType::BVEVENTTYPE_APP_SERVICE_DEREGISTERED, 
    //     std::make_unique<std::any>(std::make_any<std::string>(payloadStr)));
    manager_p->RemoveSession(this->sessionData_p->sessionID);
    // Put message in app mailbox so it can react
    // BVTCPSession remove it from the map
    // Close this session
}

bool BVTCPSession::OnReceiveStandardFrame(void)
{
    // Parse
    // copy to buffer 10 bytes and read message type
    // needed?

    LogTrace(
        "Session [{}]: Received a standard frame, parsing...",
            this->GetSessionData()->sessionID);
    
    BVTCPMessageHeader header = GetMsgHeader();
    switch (header.msgType)
    {
        case BVTCPMessageType::BVSESSIONCONTROLMESSAGETYPE_NODESESSION_GOODBYE:
        {
            // OnReceiveNodeGoodbyeFrame();
            // LogTrace("Returning early after closing the session - received _GOODBYE.");
            // return true;
        }
        default: 
        {
            LogWarn(
                "Session [{}]: Received a standard, unrecognized frame..",
                    this->GetSessionData()->sessionID);
        }
    }
    return false;
}