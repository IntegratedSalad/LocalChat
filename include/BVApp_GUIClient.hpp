#pragma once

#include <boost/asio.hpp>
#include <filesystem>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include "BVApp.hpp"
#include "BVComponent.hpp"
#include "BVLoggable.hpp"
#include "api_common.h"

class BVLocalChatFrame;

struct BVGUIServiceView
{
    std::string serviceName;
    std::string regType;
    std::string replyDomain;
    bool hasSession{false};
};

struct BVGUIChatMessageView
{
    std::string textData;
    std::string sender;
    uint64_t timestamp{0};
};

class BVApp_GUIClient : public BVApp,
                        public BVComponent,
                        public BVLoggable
{
private:
    BVLocalChatFrame* frame_p{nullptr};
    std::mutex frameMutex;
    int wxArgc{0};
    char** wxArgv{nullptr};

    void NotifyGui(void);
    void AppendGuiStatus(const std::string& message);

protected:
    BVNode ResolveServiceToEndpoint(const std::string& hosttarget, const std::string& serviceName, const int port) override;

public:
    BVApp_GUIClient(const BVServiceData _thisMachineServiceData,
                    std::shared_ptr<threadsafe_queue<BVMessage>> _outMbx,
                    std::shared_ptr<threadsafe_queue<BVMessage>> _inMbx,
                    boost::asio::io_context& _ioContext);

    void Run(void) override;
    void SetCommandLineArguments(int argc, char** argv);

    // -------------------------------------------------------
    BVStatus HandlePublishedServices(std::unique_ptr<std::any> dp) override;
    BVStatus HandleResolvedServices(std::unique_ptr<std::any> dp) override;
    BVStatus HandleServiceDeregistration(std::unique_ptr<std::any>) override;
    BVStatus HandleMessageIncoming(std::unique_ptr<std::any>) override;
    BVStatus HandleFileTransferBegin(std::unique_ptr<std::any>) override;
    BVStatus HandleFileChunkSent(std::unique_ptr<std::any>) override;
    BVStatus HandleFileTransferEnd(std::unique_ptr<std::any>) override;
    // -------------------------------------------------------

    BVStatus PrintServices(void);
    std::unique_ptr<BVTCPMessage<BVChatMessagePayload>> ConstructChatMessageFromInput(
        const std::string& inputString);

    std::vector<BVGUIServiceView> GetServicesForGui(void);
    std::vector<BVGUIChatMessageView> GetChatMessagesForGui(const std::string& serviceName);
    std::vector<std::string> GetSessionNamesForGui(void);

    BVStatus SendChatMessageToService(const std::string& serviceName, const std::string& text);
    BVStatus SendFileToService(const std::string& serviceName, const std::filesystem::path& filePath);
    void PauseDiscovery(void);
    void ResumeDiscovery(void);
    void RequestShutdown(void);

    // -------------------------------------------------------
    BVStatus OnStart(std::unique_ptr<std::any>) override;
    BVStatus OnResume(std::unique_ptr<std::any>) override;
    BVStatus OnShutdown(std::unique_ptr<std::any>) override;
    BVStatus OnRestart(std::unique_ptr<std::any>) override;
    BVStatus OnPause(std::unique_ptr<std::any>) override;
    // -------------------------------------------------------

    ~BVApp_GUIClient() override = default;
};
