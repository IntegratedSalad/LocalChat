#include "BVApp_GUIClient.hpp"

#include <algorithm>
#include <any>
#include <chrono>
#include <cstring>
#include <fstream>
#include <list>
#include <sstream>
#include <thread>

#include <wx/filedlg.h>
#include <wx/listbox.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/wx.h>

class BVLocalChatFrame : public wxFrame
{
private:
    BVApp_GUIClient& app;
    wxListBox* servicesList{nullptr};
    wxListBox* sessionsList{nullptr};
    wxTextCtrl* chatLog{nullptr};
    wxTextCtrl* messageInput{nullptr};
    wxTextCtrl* statusLog{nullptr};
    wxButton* sendButton{nullptr};
    wxButton* sendFileButton{nullptr};
    wxButton* pauseButton{nullptr};
    wxButton* resumeButton{nullptr};
    wxButton* quitButton{nullptr};

    std::vector<BVGUIServiceView> services;
    std::string selectedServiceName;

    void BuildLayout(void)
    {
        auto* rootSizer = new wxBoxSizer(wxVERTICAL);
        auto* bodySizer = new wxBoxSizer(wxHORIZONTAL);
        auto* serviceSizer = new wxBoxSizer(wxVERTICAL);
        auto* chatSizer = new wxBoxSizer(wxVERTICAL);
        auto* inputSizer = new wxBoxSizer(wxHORIZONTAL);
        auto* controlSizer = new wxBoxSizer(wxHORIZONTAL);

        serviceSizer->Add(new wxStaticText(this, wxID_ANY, "Available services"), 0, wxBOTTOM, 4);
        servicesList = new wxListBox(this, wxID_ANY);
        serviceSizer->Add(servicesList, 1, wxEXPAND | wxBOTTOM, 10);

        serviceSizer->Add(new wxStaticText(this, wxID_ANY, "Sessions"), 0, wxBOTTOM, 4);
        sessionsList = new wxListBox(this, wxID_ANY);
        serviceSizer->Add(sessionsList, 1, wxEXPAND);

        chatSizer->Add(new wxStaticText(this, wxID_ANY, "Conversation"), 0, wxBOTTOM, 4);
        chatLog = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize,
                                 wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH2);
        chatSizer->Add(chatLog, 1, wxEXPAND | wxBOTTOM, 8);

