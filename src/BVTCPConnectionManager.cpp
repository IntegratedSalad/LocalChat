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
    // Wait - is there already a connection with this peer/node?
    if (IsSessionAlreadyPresent(nodeData))
    {
        LogTrace("Session for {} already present (probably we accepted it).", nodeData.serviceName);
        return BVStatus::BVSTATUS_OK;
    }
    std::shared_ptr<BVTCPNodeConnectionSessionData> sessionData_p =
         std::make_shared<BVTCPNodeConnectionSessionData>(nodeData, ioContext, currentSessionID);
    sessionData_p->appCommChannel_p = this->appInMailBox_p;
    BVStatus registerStatus = 
        StartCommunicationSessionWithNode(sessionData_p->nodeData.id, sessionData_p->inMailbox_p);
    if (registerStatus == BVStatus::BVSTATUS_NOK)
    {
        LogInfo("BVTCPConnectionManager::InitiateSessionWithNode: Couldn't register communication channel");
        return registerStatus;
    }
    boost::asio::async_connect(*sessionData_p->sock, sessionData_p->nodeData.results, 
        std::bind(&BVTCPConnectionManager::ConnectHandler, this, std::placeholders::_1, std::placeholders::_2, sessionData_p));
    LogTrace("App: Trying to connect asynchronously...");
    // boost::asio::async_conn
    // boost::asio::async_connect()
    /*
        Okay, I have to research it more closely:
        In P2P connection, we do not create two sessions - one for incoming traffic and one for outgoing.
        Instead, we try to initiate/try to connect: if the peer (service) has already connected to US (we accepted),
        we use this session with its socket. If the peer (service) has already accepted our connection trial (we connected),
        then we use this session and socket. WE DO NOT create one listening session and one writing session - this causes a collision.
        So one session handles EVERY and ANY traffic between two peers/nodes.
        And certainly, we cannot create two sessions simultaneously.
        TCP connection is already bidirectional.
    */
    // std::shared_ptr<BVTCPSession> session_p = std::make_shared<BVTCPSession>(sessionData_p, ioContext);
    // {
    //     std::lock_guard<std::mutex> l(session_m_mutex);
    //     sessions_m[session_p->GetSessionData()->nodeData.id] = session_p;
    //     currentSessionID+=1;
    // }

    // LogTrace("BVTCPConnectionManager::InitiateSessionWithNode: Initiated session with node {}", nodeData.serviceName);
    // LogTrace("BVTCPConnectionManager::InitiateSessionWithNode: SessionID: {} NodeID: {}", 
    //     session_p->GetSessionData()->sessionID, session_p->GetSessionData()->nodeData.id);
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

    // Resolve first???
    // boost::asio::ip::tcp::resolver resolver{ioContext};
    // auto results = resolver.resolve(/*boost::asio::ip::tcp::v4()*/this->thisMachineServiceData.hostname, std::to_string(ntohs(thisMachineServiceData.port)), ec); // make that async

    boost::system::error_code ec;
    boost::asio::ip::tcp::endpoint ep{boost::asio::ip::address_v6::any(), ntohs(thisMachineServiceData.port)};
    this->acceptorSocket = boost::asio::ip::tcp::acceptor{ioContext};

    if (this->acceptorSocket.is_open())
    {
        this->acceptorSocket.close(ec);
        if (ec)
        {
            LogError("BVTCPConnectionManager: Couldn't close the acceptor socket. {} - {}", ec.value(), ec.message());
            throw std::runtime_error("Couldn't close the acceptor socket.");
        }
    }
    this->acceptorSocket.open(ep.protocol(), ec);
    if (ec)
    {
        LogError("BVTCPConnectionManager: Couldn't open the acceptor socket. {} - {}", ec.value(), ec.message());
        throw std::runtime_error("Couldn't open the acceptor socket.");
    }
    this->acceptorSocket.set_option(boost::asio::socket_base::reuse_address(true), ec);
    if (ec)
    {
        LogError("BVTCPConnectionManager: Couldn't set option 'reuse_address' for the acceptor socket. {} - {}", ec.value(), ec.message());
        throw std::runtime_error("Couldn't set option for the acceptor socket.");
    }
    this->acceptorSocket.set_option(boost::asio::ip::v6_only{false}, ec);
    if (ec)
    {
        LogError("BVTCPConnectionManager: Couldn't set option 'v6_only' for the acceptor socket. {} - {}", ec.value(), ec.message());
        throw std::runtime_error("Couldn't set option for the acceptor socket.");
    }
    this->acceptorSocket.bind(ep, ec);
    if (ec)
    {
        LogError("BVTCPConnectionManager: Couldn't bind the acceptor socket. {} - {}", ec.value(), ec.message());
        throw std::runtime_error("Couldn't bind the acceptor socket.");
    }
    LogTrace("BVTCPConnectionManager: Accepting connections on {}:{}... for service: {}",
        ep.address().to_string(), ep.port(), thisMachineServiceData.hostname);
    std::shared_ptr<BVTCPNodeConnectionSessionData> sessionData_p =
        std::make_shared<BVTCPNodeConnectionSessionData>(BVNode{}, ioContext, currentSessionID); // TODO: not thisMachineHostData!
    sessionData_p->appCommChannel_p = this->appInMailBox_p;
    this->acceptorSocket.listen(N_SERVICES_MAX, ec);
    if (ec)
    {
        LogError("BVTCPConnectionManager: Acceptor couldn't perform listening: {} - {}", ec.value(), ec.message());
        throw std::runtime_error("Acceptor couldn't perform listening");
    }
    // we pass the socket of this session
    this->acceptorSocket.async_accept(*sessionData_p->sock.get(), 
        [sessionData_p, this](const boost::system::error_code& error){
            if (!error)
            {
                // Wait - is there already a connection session with this peer/node?
                std::shared_ptr<BVTCPSession> session_p = 
                    std::make_shared<BVTCPSession>(sessionData_p, this->ioContext);

                this->LogTrace("Accept successful!");
                // construct session

                // We need to establish a handshake of sorts.
                // This node/peer has to send us back its service name/anything that we can identify it
                // and say for sure that the session with it wasn't initiated by connecting to it!
                // After that, when they send the id of sorts, we check the map.

                // Construct session.
                // Communicate and let them send us serviceName.
                // If this is a new peer/node -> at to map.
                // If not - discard

                {
                    std::lock_guard<std::mutex> l(session_m_mutex);
                    // write a function that increments the sessionID.
                    this->sessions_m[session_p->GetSessionData()->nodeData.id] = session_p;
                    this->currentSessionID+=1;
                }

            } else
            {
                this->LogError("Accept failed.");
            }
    });

        // maybe try to accept sycnhronously for now.
        // this->acceptorSocket.async_accept()
        // Create new Session

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
