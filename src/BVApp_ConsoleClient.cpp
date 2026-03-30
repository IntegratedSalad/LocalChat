#include "BVApp_ConsoleClient.hpp"

BVApp_ConsoleClient::BVApp_ConsoleClient(std::shared_ptr<threadsafe_queue<BVMessage>> _outMbx,
                                         std::shared_ptr<threadsafe_queue<BVMessage>> _inMbx) :
BVComponent(_outMbx, _inMbx)
{
    RegisterCallback(BVEventType::BVEVENTTYPE_APP_PUBLISHED_SERVICE,
                     std::bind(&BVApp_ConsoleClient::HandlePublishedServices, this, std::placeholders::_1));
    RegisterCallback(BVEventType::BVEVENTTYPE_TERMINATE_ALL,
                     std::bind(&BVApp_ConsoleClient::OnShutdown, this, std::placeholders::_1));
}

void BVApp_ConsoleClient::Run(void)
{
    std::cerr << "stdin isatty = " << ::isatty(STDIN_FILENO) << '\n';
    this->terminal.SetNonCanonicalMode();
    PrintAll();
    while (this->GetIsRunning())
    {
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
    std::cout << "LocalChat console client v0.2.1.2.2a" << std::endl;
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
        std::cout << "None available" << std::endl;
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

    LogTrace("App: HandlePublishedServices is called.");
    std::lock_guard<std::mutex> l(this->serviceVectorMutex);
    for (auto& lElem : newServicesList)
    {
        if ((std::find(this->serviceV.begin(), this->serviceV.end(), lElem) == this->serviceV.end()))
        {
            this->serviceV.push_back(lElem);
        }
    }
    // this is called from different thread
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