        messageInput = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize,
                                      wxTE_PROCESS_ENTER);
        sendButton = new wxButton(this, wxID_ANY, "Send");
        sendFileButton = new wxButton(this, wxID_ANY, "File...");
        inputSizer->Add(messageInput, 1, wxEXPAND | wxRIGHT, 6);
        inputSizer->Add(sendButton, 0, wxRIGHT, 6);
        inputSizer->Add(sendFileButton, 0);
        chatSizer->Add(inputSizer, 0, wxEXPAND | wxBOTTOM, 8);

        statusLog = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(-1, 100),
                                   wxTE_MULTILINE | wxTE_READONLY);
        chatSizer->Add(statusLog, 0, wxEXPAND);

        bodySizer->Add(serviceSizer, 0, wxEXPAND | wxALL, 10);
        bodySizer->Add(chatSizer, 1, wxEXPAND | wxTOP | wxRIGHT | wxBOTTOM, 10);

        pauseButton = new wxButton(this, wxID_ANY, "Pause discovery");
        resumeButton = new wxButton(this, wxID_ANY, "Resume discovery");
        quitButton = new wxButton(this, wxID_ANY, "Quit");
        controlSizer->Add(pauseButton, 0, wxRIGHT, 6);
        controlSizer->Add(resumeButton, 0, wxRIGHT, 6);
        controlSizer->AddStretchSpacer(1);
        controlSizer->Add(quitButton, 0);

        rootSizer->Add(bodySizer, 1, wxEXPAND);
        rootSizer->Add(controlSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);
        SetSizer(rootSizer);
    }

    void BindEvents(void)
    {
        servicesList->Bind(wxEVT_LISTBOX, [this](wxCommandEvent&)
        {
            const int selection = servicesList->GetSelection();
            if (selection == wxNOT_FOUND || static_cast<std::size_t>(selection) >= services.size())
            {
                selectedServiceName.clear();
                RefreshChatLog();
                return;
            }

            selectedServiceName = services[selection].serviceName;
            RefreshChatLog();
        });

        sendButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent&)
        {
            SendMessageFromInput();
        });

        messageInput->Bind(wxEVT_TEXT_ENTER, [this](wxCommandEvent&)
        {
            SendMessageFromInput();
        });

        sendFileButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent&)
        {
            SendFileFromDialog();
        });

        pauseButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent&)
        {
            app.PauseDiscovery();
            AppendStatus("Discovery pause requested.");
        });

        resumeButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent&)
        {
            app.ResumeDiscovery();
            AppendStatus("Discovery resume requested.");
        });

        quitButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent&)
        {
            Close();
        });

        Bind(wxEVT_CLOSE_WINDOW, [this](wxCloseEvent& event)
        {
            event.Veto();
            Hide();
            app.RequestShutdown();
            if (wxTheApp)
            {
                wxTheApp->ExitMainLoop();
            }
        });
    }

    void SendMessageFromInput(void)
    {
        if (selectedServiceName.empty())
        {
            AppendStatus("Choose a service before sending a message.");
            return;
        }

        const std::string text = messageInput->GetValue().ToStdString();
        if (text.empty())
        {
            return;
        }

        const BVStatus status = app.SendChatMessageToService(selectedServiceName, text);
        if (status == BVStatus::BVSTATUS_OK)
        {
            messageInput->Clear();
            RefreshChatLog();
        }
        else
        {
            AppendStatus("Could not send message to " + selectedServiceName + ".");
        }
    }

    void SendFileFromDialog(void)
    {
        if (selectedServiceName.empty())
        {
            AppendStatus("Choose a service before sending a file.");
            return;
        }

        wxFileDialog dialog(this, "Choose a file to send", wxEmptyString, wxEmptyString,
                            wxFileSelectorDefaultWildcardStr, wxFD_OPEN | wxFD_FILE_MUST_EXIST);
        if (dialog.ShowModal() != wxID_OK)
        {
            return;
        }

        const BVStatus status = app.SendFileToService(selectedServiceName,
                                                      std::filesystem::path(dialog.GetPath().ToStdString()));
        if (status == BVStatus::BVSTATUS_OK)
        {
            AppendStatus("File transfer started: " + dialog.GetFilename().ToStdString());
        }
        else
        {
            AppendStatus("Could not start file transfer to " + selectedServiceName + ".");
        }
    }

    void RefreshChatLog(void)
    {
        chatLog->Clear();
        if (selectedServiceName.empty())
        {
            return;
        }

        const auto messages = app.GetChatMessagesForGui(selectedServiceName);
        for (const auto& message : messages)
        {
            chatLog->AppendText("[" + message.sender + "] " + message.textData + "\n");
        }
    }

public:
    explicit BVLocalChatFrame(BVApp_GUIClient& _app) :
    wxFrame(nullptr, wxID_ANY, "LocalChat", wxDefaultPosition, wxSize(900, 600)),
    app(_app)
    {
        BuildLayout();
        BindEvents();
    }

    void RefreshAll(void)
    {
        services = app.GetServicesForGui();
        servicesList->Clear();
        int selectionToRestore = wxNOT_FOUND;

        for (std::size_t idx = 0; idx < services.size(); idx++)
        {
            const auto& service = services[idx];
            std::string label = service.serviceName;
            if (service.hasSession)
            {
                label += " (connected)";
            }
            servicesList->Append(label);
            if (service.serviceName == selectedServiceName)
            {
                selectionToRestore = static_cast<int>(idx);
            }
        }

        if (selectionToRestore != wxNOT_FOUND)
        {
            servicesList->SetSelection(selectionToRestore);
        }
        else if (!services.empty())
        {
            servicesList->SetSelection(0);
            selectedServiceName = services.front().serviceName;
        }
        else
        {
            selectedServiceName.clear();
        }

        sessionsList->Clear();
        for (const auto& sessionName : app.GetSessionNamesForGui())
        {
            sessionsList->Append(sessionName);
        }

        RefreshChatLog();
    }

    void AppendStatus(const std::string& message)
    {
        statusLog->AppendText(message + "\n");
    }
};

