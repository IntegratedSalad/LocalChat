#include "BVTCPSession.hpp"

BVTCPSession::BVTCPSession(std::unique_ptr<BVTCPNodeConnectionSessionData> _sessionData_p,
                           boost::asio::io_context& _ioContext) :
sessionData_p(std::move(_sessionData_p)),
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

void BVTCPSession::Write(const BVMessage& message)
{

}
