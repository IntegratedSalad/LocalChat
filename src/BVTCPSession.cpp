#include "BVTCPSession.hpp"

BVTCPSession::BVTCPSession(std::shared_ptr<BVTCPNodeConnectionSessionData> _sessionData_p,
                           boost::asio::io_context& _ioContext) :
sessionData_p(_sessionData_p),
ioContext(_ioContext)
{
    this->sessionData_p->alive = true;
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

void BVTCPSession::Write(void)
{
    this->sessionData_p->sock.async_write_some(
        boost::asio::buffer(this->sessionData_p->buf),
        std::bind(&BVTCPSession::WriteCallback, this, std::placeholders::_1, std::placeholders::_2, this->sessionData_p)
    );
}

void BVTCPSession::RequestWrite(const std::string& data)
{
    this->sessionData_p->buf = data;
    this->Write();
}