namespace
{
    class BVWxApp : public wxApp
    {
    public:
        bool OnInit(void) override
        {
            return true;
        }
    };
}

wxIMPLEMENT_APP_NO_MAIN(BVWxApp);

BVApp_GUIClient::BVApp_GUIClient(const BVServiceData _thisMachineServiceData,
                                 std::shared_ptr<threadsafe_queue<BVMessage>> _outMbx,
                                 std::shared_ptr<threadsafe_queue<BVMessage>> _inMbx,
                                 boost::asio::io_context& _ioContext) :
BVApp(_ioContext, _thisMachineServiceData),
BVComponent(_outMbx, _inMbx)
{
    RegisterCallback(BVEventType::BVEVENTTYPE_APP_PUBLISHED_SERVICE,
                     std::bind(&BVApp_GUIClient::HandlePublishedServices, this, std::placeholders::_1));
    RegisterCallback(BVEventType::BVEVENTTYPE_TERMINATE_ALL,
                     std::bind(&BVApp_GUIClient::OnShutdown, this, std::placeholders::_1));
    RegisterCallback(BVEventType::BVEVENTTYPE_APP_DISCOVERY_SERVICE_RESOLVED,
                     std::bind(&BVApp_GUIClient::HandleResolvedServices, this, std::placeholders::_1));
    RegisterCallback(BVEventType::BVEVENTTYPE_APP_DEREGISTERED_SERVICE,
                     std::bind(&BVApp_GUIClient::HandleServiceDeregistration, this, std::placeholders::_1));
    RegisterCallback(BVEventType::BVEVENTTYPE_APP_MESSAGE_INCOMING,
                     std::bind(&BVApp_GUIClient::HandleMessageIncoming, this, std::placeholders::_1));
    RegisterCallback(BVEventType::BVEVENTTYPE_APP_FILE_TRANSFER_BEGIN,
                     std::bind(&BVApp_GUIClient::HandleFileTransferBegin, this, std::placeholders::_1));
    RegisterCallback(BVEventType::BVEVENTTYPE_APP_FILE_TRANSFER_CHUNK_SENT,
                     std::bind(&BVApp_GUIClient::HandleFileChunkSent, this, std::placeholders::_1));
    RegisterCallback(BVEventType::BVEVENTTYPE_APP_FILE_TRANSFER_END,
                     std::bind(&BVApp_GUIClient::HandleFileTransferEnd, this, std::placeholders::_1));

    this->GetConnectionManager().SetMailboxGetterF(
        [this]() -> std::shared_ptr<threadsafe_queue<BVMessage>>
        {
            return this->GetInMailBox();
        }
    );
    this->GetConnectionManager().SetFileTransferFinishedCallback(
        [this](const std::string& fileName)
        {
            AppendGuiStatus("File sent: " + fileName);
        }
    );
}

