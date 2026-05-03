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
    RegisterCallback(BVEventType::BVEVENTTYPE_APP_DEREGISTERED_SERVICE,
                     std::bind(&BVApp_ConsoleClient::HandleServiceDeregistration, this, std::placeholders::_1));

    this->GetConnectionManager().SetAppInMailBoxP(_inMbx);

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
        auto type = (*action).type;
        switch (type)
        {
            case BVConsoleActionType::BVCONSOLEACTION_REPRINT:
                // send reprint event/message
                PrintAll();
                break;
            case BVConsoleActionType::BVCONSOLEACTION_SENDMSG:
            {
                // send sendmsg event/message
                // Choose host
                LogDebug("App: Choosing sending message...");
                const auto hostChosen = (*action).num;
                if (hostChosen.has_value())
                {
                    const int idx = hostChosen.value(); // this is only the idx in the vector!
                    LogDebug("App: chosen idx: {}", idx);
                    bool found = false;
                    try {

                        // BVNode node = nodesV.at(idx);
                        // ClearScreen();
                        // const std::string msgStr = this->terminal.GetStringFromSTDIN("Enter message: ");
                        // std::unique_ptr<BVChatMessage> chatMsg = ConstructChatMessageFromInput(msgStr, node.id);

                    } catch (const std::out_of_range& ex)
                    {
                        LogInfo("App: There's no Node at {}", idx);
                    }
                } else
                {
                    LogError("App: Optional does not have value!");
                }
                break;
            }
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
            {
                // using CharPayload128B = std::array<char, 128>;
                // using GoodbyeMsg = BVTCPMessage<CharPayload128B>;
                // // Send message that we are deregistering
                // // TODO ...
                // BVTCPMessageHeader header = ConstructHeader(
                //     BVTCPMessageType::BVSESSIONCONTROLMESSAGETYPE_NODESESSION_GOODBYE);
                // CharPayload128B payloadRaw;
                // const std::string& serviceNameToCopy = 
                //     this->GetThisMachineServiceData().hostname;
                // std::copy(serviceNameToCopy.begin(), serviceNameToCopy.end(), payloadRaw.data());
                // GoodbyeMsg goodbyeMsg = ConstructMessage(header, payloadRaw);
                // goodbyeMsg.header.dataLen = this->GetThisMachineServiceData().hostname.length();
                // this->GetConnectionManager().SendDataToEveryone(goodbyeMsg);
                // send quit event/message
                SendMessage(BVMessage(
                    BVEventType::BVEVENTTYPE_TERMINATE_ALL, nullptr));
                SetIsRunning(false);
                LogTrace("App: quitting. Sent TERMINATE_ALL message and BVEVENTTYPE_APP_SERVICE_DEREGISTERED to everyone");
                break;
            }
            case BVConsoleActionType::BVCONSOLEACTION_BLOCKHOST:
                // send blockhost event/message
                break;
        }
    }
}

inline void BVApp_ConsoleClient::ClearScreen(void)
{
    for (int i = 0; i < 200; i++) {std::cout << std::endl;}
}

// I think that any event that needs to draw something
// must redraw everything
void BVApp_ConsoleClient::PrintAll(void)
{
    ClearScreen();
    std::cout << "LocalChat console client v0.4.0" << std::endl;
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
        std::cout << "None available apart from ours... :(" << std::endl;
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
                // LogDebug("This machine: domain: {}", thisMachineServiceData.domain.c_str());
                // LogDebug("This machine: regtype: {}", thisMachineServiceData.regtype.c_str());
                // LogDebug("This machine: hostname: {}", thisMachineServiceData.hostname.c_str());
                // LogDebug("Found domain: {}", lElem.replyDomain.c_str());
                // LogDebug("Found regtype: {}", lElem.regType.c_str());
                // LogDebug("Found hostname/servicename: {}", lElem.serviceName.c_str());
                // if (lElem.regType == thisMachineServiceData.regtype &&
                //     lElem.serviceName == thisMachineServiceData.hostname &&
                //     lElem.replyDomain == thisMachineServiceData.domain)
                if (lElem.serviceName == thisMachineServiceData.hostname)
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
    BVStatus status = BVStatus::BVSTATUS_OK;
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
    nodesM[serviceName] = node;

    // Initiate connection (session)
    // Open socket.
    // Will this connection listen to anything that other endpoint says?
    // Will these connections be persistent?
    // Start with initiating connection to an endpoint

    status = this->GetConnectionManager().InitiateSessionWithNode(node);

    if (status == BVStatus::BVSTATUS_FATAL_ERROR)
    {
        LogError("Couldn't Initiate Session with a node! {}:{} [{}]", node.hostname, node.port, node.address.to_string());
    }

    // Very important, as we manually allocate DNSResolutionResult in C_ResolveReply!!!
    ::free(res);
    return status;
}

