#include "BVApp_ConsoleClient.hpp"

BVApp_ConsoleClient::BVApp_ConsoleClient(const BVServiceData _thisMachineServiceData,
                                         std::shared_ptr<threadsafe_queue<BVMessage>> _outMbx,
                                         std::shared_ptr<threadsafe_queue<BVMessage>> _inMbx,
                                         boost::asio::io_context& _ioContext) :
BVApp(_ioContext, _thisMachineServiceData),
BVComponent(_outMbx, _inMbx)
{
    RegisterCallback(BVEventType::BVEVENTTYPE_APP_PUBLISHED_SERVICE,
                     std::bind(&BVApp_ConsoleClient::HandlePublishedServices, this, std::placeholders::_1));
    RegisterCallback(BVEventType::BVEVENTTYPE_TERMINATE_ALL,
                     std::bind(&BVApp_ConsoleClient::OnShutdown, this, std::placeholders::_1));
    RegisterCallback(BVEventType::BVEVENTTYPE_APP_DISCOVERY_SERVICE_RESOLVED,
                     std::bind(&BVApp_ConsoleClient::HandleResolvedServices, this, std::placeholders::_1));
    this->GetConnectionManager().SetAppInMailBoxP(_inMbx);
    this->GetConnectionManager().StartAcceptingConnections();

    // TODO: Create an auxhilary object which listens to messages
    //       coming from App to sessions and that should be routed from sessions
    //       to App AND route its traffic to global queue
    //       Or don't create other components - just make each session a component 
    //       and somehow redirect their produced messages into global queue
    //       We can just pass the pointer to the existing inMailbox_p
    // 
}

void BVApp_ConsoleClient::Run(void)
{
    // std::cerr << "stdin isatty = " << ::isatty(STDIN_FILENO) << '\n';
    this->terminal.SetNonCanonicalMode();
    PrintAll();
    while (this->GetIsRunning())
    {
        // continue; // uncomment when debugging - this hangs the main thread.
        const char key = this->terminal.ReadChar();
        auto action = ParseConsoleActionFromKey(key);
        if (!action.has_value())
        {
            continue; // actions not handled
        }
        switch (*action)
        {
            case BVConsoleActionType::BVCONSOLEACTION_REPRINT:
                // send reprint event/message
                PrintAll();
                break;
            case BVConsoleActionType::BVCONSOLEACTION_SENDMSG:
                // send sendmsg event/message
                break;
            case BVConsoleActionType::BVCONSOLEACTION_PAUSE_DISCOVERY:
                LogTrace("App: Pause discovery message sent.");
                SendMessage(BVMessage(
                    BVEventType::BVEVENTTYPE_DISCOVERY_REQUEST_PAUSE, nullptr));
                break;
            case BVConsoleActionType::BVCONSOLEACTION_RESUME_DISCOVERY:
                LogTrace("App: Resume discovery message sent.");
                SendMessage(BVMessage(
                    BVEventType::BVEVENTTYPE_DISCOVERY_REQUEST_RESUME, nullptr));
                break;
            case BVConsoleActionType::BVCONSOLEACTION_QUIT:
                // send quit event/message
                SendMessage(BVMessage(
                    BVEventType::BVEVENTTYPE_TERMINATE_ALL, nullptr));
                SetIsRunning(false);
                LogTrace("App: quitting. Sent TERMINATE_ALL message");
                break;
            case BVConsoleActionType::BVCONSOLEACTION_BLOCKHOST:
                // send blockhost event/message
                break;
        }

        // I don't know if this is right,
        // but I think every implementaiton of a UI
        // has to perform on it, only from the main thread.
        // If this is the case, the App shouldn't be blocked at all,
        // only tasks should be put on queue/something? and app only performs these tasks.

        // For now - let's make echoless input...
        // If write message input:
        // 1.Print new console - don't redraw it untill on main screen
        // 2.Choose a recipient - new screen with all hosts
        // 3.Choose message - turn echo on.
    }
}

// I think that any event that needs to draw something
// must redraw everything
void BVApp_ConsoleClient::PrintAll(void)
{
    for (int i = 0; i < 200; i++) {std::cout << std::endl;}
    std::cout << "LocalChat console client v0.2.2" << std::endl;
    std::cout << "Re(D)raw" << std::endl;
    std::cout << "Send (M)essage" << std::endl;
    std::cout << "(P)ause discovery" << std::endl;
    std::cout << "(R)esume discovery" << std::endl;
    std::cout << "(Q)uit" << std::endl;
    std::cout << "-----------------------------" << std::endl;
    std::cout << "Available services:" << std::endl;
    this->PrintServices();
    // statuses like is discovery paused...
    std::cout << "=============================" << std::endl;
    std::cout << std::flush;
}