void BVApp_GUIClient::Run(void)
{
    if (!wxEntryStart(wxArgc, wxArgv))
    {
        LogError("[BVApp_GUIClient]: wxEntryStart failed.");
        SetIsRunning(false);
        return;
    }

    if (!wxTheApp || !wxTheApp->CallOnInit())
    {
        LogError("[BVApp_GUIClient]: wxApp initialization failed.");
        wxEntryCleanup();
        SetIsRunning(false);
        return;
    }

    auto frame = std::make_unique<BVLocalChatFrame>(*this);
    {
        std::lock_guard<std::mutex> l(frameMutex);
        frame_p = frame.get();
    }
    frame->RefreshAll();
    frame->Show(true);

    wxTheApp->OnRun();

    {
        std::lock_guard<std::mutex> l(frameMutex);
        frame_p = nullptr;
    }
    frame.reset();
    wxTheApp->OnExit();
    wxEntryCleanup();
}

void BVApp_GUIClient::SetCommandLineArguments(int argc, char** argv)
{
    wxArgc = argc;
    wxArgv = argv;
}

BVStatus BVApp_GUIClient::HandlePublishedServices(std::unique_ptr<std::any> dp)
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

    std::vector<BVServiceBrowseInstance> toResolve;
    {
        std::lock_guard<std::mutex> l(this->serviceVectorMutex);
        for (auto& lElem : newServicesList)
        {
            if (std::find(this->serviceV.begin(), this->serviceV.end(), lElem) == this->serviceV.end())
            {
                const BVServiceData& thisMachineServiceData = GetThisMachineServiceData();
                if (lElem.serviceName == thisMachineServiceData.hostname)
                {
                    continue;
                }

                this->serviceV.push_back(lElem);
                toResolve.push_back(lElem);
                LogTrace("App, HandlePublishedServices: Added {} to serviceV", lElem.serviceName);
            }
        }
    }

    for (auto& lElem : toResolve)
    {
        SendMessage(BVMessage(
            BVEventType::BVEVENTTYPE_DISCOVERY_REQUEST_RESOLVE,
            std::make_unique<std::any>(std::make_any<BVServiceBrowseInstance>(lElem))));
        LogTrace("App: Sending request to Discovery to resolve {}", lElem.serviceName);
    }

    NotifyGui();
    return BVStatus::BVSTATUS_OK;
}

BVStatus BVApp_GUIClient::HandleResolvedServices(std::unique_ptr<std::any> dp)
{
    BVStatus status = BVStatus::BVSTATUS_OK;
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
        LogError("App: Bad cast in HandleResolvedServices! Error details: {}", e.what());
        return BVStatus::BVSTATUS_FATAL_ERROR;
    }

    const std::string hosttarget  = res->hosttarget;
    const std::string serviceName = res->serviceName;
    const int         port        = res->port;

    BVNode node = ResolveServiceToEndpoint(hosttarget, serviceName, port);
    if (node.serviceName == "ERROR")
    {
        ::free(res);
        return BVStatus::BVSTATUS_FATAL_ERROR;
    }

    auto _nodesM = this->GetConnectionManager().GetNodesM();
    auto it = _nodesM.find(serviceName);
    if (it == _nodesM.end())
    {
        GetConnectionManager().AddNodeToNodesM(serviceName, node);
        LogTrace("App: Added node representing service {} to nodesM", serviceName);
    }
    else
    {
        LogWarn("App: Node representing service {} already present in nodesM", serviceName);
        ::free(res);
        NotifyGui();
        return BVStatus::BVSTATUS_OK;
    }

    status = this->GetConnectionManager().InitiateSessionWithNode(node);
    if (status == BVStatus::BVSTATUS_FATAL_ERROR)
    {
        LogError("Couldn't Initiate Session with a node! {}:{} [{}]",
                 node.hostname, node.port, node.address.to_string());
    }

    ::free(res);
    NotifyGui();
    return status;
}

