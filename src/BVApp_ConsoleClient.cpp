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
    RegisterCallback(BVEventType::BVEVENTTYPE_APP_MESSAGE_INCOMING,
                     std::bind(&BVApp_ConsoleClient::HandleMessageIncoming, this, std::placeholders::_1));
    RegisterCallback(BVEventType::BVEVENTTYPE_APP_FILE_TRANSFER_BEGIN,
                     std::bind(&BVApp_ConsoleClient::HandleFileTransferBegin, this, std::placeholders::_1));

    // Set getter that returns a correct pointer to Apps inMailBox
    this->GetConnectionManager().SetMailboxGetterF(
        [this]() -> std::shared_ptr<threadsafe_queue<BVMessage>>
        {
            return this->GetInMailBox();
        }
    );
    // this->GetConnectionManager().SetAppInMailBoxP(_inMbx);
    // maybe set a callback for connection manager to be GetInMailBox

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
    this->terminal.SetNonCanonicalMode();
    PrintAll();
    while (this->GetIsRunning())
    {
        // continue; // uncomment when debugging - this hangs the main thread.
        ClearScreen();
        PrintAll();
        const char key = this->terminal.ReadChar();
        auto action = ParseConsoleActionFromKey(key);
        bool sendingFile = false;
        if (!action.has_value())
        {
            continue; // actions not handled
        }
        auto type = (*action).type;
        switch (type)
        {
            case BVConsoleActionType::BVCONSOLEACTION_REPRINT:
                PrintAll();
                break;
            case BVConsoleActionType::BVCONSOLEACTION_SENDMSG: // also views messages
            {
                // send sendmsg event/message
                // Choose host
                bool runMessagingMenu = true;
                LogTrace("App: Choosing sending message...");
                const auto hostChosen = (*action).num;
                if (hostChosen.has_value())
                {
                    const int idx = hostChosen.value(); // this is only the idx in the vector!
                    LogDebug("App: chosen idx: {}", idx);
                    bool found = false;
                    try {
                        int nodeIdx = 0;
                        std::string serviceName;
                        for (const auto& [k,v] : this->GetConnectionManager().GetNodesM())
                        {
                            if (idx == nodeIdx)
                            {
                                found = true;
                                serviceName = v.serviceName;
                                LogDebug("App: At {} there's {}. Entering messaging menu.", nodeIdx, serviceName);
                                while (runMessagingMenu)
                                {
                                    ClearScreen();
                                    PrintAll();
                                    {
                                        std::lock_guard<std::mutex> l(chatLogsMapMutex);
                                        BVChatMessageLog log;
                                        try
                                        {
                                            log = chatLogsM.at(serviceName);
                                            std::cout << "Message log with " << serviceName << std::endl;
                                            log.PrintNLastMessages(TEN_LAST_MESSAGES);
                                        }
                                        catch(const std::out_of_range& e)
                                        {
                                            std::cout << "No messages with " << serviceName << " :)" << std::endl;
                                        }
                                    }
                                    const std::string prompt("|q OR |r OR >> ");
                                    const std::string msgStr = terminal.PromptLine(prompt);
                                    LogDebug("App: Gotten msg string: {}", msgStr);
                                    // split string after space
                                    std::string argStr{};
                                    std::string::size_type argPos = msgStr.find(' ');
                                    if (argPos == 2)
                                    {
                                        argStr = msgStr.substr(argPos+1);
                                    }
                                    if (msgStr.length() == 2)
                                    {
                                        if (msgStr == "|q")
                                        {
                                            runMessagingMenu = false;
                                            continue;
                                        }
                                        if (msgStr == "|r")
                                        {
                                            continue;
                                        }
                                    } else if (msgStr.length() > 2)
                                    {
                                        if (argStr.length() > 0)
                                        {
                                            if (msgStr.find("|f") != std::string::npos)
                                            {
                                                sendingFile = true;
                                            }
                                        }
                                    }
                                    SessionID sid;
                                    BVStatus sidStatus = GetConnectionManager().GetSessionIDFromServiceName(serviceName, sid);
                                    if (sidStatus != BVStatus::BVSTATUS_OK)
                                    {
                                        LogError("Couldn't get sid from {}", serviceName);
                                        break;
                                    }
                                    if (sendingFile)
                                    {
                                        // 1. Get file but don't load it all to the memory - get descriptor/load only chunk.
                                        // 2. Get file size and determine file chunk size.
                                        // 3. Write some sort of utility for BVTCPConnectionManager
                                        //    that allows for continuous file sending
                                        //    probably a thread must be spawned (which can also listen for stop)
                                        //    maybe a file sender component? or a very simple
                                        //    object that spawns a mutex and conditional variable
                                        //    it can have an inMailBox of App, and it can
                                        //    send messages telling how much bytes it has sent 
                                        //    Maybe BVFileTransferContext?
                                        //    class that holds information about sent file.
                                        //    it is put on a separate thread and receives
                                        //    inMailBox
                                        /*
                                            File sending:
                                            Send _FILE_TRANSFER_BEGIN
                                            Payload:
                                            Size
                                            Wait for _CONFIRM_CAN_RECEIVE_FILE (if there's space on the other machine)
                                            We can also just omit this, as there's no much time left...
                                            If that takes too long, then just scrap the confirmation and send anyway (ecksdee).
                                        */
                                        LogDebug("[BVApp_ConsoleClient]: Sending file, path: {}", argStr);
                                        std::filesystem::path filePath = argStr;
                                        if (std::filesystem::exists(filePath))
                                        {
                                            LogDebug("[BVApp_ConsoleClient]: File {} exists!", argStr);
                                            GetConnectionManager().InitiateFileTransferWithSession(sid, filePath);
                                            LogDebug("[BVApp_ConsoleClient]: Initiated file transfer with session: {}", sid);
                                        } else
                                        {
                                            LogDebug("[BVApp_ConsoleClient]: File {} does not exist!", argStr);
                                        }
                                        sendingFile = false;
                                        continue;
                                    }

                                    std::unique_ptr<BVTCPMessage<BVChatMessagePayload>> chatMsg = ConstructChatMessageFromInput(msgStr);
                                    uint64_t timestamp = chatMsg->header.timestamp;
                                    BVStatus sentStatus = GetConnectionManager().SendDataToNode(std::move(chatMsg), sid);
                                    if (sentStatus == BVStatus::BVSTATUS_FATAL_ERROR)
                                    {
                                        std::cout << "Error: No session! Check logs or contact support or something." << std::endl;
                                        break;
                                    }
                                    {
                                        std::lock_guard<std::mutex> l(chatLogsMapMutex);
                                        try
                                        {
                                            chatLogsM.at(serviceName).AddMessage(BVChatMessage(msgStr, timestamp, GetThisMachineServiceData().hostname));
                                        }
                                        catch(const std::out_of_range& e)
                                        {
                                            BVChatMessageLog log{serviceName, BVChatMessage(msgStr, timestamp, GetThisMachineServiceData().hostname)};
                                            chatLogsM.emplace(serviceName, log);
                                        }
                                    }
                                }
                                // ClearScreen();
                                // PrintAll();
                                break;
                            }
                            nodeIdx++;
                        }
                        if (!found)
                        {
                            LogWarn("Not found any service at idx {}", nodeIdx);
                            break;
                        }
                        // Enumerate nodesM
                        // choose node at nodesM
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
                // BVTCPMessageHeader header = ConstructMessageHeader(
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
    std::cout << "(0-9) Choose host to send message to" << std::endl;
    std::cout << "(P)ause discovery" << std::endl;
    std::cout << "(R)esume discovery" << std::endl;
    std::cout << "(Q)uit" << std::endl;
    std::cout << "-----------------------------" << std::endl;
    std::cout << "Available services:" << std::endl;
    this->PrintServices();
    // TODO: statuses like is discovery paused...
    std::cout << "-----------------------------" << std::endl;
    std::cout << "Sessions established:" << std::endl;
    this->GetConnectionManager().PrintSessions();
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
                LogTrace("App, HandlePublishedServices: Added {} to serviceV", lElem.serviceName);
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

    auto _nodesM = this->GetConnectionManager().GetNodesM();
    auto it = _nodesM.find(serviceName);
    if (it == _nodesM.end())
    {
        GetConnectionManager().AddNodeToNodesM(serviceName, node);
        LogTrace("App: Added node representing service {} to nodesM", serviceName);
    } else
    {
        LogWarn("App: Node representing service {} already present in nodesM", serviceName);
        ::free(res);
        return BVStatus::BVSTATUS_OK;
    }

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
        std::lock_guard<std::mutex> l(serviceVectorMutex);
        for (auto& lElem : newServicesList)
        {
            const auto oldSize = serviceV.size();
            serviceV.erase(
                std::remove_if(serviceV.begin(),
                               serviceV.end(),
                               [&](const BVServiceBrowseInstance& s)
                               {
                                    return s.serviceName == lElem.serviceName;
                               }),
                               serviceV.end()
            );
            if (serviceV.size() < oldSize)
            {
                GetConnectionManager().GetNodesM().erase(lElem.serviceName);
                LogTrace("App, HandleServiceDeregistration: removed {}.", lElem.serviceName);
                this->GetConnectionManager().RemoveSession(lElem.serviceName);
            } else
            {
                LogWarn("App, HandleServiceDeregistration: {} not found in serviceV!", lElem.serviceName);
            }
            LogInfo("App, HandleServiceDeregistration: Currently: {} Services in serviceV:", serviceV.size());
            int idx = 1;
            for (const auto& s : serviceV)
            {
                LogInfo("{}: {}", idx, s.serviceName);
                idx++;
            }
        }
    }
    PrintAll();
    return BVStatus::BVSTATUS_OK;
}

BVStatus BVApp_ConsoleClient::HandleMessageIncoming(std::unique_ptr<std::any> dp)
{
    LogTrace("[BVApp_ConsoleClient]: Received HandleMessageIncoming");
    if (dp == nullptr)
    {
        LogError("App: Error - HandleResolvedServices, data pointer is null!");
        return BVStatus::BVSTATUS_FATAL_ERROR;
    }
    BVChatMessage res;
    try
    {
        res = std::any_cast<BVChatMessage>(*dp);
    }
    catch(const std::bad_any_cast& e)
    {
        std::cerr << "Bad cast in BVEventType::BVEVENTTYPE_APP_MESSAGE_INCOMING callback. "
                    << e.what() << std::endl;
        LogError("[BVApp_ConsoleClient]: Bad cast in HandleMessageIncoming! Error details: {}", e.what());
        return BVStatus::BVSTATUS_FATAL_ERROR;
    }

    const std::string textData  = res.textData;  
    const std::string sender    = res.sender;
    const uint64_t    timestamp = res.timestamp;

    LogTrace("[BVApp_ConsoleClient]: Received message: {} from: {} at: {}",
        textData, sender, timestamp);
    {
        std::lock_guard<std::mutex> l(chatLogsMapMutex);
        try
        {
            chatLogsM.at(sender).AddMessage(res);
        }
        catch(const std::out_of_range& e)
        {
            BVChatMessageLog log{sender, res};
            chatLogsM.emplace(sender, log);
        }
    }
    return BVStatus::BVSTATUS_OK;
}

BVStatus BVApp_ConsoleClient::HandleFileTransferBegin(std::unique_ptr<std::any> dp)
{
    LogTrace("[BVApp_ConsoleClient]: Received HandleFileTransferBegin");
    if (dp == nullptr)
    {
        LogError("[BVApp_ConsoleClient]: Error - HandleFileTransferBegin, data pointer is null!");
        return BVStatus::BVSTATUS_FATAL_ERROR;
    }
    BVTCPFileData res;
    try
    {
        res = std::any_cast<BVTCPFileData>(*dp);
    }
    catch(const std::bad_any_cast& e)
    {
        std::cerr << "Bad cast in BVEventType::BVEVENTTYPE_APP_FILE_TRANSFER_BEGIN callback. "
                    << e.what() << std::endl;
        LogError("[BVApp_ConsoleClient]: Bad cast in HandleFileTransferBegin! Error details: {}", e.what());
        return BVStatus::BVSTATUS_FATAL_ERROR;
    }

    const uint32_t    csize  = res.csize;  
    const uint32_t    fsize  = res.fsize;
    const std::vector<char> fdata  = res.fdata; // service name and filename!
    const std::string fdataStr{fdata.begin(), fdata.end()};
    const std::string serviceName = fdataStr.substr(0, fdataStr.find('|'));
    const std::string fname       = fdataStr.substr(fdataStr.find("|")+1);

    // TODO: Parse fdata, create a context/something for the incoming file.
    //       Create directory under the service name and open file with the name provided.

    LogTrace("[BVApp_ConsoleClient]: File size: {} Chunk size: {} Payload: {}\nFrom: {} Name: {}",
        fsize, csize, fdataStr, serviceName, fname);

    const std::filesystem::path rootdir = std::filesystem::current_path() / "..";
    try
    {
        const std::filesystem::path dirpath  = rootdir / "data" / serviceName;
        const std::filesystem::path filepath = dirpath / fname;
        if (std::filesystem::is_directory(dirpath))
        {
            LogTrace("[BVApp_ConsoleClient]: Directory already exists: {}", dirpath.string());
        } else
        {
            if (std::filesystem::create_directory(rootdir / "data" / serviceName))
            {
                LogTrace("[BVApp_ConsoleClient]: Directory created at: ", rootdir.string());
                if (std::filesystem::is_regular_file(filepath))
                {
                    LogTrace("[BVApp_ConsoleClient]: File already exists: {}", filepath.string());
                } else
                {
                    std::ofstream incomingFile(filepath, std::ios::binary | std::ios::out);
                    LogTrace("[BVApp_ConsoleClient]: Created file at: {}", filepath.string());
                }
            } else
            {
                LogError("[BVApp_ConsoleClient]: Directory not created.");
            }
        }
    }
    catch(const std::exception& e)
    {
        LogError("[BVApp_ConsoleClient]: Error while creating directory and/or file: {}", e.what());
        return BVStatus::BVSTATUS_NOK;
    }
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
        // case 'm':
        //     return ParsingResult{BVConsoleActionType::BVCONSOLEACTION_SENDMSG, std::nullopt};
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

std::unique_ptr<BVTCPMessage<BVChatMessagePayload>> BVApp_ConsoleClient::ConstructChatMessageFromInput(
    const std::string& inputString)//, const NodeID nodeID)
{
    std::unique_ptr<BVTCPMessage<BVChatMessagePayload>> msg = 
        std::make_unique<BVTCPMessage<BVChatMessagePayload>>();
    std::chrono::milliseconds ts = 
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch());
    msg->header.timestamp = ts.count();
    msg->header.msgType = 
        static_cast<uint8_t>(BVTCPMessageType::BVSESSIONREGULARMESSAGETYPE_CHATMESSAGE);
    msg->payload.textData.fill('\0');

    const std::size_t maxLen = msg->payload.textData.size();
    const std::size_t copyLen = std::min(inputString.size(), maxLen);
    std::memcpy(msg->payload.textData.data(), inputString.data(), copyLen);

    msg->header.dataLen = static_cast<uint8_t>(copyLen);

    return msg;
}
