#include "BV.hpp"
#include "BVComponent.hpp"
#include "BVBroker.hpp"
#include "BVDiscovery.hpp"
#include "BVService.hpp"
#include "const.h"

// Mock interface?

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

    BVStatus OnStart(void) override;
    BVStatus OnShutdown(void) override;
    BVStatus OnRestart(void) override;
};

class MockService : public BVService,
                    public BVComponent
{
private:

public:
    MockService() :
    BVService(std::string("mockhost"), std::string(".local"), PORT)
    {}

    ~MockService() override 
    {}

    BVStatus Register(void) override;

    BVStatus OnStart(void) override;
    BVStatus OnShutdown(void) override;
    BVStatus OnRestart(void) override;
};