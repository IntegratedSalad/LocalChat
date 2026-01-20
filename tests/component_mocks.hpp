#include "BV.hpp"
#include "BVApp.hpp"
#include "BVComponent.hpp"
#include "BVBroker.hpp"
#include "BVDiscovery.hpp"
#include "BVService.hpp"
#include "const.h"
#include "linked_list.h"
#include "stdio.h"

class MockDiscovery : public BVDiscovery, 
                      public BVComponent
{
private:
    bool isConnectionContextAlive;

    void CreateConnectionContext(void) override;
    void Setup(void) override;
    void run(void) override;

    boost::asio::steady_timer discoveryTimer; // regular timer
    boost::asio::io_context& ioContext;

public:
    MockDiscovery(const BVServiceHostData _hostData,
                  std::mutex& _discoveryQueueMutex,
                  boost::asio::io_context& _ioContext,
                  std::shared_ptr<std::queue<BVServiceBrowseInstance>> _discoveryQueue,
                  std::condition_variable& _discoveryQueueCV,
                  bool& _isDiscoveryQueueReady);

    ~MockDiscovery() override;

    bool GetIsconnectionContextAlive(void)
    {
        return isConnectionContextAlive;
    }

    // Define callbacks for other events

    BVStatus OnStart(std::unique_ptr<std::any>) override;
    BVStatus OnShutdown(std::unique_ptr<std::any>) override;
    BVStatus OnRestart(std::unique_ptr<std::any>) override;
};

class MockService : public BVService,
                    public BVComponent
{
private:

public:
    MockService() :
    BVService(std::string("mockhost"), std::string(".local"), PORT)
    {}

    ~MockService() 
    {}

    BVStatus Register(void) override;

    BVStatus OnStart(std::unique_ptr<std::any>) override;
    BVStatus OnShutdown(std::unique_ptr<std::any>) override;
    BVStatus OnRestart(std::unique_ptr<std::any>) override;
};

class MockApp : public BVApp,
                public BVComponent
{
private:

public:
    MockApp(std::shared_ptr<std::queue<BVServiceBrowseInstance>> _discoveryQueue,
            std::mutex& _discoveryQueueMutex,
            std::condition_variable& _discoveryQueueCV,
            bool& _isDiscoveryQueueReady) :
    BVApp(_discoveryQueue,
            _discoveryQueueMutex,
            _discoveryQueueCV,
            _isDiscoveryQueueReady)
    {
    }

    ~MockApp() override
    {}

    void Init(void) override;
    void Run(void) override;
    void Quit(void) override;

    void HandleServicesDiscoveredUpdateEvent(void) override;
    void HandleUserKeyboardInput(void) override;

    BVStatus OnStart(std::unique_ptr<std::any>) override;
    BVStatus OnShutdown(std::unique_ptr<std::any>) override;
    BVStatus OnRestart(std::unique_ptr<std::any>) override;
};