BVStatus BVApp_GUIClient::HandleServiceDeregistration(std::unique_ptr<std::any> dp)
{
    using BVServiceBrowseInstanceList = std::list<BVServiceBrowseInstance>;
    if (dp == nullptr)
    {
        LogError("App: No deregistered services received.");
        return BVStatus::BVSTATUS_FATAL_ERROR;
    }

    BVServiceBrowseInstanceList newServicesList;
    try
    {
        newServicesList = std::any_cast<BVServiceBrowseInstanceList>(*dp);
    }
    catch(const std::bad_any_cast& e)
    {
        LogError("App: Bad cast in BVEventType::BVEVENTTYPE_APP_DEREGISTERED_SERVICE callback.");
        return BVStatus::BVSTATUS_FATAL_ERROR;
    }

    {
        std::lock_guard<std::mutex> l(serviceVectorMutex);
        for (auto& lElem : newServicesList)
        {
            serviceV.erase(
                std::remove_if(serviceV.begin(),
                               serviceV.end(),
                               [&](const BVServiceBrowseInstance& s)
                               {
                                   return s.serviceName == lElem.serviceName;
                               }),
                serviceV.end()
            );
            GetConnectionManager().GetNodesM().erase(lElem.serviceName);
            this->GetConnectionManager().RemoveSession(lElem.serviceName);
        }
    }

    NotifyGui();
    return BVStatus::BVSTATUS_OK;
}

BVStatus BVApp_GUIClient::HandleMessageIncoming(std::unique_ptr<std::any> dp)
{
    if (dp == nullptr)
    {
        LogError("App: Error - HandleMessageIncoming, data pointer is null!");
        return BVStatus::BVSTATUS_FATAL_ERROR;
    }

    BVChatMessage res;
    try
    {
        res = std::any_cast<BVChatMessage>(*dp);
    }
    catch(const std::bad_any_cast& e)
    {
        LogError("[BVApp_GUIClient]: Bad cast in HandleMessageIncoming! Error details: {}", e.what());
        return BVStatus::BVSTATUS_FATAL_ERROR;
    }

    {
        std::lock_guard<std::mutex> l(chatLogsMapMutex);
        try
        {
            chatLogsM.at(res.sender).AddMessage(res);
        }
        catch(const std::out_of_range& e)
        {
            BVChatMessageLog log{res.sender, res};
            chatLogsM.emplace(res.sender, log);
        }
    }

    NotifyGui();
    return BVStatus::BVSTATUS_OK;
}

BVStatus BVApp_GUIClient::HandleFileTransferBegin(std::unique_ptr<std::any> dp)
{
    if (dp == nullptr)
    {
        LogError("[BVApp_GUIClient]: Error - HandleFileTransferBegin, data pointer is null!");
        return BVStatus::BVSTATUS_FATAL_ERROR;
    }

    BVTCPFileData res;
    try
    {
        res = std::any_cast<BVTCPFileData>(*dp);
    }
    catch(const std::bad_any_cast& e)
    {
        LogError("[BVApp_GUIClient]: Bad cast in HandleFileTransferBegin! Error details: {}", e.what());
        return BVStatus::BVSTATUS_FATAL_ERROR;
    }

    const uint32_t correlationKey = res.correlationKey;
    const std::vector<char> fdata = res.fdata;
    const std::string fdataStr{fdata.begin(), fdata.end()};
    const std::string serviceName = fdataStr.substr(0, fdataStr.find('|'));
    const std::string fname = fdataStr.substr(fdataStr.find("|") + 1);

    const std::filesystem::path rootdir = std::filesystem::current_path();
    try
    {
        const std::filesystem::path dirpath = rootdir / "data" / serviceName;
        const std::filesystem::path filepath = dirpath / fname;
        if (!std::filesystem::is_directory(dirpath))
        {
            std::filesystem::create_directories(dirpath);
        }
        if (!std::filesystem::is_regular_file(filepath))
        {
            std::ofstream incomingFile(filepath, std::ios::binary | std::ios::out);
        }
    }
    catch(const std::exception& e)
    {
        LogError("[BVApp_GUIClient]: Error while creating directory and/or file: {}", e.what());
        return BVStatus::BVSTATUS_NOK;
    }

    fileTransferData[correlationKey] = std::make_tuple(serviceName, fname);
    AppendGuiStatus("Receiving file from " + serviceName + ": " + fname);
    return BVStatus::BVSTATUS_OK;
}

