#include "BVTCPConnection.hpp"

BVTCPConnectionManager::BVTCPConnectionManager(boost::asio::io_context& _ioContext,
                                               const BVServiceData _thisMachineServiceData):
ioContext(_ioContext),
thisMachineServiceData(_thisMachineServiceData),
acceptorSocket(_ioContext)
{
    // boost::asio::

    // TODO: set host data - service name and hostname
    // thisMachineHostData.
    // downcast is ok.
    boost::asio::ip::tcp::resolver resolver{ioContext};
    boost::system::error_code ec;
    auto results = resolver.resolve(thisMachineServiceData.hostname, std::to_string(thisMachineServiceData.port), ec);
    thisMachineHostData = BVHost{ thisMachineServiceData.hostname, 
                                  thisMachineServiceData.hostname,
                                  ntohs(thisMachineServiceData.port),
                                  results };

    if (ec)
    {
        LogError("BVTCPConnectionManager: Couldn't resolve {} {}", thisMachineServiceData.hostname.c_str(), ec.value());
        throw std::runtime_error("Couldn't resolve this machine service.");
    }

    // start listening
    // on what port?
    // on port 50001 (we announce our service on this port). And this is the port of the actual service,
    // not part of mDNS.
    // which IP I should get? It doesn't matter if it's IPv4 or IPv6. It's just how it is described
}

BVStatus BVTCPConnectionManager::CreateTCPConnection(const BVHost& hostData)
{
    return BVStatus::BVSTATUS_OK;
}

BVStatus BVTCPConnectionManager::InitializeTCPConnectionForThisMachine(void)
{
    // We have to have the same local IP address that will be resolved.
    // synchronousously initialize an acceptor socket

    // get endpoint
    boost::asio::ip::tcp::endpoint ep = thisMachineHostData.results.begin()->endpoint();
    this->acceptorSocket = boost::asio::ip::tcp::acceptor{ioContext, ep.protocol()};

    boost::system::error_code ec;
    this->acceptorSocket.bind(ep, ec);

    if (ec)
    {
        LogError("BVTCPConnectionManager: Couldn't bind the acceptor socket. {}", ec.value());
        throw std::runtime_error("Couldn't bind the acceptor socket.");
    }

    try {


    } catch (boost::asio::system::system_error& e)
    {
        // LogError("")
    }
    // N_SERVICES_MAX

    return BVStatus::BVSTATUS_OK;
}

BVTCPConnectionManager::~BVTCPConnectionManager()
{

}