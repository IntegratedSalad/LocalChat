#include "BVTCPConnectionManager.hpp"

BVTCPConnectionManager::BVTCPConnectionManager(boost::asio::io_context& _ioContext,
                                               const BVServiceData _thisMachineServiceData):
ioContext(_ioContext),
thisMachineServiceData(_thisMachineServiceData),
acceptorSocket(_ioContext)
{
    for (NodeID _id = 0; _id < N_SERVICES_MAX; _id++)
    {
        this->outMailboxes_p.emplace(std::make_pair(_id, nullptr));
    }

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
BVStatus BVTCPConnectionManager::InitiateSessionWithNode(const BVNode nodeData)
{
    std::shared_ptr<BVTCPNodeConnectionSessionData> sessionData_p =
         std::make_shared<BVTCPNodeConnectionSessionData>(nodeData, ioContext);

    sessionData_p->appCommChannel_p = this->appInMailBox_p;
    BVStatus registerStatus = 
        StartCommunicationSessionWithNode(sessionData_p->nodeData.id, sessionData_p->inMailbox_p);
    if (registerStatus == BVStatus::BVSTATUS_NOK)
    {
        LogError("BVTCPConnectionManager::InitiateSessionWithNode: Couldn't register communication channel");
        return registerStatus;
    }

    try {
        sessionData_p->sock.open(sessionData_p->nodeData.ep.protocol());
        {
            std::lock_guard<std::mutex> l(session_m_mutex);
            sessions_m[sessionData_p->nodeData.id] = sessionData_p;
        }
        sessionData_p->sock.connect(sessionData_p->nodeData.ep);
    } catch (boost::system::system_error& e)
    {
        LogError(
            "BVTCPConnectionManager::InitiateSessionWithNode Fatal error - establish connection with a socket: {}",
            e.what());
        return BVStatus::BVSTATUS_FATAL_ERROR;
    }

    // How to notify session that it needs to write something?
    // Shouldn't manager become a Component?
    // Or App just calls a manager function (interface)

    // TODO:  How will this session communicate with ConnectionManager and how will it communicate with App?
    // via the same broker?
    // Intantiate a new broker in app?
    // another, internal broker instantiated in BVTCPConnectionManager?
    // The connection instance here will have to have access to what App is producing

    // TODO: Pass inMailbox_p
    // Just share an inMailBox_p with every Session!
    // These messages will probably get shared with broker, but it doesn't matter.
    // That's for reading messages and passing them to App, so incoming traffic
    // But how to read from App?
    // Maybe create a shared pointer to a thread_safe queue which will be
    // a communication channel for outgoing messages to each Session ( App -> Session )
    // Maybe we need a map keyed by sessionID/nodeID with a threadsafe_queue,
    // something like in Broker, but not for internal messages but TCP traffic.
    // This is BVTCPConnectionManager's map, but it will be gettable for App. -> functions
    // providing an interface with the map
    // App just pushes things outwards, incoming traffic is directed to App's inMailbox_p.
    // This will be efficient, as BVTCPConnectionManager neither BVTCPSession will be Components. 

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