BVStatus BVApp_ConsoleClient::PrintServices(void)
{
    // std::lock_guard<std::mutex> l(this->serviceVectorMutex);
    BVStatus status = BVStatus::BVSTATUS_OK;
    if (this->serviceV.size() == 0)
    {
        std::cout << "None available apart from ourselves... :(" << std::endl;
        std::cout << this->GetThisMachineServiceData().hostname << std::endl;
    }
    int i = 1;
    for (BVServiceBrowseInstance& bI : this->serviceV)
    {
        std::cout << i++ << ":" << std::endl;
        bI.print();
        std::cout << "+-+-+-+-" << std::endl;
    }
    return status;
}

BVStatus BVApp_ConsoleClient::HandlePublishedServices(std::unique_ptr<std::any> dp)
{
    using BVServiceBrowseInstanceList = std::list<BVServiceBrowseInstance>;
    if (dp == nullptr)
    {
        LogError("App: No new services received.");
        return BVStatus::BVSTATUS_FATAL_ERROR;
    }
    BVServiceBrowseInstanceList newServicesList;
    try
    {
        newServicesList = std::any_cast<BVServiceBrowseInstanceList>(*dp);    
    }
    catch(const std::bad_any_cast& e)
    {
        LogError("App: Bad cast in BVEventType::BVEVENTTYPE_APP_PUBLISHED_SERVICE callback.");
        return BVStatus::BVSTATUS_FATAL_ERROR;
    }
    // Update service vector.
    // Does it need to be guarded? I think so, because here we are modifying it.
    // Mock client will periodically read it, but real user in the product implementation
    // will try to read it and they might do it when this is updated here

    std::vector<BVServiceBrowseInstance> toResolve;
    LogTrace("App: HandlePublishedServices is called.");
    {
        std::lock_guard<std::mutex> l(this->serviceVectorMutex);
        for (auto& lElem : newServicesList)
        {
            if ((std::find(this->serviceV.begin(), this->serviceV.end(), lElem) == this->serviceV.end()))
            {
                const BVServiceData& thisMachineServiceData = GetThisMachineServiceData();
                LogDebug(thisMachineServiceData.domain.c_str());
                LogDebug(thisMachineServiceData.regtype.c_str());
                LogDebug(thisMachineServiceData.hostname.c_str());
                LogDebug(lElem.replyDomain.c_str());
                LogDebug(lElem.regType.c_str());
                LogDebug(lElem.serviceName.c_str());
                if (lElem.regType == thisMachineServiceData.regtype &&
                    lElem.serviceName == thisMachineServiceData.hostname &&
                    lElem.replyDomain == thisMachineServiceData.domain)
                {
                    continue; // do not resolve service on the same machine
                }
                this->serviceV.push_back(lElem);
                toResolve.push_back(lElem);
                // Send request to resolve
                // Should we exchange messages here or just resolve 
                // in Discovery after browsing there?
                // And report once we have everything (browsing + resolved hostname)
                // First - let's try exchanging messages.
                // Also - if we do Resolution straight in the BVDiscovery,
                // we might do repeat it for the same service.
                // Maybe also take note in BVDiscovery
            }
        }
    }
    {
        for (auto& lElem : toResolve)
        {
            SendMessage(BVMessage(
                    BVEventType::BVEVENTTYPE_DISCOVERY_REQUEST_RESOLVE,
                        std::make_unique<std::any>(std::make_any<BVServiceBrowseInstance>(lElem))));
            LogTrace("App: Sending request to Discovery to resolve {}", lElem.serviceName);
        }
    }
    // this is called from different thread
    PrintAll();

    // Should we resolve here? Maybe just send a request to Discovery to resolve
    return BVStatus::BVSTATUS_OK;
}