BVStatus BVApp_GUIClient::HandleFileChunkSent(std::unique_ptr<std::any> dp)
{
    if (dp == nullptr)
    {
        LogError("[BVApp_GUIClient]: Error - HandleFileChunkSent, data pointer is null!");
        return BVStatus::BVSTATUS_FATAL_ERROR;
    }

    BVTCPFileData res;
    try
    {
        res = std::any_cast<BVTCPFileData>(*dp);
    }
    catch(const std::bad_any_cast& e)
    {
        LogError("[BVApp_GUIClient]: Bad cast in HandleFileChunkSent! Error details: {}", e.what());
        return BVStatus::BVSTATUS_FATAL_ERROR;
    }

    std::string serviceName;
    std::string fname;
    try
    {
        std::tuple<std::string, std::string> fdata_t = fileTransferData.at(res.correlationKey);
        serviceName = std::get<0>(fdata_t);
        fname = std::get<1>(fdata_t);
    }
    catch(const std::out_of_range& ex)
    {
        LogError("Couldn't find the correlated data for this file transfer!");
        return BVStatus::BVSTATUS_FATAL_ERROR;
    }

    const std::filesystem::path rootdir = std::filesystem::current_path();
    try
    {
        const std::filesystem::path filepath = rootdir / "data" / serviceName / fname;
        std::ofstream incomingFile(filepath, std::ios::binary | std::ios::app);
        if (!incomingFile)
        {
            LogError("Couldn't open an out stream for: {}", filepath.string());
            return BVStatus::BVSTATUS_FATAL_ERROR;
        }
        incomingFile.write(res.fdata.data(), static_cast<std::streamsize>(res.fdata.size()));
    }
    catch(const std::exception& e)
    {
        LogError("[BVApp_GUIClient]: Error while writing to file: {}", e.what());
        return BVStatus::BVSTATUS_NOK;
    }

    return BVStatus::BVSTATUS_OK;
}

BVStatus BVApp_GUIClient::HandleFileTransferEnd(std::unique_ptr<std::any> dp)
{
    if (dp == nullptr)
    {
        LogError("[BVApp_GUIClient]: Error - HandleFileTransferEnd, data pointer is null!");
        return BVStatus::BVSTATUS_FATAL_ERROR;
    }

    BVTCPFileData res;
    try
    {
        res = std::any_cast<BVTCPFileData>(*dp);
    }
    catch(const std::bad_any_cast& e)
    {
        LogError("[BVApp_GUIClient]: Bad cast in HandleFileTransferEnd! Error details: {}", e.what());
        return BVStatus::BVSTATUS_FATAL_ERROR;
    }

    std::string serviceName;
    std::string fname;
    try
    {
        std::tuple<std::string, std::string> fdata_t = fileTransferData.at(res.correlationKey);
        serviceName = std::get<0>(fdata_t);
        fname = std::get<1>(fdata_t);
    }
    catch(const std::out_of_range& ex)
    {
        LogError("Couldn't find the correlated data for this file transfer!");
        return BVStatus::BVSTATUS_FATAL_ERROR;
    }

    const std::filesystem::path rootdir = std::filesystem::current_path();
    try
    {
        const std::filesystem::path filepath = rootdir / "data" / serviceName / fname;
        std::ofstream incomingFile(filepath, std::ios::binary | std::ios::app);
        if (!incomingFile)
        {
            LogError("Couldn't open an out stream for: {}", filepath.string());
            return BVStatus::BVSTATUS_FATAL_ERROR;
        }
        incomingFile.write(res.fdata.data(), static_cast<std::streamsize>(res.fdata.size()));
    }
    catch(const std::exception& e)
    {
        LogError("[BVApp_GUIClient]: Error while writing to file: {}", e.what());
        return BVStatus::BVSTATUS_NOK;
    }

    fileTransferData.erase(res.correlationKey);
    AppendGuiStatus("File saved: " + fname);
    return BVStatus::BVSTATUS_OK;
}