BVStatus BVApp_ConsoleClient::HandleServiceDeregistration(std::unique_ptr<std::any> dp)
{
    LogTrace("BVApp_ConsoleClient: HandleServiceDeregistration called");

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
    {
        // TODO: debug this...
        std::lock_guard<std::mutex> l(serviceVectorMutex);
        for (auto& lElem : newServicesList)
        {
            if (serviceV.size() != 1)
            {
                if ( serviceV.erase(std::remove(serviceV.begin(), serviceV.end(), lElem), serviceV.end()) !=
                    serviceV.end())
                {
                    nodesM.erase(lElem.serviceName);
                    LogTrace("App, HandleServiceDeregistration: removed {}.", lElem.serviceName);
                    this->GetConnectionManager().RemoveSession(lElem.serviceName);
                } else
                {
                    LogWarn("App, HandleServiceDeregistration: {} not found in serviceV!", lElem.serviceName);
                }
            } else
            {
                if (lElem == serviceV[0])
                {
                    serviceV.clear();
                    nodesM.erase(lElem.serviceName);
                    LogTrace("App, HandleServiceDeregistration: removed {}.", lElem.serviceName);
                    this->GetConnectionManager().RemoveSession(lElem.serviceName);
                } else
                {
                    LogWarn("App, HandleServiceDeregistration: {} not found in serviceV!", lElem.serviceName);
                }
            }
        }
    }
    PrintAll();
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

std::optional<ParsingResult> BVApp_ConsoleClient::ParseConsoleActionFromKey
(char key)
{
    unsigned char ukey = static_cast<unsigned char>(key);
    switch (static_cast<char>(std::tolower(static_cast<unsigned char>(key))))
    {
        case 'd':
            return ParsingResult{BVConsoleActionType::BVCONSOLEACTION_REPRINT, std::nullopt};
        case 'm':
            return ParsingResult{BVConsoleActionType::BVCONSOLEACTION_SENDMSG, std::nullopt};
        case 'p':
            return ParsingResult{BVConsoleActionType::BVCONSOLEACTION_PAUSE_DISCOVERY, std::nullopt};
        case 'r':
            return ParsingResult{BVConsoleActionType::BVCONSOLEACTION_RESUME_DISCOVERY, std::nullopt};
        case 'q':
            return ParsingResult{BVConsoleActionType::BVCONSOLEACTION_QUIT, std::nullopt};
        case 'b':
            return ParsingResult{BVConsoleActionType::BVCONSOLEACTION_BLOCKHOST, std::nullopt};
        default:
            break;
    }
    if (std::isdigit(ukey))
    {
        return ParsingResult{BVConsoleActionType::BVCONSOLEACTION_SENDMSG, key - '0'};
    }
    return std::nullopt;
}

BVNode BVApp_ConsoleClient::ResolveServiceToEndpoint(const std::string& hosttarget, const std::string& serviceName, const int port)
{
    LogTrace("BVApp_ConsoleClient::ResolveServiceToEndpoint: Resolving host {} on port: {}", hosttarget, port);
    BVNode nodeData{};
    boost::system::error_code ec;
    boost::asio::ip::tcp::resolver resolver{GetIoContext()};
    
    boost::asio::ip::tcp::resolver::results_type results;
    for (int attempt = 0; attempt < 5; attempt++)
    {
        ec.clear();
        results = resolver.resolve(/*boost::asio::ip::tcp::v4()*/hosttarget, std::to_string(port), ec); // make that async
        if (!ec && !results.empty())
        {
            break;
        }
        LogWarn("App: Resolve attempt failed... Retrying...");
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    if (ec && results.empty())
    {
        LogError("App: Error while resolving to... {}", ec.to_string());
        LogError("App: Error while resolving info {} {}", ec.message(), ec.category().name());
        nodeData.serviceName = "ERROR";
        return nodeData;
    }

    // auto results = resolver.resolve(/*boost::asio::ip::tcp::v4()*/hosttarget, std::to_string(port), ec); // make that async
    // TODO: There's a problem with this resolution!! Probably

    /* 
        This sometimes fail.
        We have to use DNSServiceGetAddrInfo...
        For now, this workaround is ok.
    */

    // if (ec)
    // {
    //     LogWarn("App: Error while resolving to... {}", ec.to_string());
    //     LogWarn("App: Error while resolving info {} {}", ec.message(), ec.category().name());
    // }
    // if (results.empty())
    // {
    //     LogError("App: Endpoints empty...");
    //     nodeData.serviceName = "ERROR";
    //     return nodeData;
    // }
    boost::asio::ip::tcp::endpoint endpoint = results.begin()->endpoint(); // try first endpoint
    LogTrace("Successfuly resolved {} to {}", serviceName, endpoint.address().to_string());
    nodeData.ep = endpoint;
    nodeData.address = endpoint.address();
    nodeData.hostname = hosttarget;
    nodeData.serviceName = serviceName; 
    nodeData.results = results;
    nodeData.port = port;
    return nodeData;
}

std::unique_ptr<BVTCPMessage<BVChatMessage>> BVApp_ConsoleClient::ConstructChatMessageFromInput(
    const std::string& inputString, const NodeID nodeID)
{
    BVTCPMessage<BVChatMessage> msg;
    std::chrono::milliseconds ts = 
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch());
    msg.header.timestamp = ts.count();

    // msgType
    // dataLen
    msg.payload.textData = inputString;

    return std::make_unique<BVTCPMessage<BVChatMessage>>(msg);
}