BVStatus BVApp_ConsoleClient::HandleResolvedServices(std::unique_ptr<std::any> dp)
{
    LogTrace("App: HandleResolvedServices ENTER");
    if (dp == nullptr)
    {
        LogError("App: Error - HandleResolvedServices, data pointer is null!");
        return BVStatus::BVSTATUS_FATAL_ERROR;
    }
    DNSResolutionResult* res;
    try
    {
        res = std::any_cast<DNSResolutionResult*>(*dp);
    }
    catch(const std::bad_any_cast& e)
    {
        std::cerr << "Bad cast in BVEventType::BVEVENTTYPE_APP_DISCOVERY_SERVICE_RESOLVED callback. " 
                    << e.what() << std::endl;
        LogError("App: Bad cast in HandleResolvedServices! Error details: {}", e.what());
        return BVStatus::BVSTATUS_FATAL_ERROR;
    }

    std::string hosttarget  = res->hosttarget;
    std::string serviceName = res->serviceName;
    int         port        = res->port;

    LogTrace("App: Resolved {} to hosttarget: {}", serviceName, hosttarget);
    LogTrace("App: on port {}", port);

    BVNode node = ResolveServiceToEndpoint(hosttarget, serviceName, port);
    if (node.serviceName == "ERROR")
    {
        return BVStatus::BVSTATUS_FATAL_ERROR;
    }
    nodesV.push_back(node);

    // Initiate connection (session)
    // Open socket.
    // Spawn thread
    // Will this connection listen to anything that other endpoint says?
    // Will these connections be persistent?
    // Start with initiating connection to an endpoint
    this->GetConnectionManager().InitiateSessionWithNode(node);

    // Very important, as we manually allocate DNSResolutionResult in C_ResolveReply!!!
    ::free(res);
    return BVStatus::BVSTATUS_OK;
}

void BVApp_ConsoleClient::PrintNewServicesNotification(void)
{
    std::lock_guard stdoutlk{this->stdoutMutex};
    std::cout << "New services received!" << std::endl;
}

BVStatus BVApp_ConsoleClient::OnStart(std::unique_ptr<std::any>)
{
    return BVStatus::BVSTATUS_OK;
}

BVStatus BVApp_ConsoleClient::OnResume(std::unique_ptr<std::any>)
{
    return BVStatus::BVSTATUS_OK;
}

BVStatus BVApp_ConsoleClient::OnShutdown(std::unique_ptr<std::any>)
{
    StopIOContext();
    LogTrace("App: Shutting down...");
    return BVStatus::BVSTATUS_OK;
}

BVStatus BVApp_ConsoleClient::OnRestart(std::unique_ptr<std::any>)
{
    return BVStatus::BVSTATUS_OK;
}

BVStatus BVApp_ConsoleClient::OnPause(std::unique_ptr<std::any>)
{
    return BVStatus::BVSTATUS_OK;
}

std::optional<BVConsoleActionType> BVApp_ConsoleClient::ParseConsoleActionFromKey
(char key)
{
    switch (static_cast<char>(std::tolower(static_cast<unsigned char>(key))))
    {
        case 'd':
            return BVConsoleActionType::BVCONSOLEACTION_REPRINT;
        case 'm':
            return BVConsoleActionType::BVCONSOLEACTION_SENDMSG;
        case 'p':
            return BVConsoleActionType::BVCONSOLEACTION_PAUSE_DISCOVERY;
        case 'r':
            return BVConsoleActionType::BVCONSOLEACTION_RESUME_DISCOVERY;
        case 'q':
            return BVConsoleActionType::BVCONSOLEACTION_QUIT;
        case 'b':
            return BVConsoleActionType::BVCONSOLEACTION_BLOCKHOST;
        default:
            return std::nullopt;
    }
}

BVNode BVApp_ConsoleClient::ResolveServiceToEndpoint(const std::string& hosttarget, const std::string& serviceName, const int port)
{
    BVNode nodeData{};
    boost::system::error_code ec;
    boost::asio::ip::tcp::resolver resolver{GetIoContext()};
    // TODO: probably we want to do ntohs on the port!
    auto results = resolver.resolve(/*boost::asio::ip::tcp::v4(), */hosttarget, std::to_string(port), ec); // make that async

    if (ec)
    {
        LogError("App: Error while resolving to IPv4... {}", ec.to_string());
        return nodeData;
    }
    if (results.empty())
    {
        LogError("App: Endpoints empty...");
        nodeData.serviceName = "ERROR";
        return nodeData;
    }
    boost::asio::ip::tcp::endpoint endpoint = results.begin()->endpoint(); // try first endpoint
    LogTrace("Successfuly resolved {} to {}", serviceName, endpoint.address().to_string());
    nodeData.ep = endpoint;
    nodeData.address = endpoint.address();
    nodeData.hostname = hosttarget;
    nodeData.serviceName = serviceName; 
    nodeData.results = results;
    return nodeData;
}