BVStatus BVApp_GUIClient::PrintServices(void)
{
    return BVStatus::BVSTATUS_OK;
}

std::unique_ptr<BVTCPMessage<BVChatMessagePayload>> BVApp_GUIClient::ConstructChatMessageFromInput(
    const std::string& inputString)
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

std::vector<BVGUIServiceView> BVApp_GUIClient::GetServicesForGui(void)
{
    std::vector<BVGUIServiceView> result;
    std::lock_guard<std::mutex> l(this->serviceVectorMutex);
    for (const auto& service : this->serviceV)
    {
        result.push_back(BVGUIServiceView{
            service.serviceName,
            service.regType,
            service.replyDomain,
            this->GetConnectionManager().IsSessionAlreadyPresent(service.serviceName)
        });
    }
    return result;
}

std::vector<BVGUIChatMessageView> BVApp_GUIClient::GetChatMessagesForGui(const std::string& serviceName)
{
    std::vector<BVGUIChatMessageView> result;
    std::lock_guard<std::mutex> l(chatLogsMapMutex);
    try
    {
        for (const auto& message : chatLogsM.at(serviceName).logV)
        {
            result.push_back(BVGUIChatMessageView{message.textData, message.sender, message.timestamp});
        }
    }
    catch(const std::out_of_range& e)
    {
    }
    return result;
}

std::vector<std::string> BVApp_GUIClient::GetSessionNamesForGui(void)
{
    std::vector<std::string> result;
    for (const auto& [serviceName, node] : this->GetConnectionManager().GetNodesM())
    {
        if (this->GetConnectionManager().IsSessionAlreadyPresent(serviceName))
        {
            result.push_back(serviceName);
        }
    }
    return result;
}

BVStatus BVApp_GUIClient::SendChatMessageToService(const std::string& serviceName, const std::string& text)
{
    SessionID sid;
    BVStatus sidStatus = GetConnectionManager().GetSessionIDFromServiceName(serviceName, sid);
    if (sidStatus != BVStatus::BVSTATUS_OK)
    {
        LogError("Couldn't get sid from {}", serviceName);
        return BVStatus::BVSTATUS_NOK;
    }

    std::unique_ptr<BVTCPMessage<BVChatMessagePayload>> chatMsg = ConstructChatMessageFromInput(text);
    const uint64_t timestamp = chatMsg->header.timestamp;
    BVStatus sentStatus = GetConnectionManager().SendDataToNode(std::move(chatMsg), sid);
    if (sentStatus != BVStatus::BVSTATUS_OK)
    {
        return sentStatus;
    }

    {
        std::lock_guard<std::mutex> l(chatLogsMapMutex);
        try
        {
            chatLogsM.at(serviceName).AddMessage(BVChatMessage(text, timestamp, GetThisMachineServiceData().hostname));
        }
        catch(const std::out_of_range& e)
        {
            BVChatMessageLog log{serviceName, BVChatMessage(text, timestamp, GetThisMachineServiceData().hostname)};
            chatLogsM.emplace(serviceName, log);
        }
    }

    NotifyGui();
    return BVStatus::BVSTATUS_OK;
}

BVStatus BVApp_GUIClient::SendFileToService(const std::string& serviceName, const std::filesystem::path& filePath)
{
    if (!std::filesystem::exists(filePath))
    {
        LogError("[BVApp_GUIClient]: File {} does not exist!", filePath.string());
        return BVStatus::BVSTATUS_NOK;
    }

    SessionID sid;
    BVStatus sidStatus = GetConnectionManager().GetSessionIDFromServiceName(serviceName, sid);
    if (sidStatus != BVStatus::BVSTATUS_OK)
    {
        LogError("Couldn't get sid from {}", serviceName);
        return BVStatus::BVSTATUS_NOK;
    }

    std::filesystem::path mutableFilePath = filePath;
    return GetConnectionManager().InitiateFileTransferWithSession(sid, mutableFilePath);
}

