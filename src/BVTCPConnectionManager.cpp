#include "BVTCPConnectionManager.hpp"

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
    thisMachineHostData = BVNode{ thisMachineServiceData.hostname, 
                                  thisMachineServiceData.hostname,
                                  ntohs(thisMachineServiceData.port),
                                  results };
    
    if (ec)
    {
        LogError("BVTCPConnectionManager: Couldn't resolve {} {}", thisMachineServiceData.hostname.c_str(), ec.value());
#ifdef RESOLUTION_POLICY_HARD_FAIL
        throw std::runtime_error("Couldn't resolve this machine service.");
#endif
    } else
    {

    }

    // start listening
    // on what port?
    // on port 50001 (we announce our service on this port). And this is the port of the actual service,
    // not part of mDNS.
}

// Initiate a Client connection with a Node
BVStatus BVTCPConnectionManager::InitiateSessionWithNode(const BVNode hostData)
{
    std::shared_ptr<BVTCPNodeConnectionSessionData> session_p =
         std::make_shared<BVTCPNodeConnectionSessionData>(hostData, ioContext);

    session_p->sock.open(session_p->nodeData.ep.protocol());

    {
        std::lock_guard<std::mutex> l(session_m_mutex);
        sessions_m[hostData.serviceName] = session_p;
    }

    // TODO:  How will this session communicate with ConnectionManager and how will it communicate with App?
    // via the same broker?
    // Intantiate a new broker in app?
    // another, internal broker instantiated in BVTCPConnectionManager?
    // The connection instance here will have to have access to what App is producing

    // TODO: Pass inMailbox_p
    // Just share an inMailBox_p with every Session!
    // These messages will probably get shared with broker, but it doesn't matter.

    return BVStatus::BVSTATUS_OK;
}

BVStatus BVTCPConnectionManager::StartAcceptingConnections(void)
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


    } catch (boost::system::system_error& e)
    {
        // LogError("")
    }

    // listen
    // N_SERVICES_MAX

    /*
        Asynchronously accept other connections.
        At this point data exchange will be between BVTCPConnectionHandlers
        TODO: Rename the file to BVTCPConnectionManager
    */

    // TODO: Try to synchronously wait for a connection, send a simple message and synchronously
    //       Receive that message!
    /*
        What is spawned here is a reactor for incoming TCP events triggered by messages sent by other nodes.
        What is spawned in BVApp_ConsoleClient::ResolveServiceToEndpoint is a communication endpoint
        for events that Application triggers in other nodes.
    */

    return BVStatus::BVSTATUS_OK;
}

BVTCPConnectionManager::~BVTCPConnectionManager()
{

}