void BVApp_GUIClient::PauseDiscovery(void)
{
    SendMessage(BVMessage(BVEventType::BVEVENTTYPE_DISCOVERY_REQUEST_PAUSE, nullptr));
}

void BVApp_GUIClient::ResumeDiscovery(void)
{
    SendMessage(BVMessage(BVEventType::BVEVENTTYPE_DISCOVERY_REQUEST_RESUME, nullptr));
}

void BVApp_GUIClient::RequestShutdown(void)
{
    if (!GetIsRunning())
    {
        return;
    }
    SendMessage(BVMessage(BVEventType::BVEVENTTYPE_TERMINATE_ALL, nullptr));
    SetIsRunning(false);
}

BVStatus BVApp_GUIClient::OnStart(std::unique_ptr<std::any>)
{
    return BVStatus::BVSTATUS_OK;
}

BVStatus BVApp_GUIClient::OnResume(std::unique_ptr<std::any>)
{
    return BVStatus::BVSTATUS_OK;
}

BVStatus BVApp_GUIClient::OnShutdown(std::unique_ptr<std::any>)
{
    StopIOContext();
    SetIsRunning(false);
    if (wxTheApp)
    {
        wxTheApp->CallAfter([]()
        {
            if (wxTheApp)
            {
                wxTheApp->ExitMainLoop();
            }
        });
    }
    LogTrace("App: Shutting down...");
    return BVStatus::BVSTATUS_OK;
}

BVStatus BVApp_GUIClient::OnRestart(std::unique_ptr<std::any>)
{
    return BVStatus::BVSTATUS_OK;
}

BVStatus BVApp_GUIClient::OnPause(std::unique_ptr<std::any>)
{
    return BVStatus::BVSTATUS_OK;
}

BVNode BVApp_GUIClient::ResolveServiceToEndpoint(const std::string& hosttarget, const std::string& serviceName, const int port)
{
    LogTrace("BVApp_GUIClient::ResolveServiceToEndpoint: Resolving host {} on port: {}", hosttarget, port);
    BVNode nodeData{};
    boost::system::error_code ec;
    boost::asio::ip::tcp::resolver resolver{GetIoContext()};

    boost::asio::ip::tcp::resolver::results_type results;
    for (int attempt = 0; attempt < 5; attempt++)
    {
        ec.clear();
        results = resolver.resolve(hosttarget, std::to_string(port), ec);
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

    boost::asio::ip::tcp::endpoint endpoint = results.begin()->endpoint();
    LogTrace("Successfuly resolved {} to {}", serviceName, endpoint.address().to_string());
    nodeData.ep = endpoint;
    nodeData.address = endpoint.address();
    nodeData.hostname = hosttarget;
    nodeData.serviceName = serviceName;
    nodeData.results = results;
    nodeData.port = port;
    return nodeData;
}

void BVApp_GUIClient::NotifyGui(void)
{
    std::lock_guard<std::mutex> l(frameMutex);
    if (wxTheApp && frame_p)
    {
        wxTheApp->CallAfter([this]()
        {
            std::lock_guard<std::mutex> innerLock(frameMutex);
            if (frame_p)
            {
                frame_p->RefreshAll();
            }
        });
    }
}

void BVApp_GUIClient::AppendGuiStatus(const std::string& message)
{
    std::lock_guard<std::mutex> l(frameMutex);
    if (wxTheApp && frame_p)
    {
        wxTheApp->CallAfter([this, message]()
        {
            std::lock_guard<std::mutex> innerLock(frameMutex);
            if (frame_p)
            {
                frame_p->AppendStatus(message);
            }
        });
    }
}
