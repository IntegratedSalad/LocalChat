#include "BV.hpp"
#include "BVApp.hpp"
#include "BVComponent.hpp"
#include "BVBroker.hpp"
#include "BVDiscovery.hpp"
#include "BVService.hpp"
#include "const.h"
#include "linked_list.h"
#include "stdio.h"
#include <boost/asio.hpp>

class MockDiscovery : public BVDiscovery, 
                      public BVComponent
{
private:
    bool isConnectionContextAlive;

    void CreateConnectionContext(void) override;
    void Setup(void) override;
    void run(void) override;

public:
    MockDiscovery(const BVServiceHostData _hostData,
                  std::shared_ptr<threadsafe_queue<BVMessage>> _outMbx,
                  std::shared_ptr<threadsafe_queue<BVMessage>> _inMbx);

    ~MockDiscovery() override;

    bool GetIsconnectionContextAlive(void)
    {
        return isConnectionContextAlive;
    }

    void RunOnce(void);
    void RunContinuously(void);
    void RunNTimes(const int n);
    void RunNTimesWithKElements(const int n, const int k);

    // Define callbacks for other events

    BVStatus OnStart(std::unique_ptr<std::any>) override;
    BVStatus OnPause(std::unique_ptr<std::any>) override;
    BVStatus OnRestart(std::unique_ptr<std::any>) override;
    BVStatus OnShutdown(std::unique_ptr<std::any>) override;
};

class MockService : public BVService,
                    public BVComponent
{
private:

public:
    MockService(std::shared_ptr<threadsafe_queue<BVMessage>> _outMbx,
                std::shared_ptr<threadsafe_queue<BVMessage>> _inMbx) :
    BVService(std::string("mockhost"), std::string(".local"), PORT),
    BVComponent(_outMbx, _inMbx)
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
    MockApp(std::shared_ptr<threadsafe_queue<BVMessage>> _outMbx,
            std::shared_ptr<threadsafe_queue<BVMessage>> _inMbx) :
    BVComponent(_outMbx, _inMbx)
    {
    }

    ~MockApp() override
    {}

    void Init(void) override;
    void Run(void) override;
    void Quit(void) override;

    // void HandleServicesDiscoveredUpdateEvent(void) override;
    // void HandleUserKeyboardInput(void) override;

    BVStatus OnStart(std::unique_ptr<std::any>) override;
    BVStatus OnShutdown(std::unique_ptr<std::any>) override;
    BVStatus OnRestart(std::unique_ptr<std::any>) override;
};

// General, abstract Component implementation.
// Every x ms it announces that it lives on a separate thread.
// (Producer)
class TestHeartbeatComponent : public BVComponent
{
private:
    int hid;
    size_t heartbeatMs;
    std::vector<BVEventType> eventTypesOfInterest;
    std::thread worker_thread;

    boost::asio::steady_timer timer;
    boost::asio::io_context& ioContext;

    std::atomic<bool> working{true};

public:
    TestHeartbeatComponent(std::vector<BVEventType> _eventTypesOfInterest,
                           std::shared_ptr<threadsafe_queue<BVMessage>> _outMbx,
                           std::shared_ptr<threadsafe_queue<BVMessage>> _inMbx,
                           boost::asio::io_context& _iocontext,
                           const int _hid, // produces heartbeat for this id
                           const size_t _heartbeatMs);

    ~TestHeartbeatComponent() override
    {}

    void Setup(void);
    void LaunchWorkerThread(void);
    void StartAnnouncingHeartbeat(void); // TODO: Announce with id of the TestHeartbeatComponent
    void Beat(void);
    void JoinWorkerThread(void)
    {
        this->worker_thread.join();
    }

    int GetHid(void) const
    {
        return this->hid;
    }

    BVStatus OnStart(std::unique_ptr<std::any>) override;
    BVStatus OnShutdown(std::unique_ptr<std::any>) override;
    BVStatus OnRestart(std::unique_ptr<std::any>) override;
    BVStatus OnPause(std::unique_ptr<std::any>) override;
};

// Listener to BVEVENTTYPE_TEST_HEARTBEAT
// (Consumer)
// Its role is to send ACK/NACK upon receiving BVEVENTTYPE_TEST_HEARTBEAT
// with an HID corresponding to the one, that TestHeartbeatListenerComponent is listening to.
class TestHeartbeatListenerComponent : public BVComponent
{
private:
    int hid; // listens for hearbeat with this hid

public:
    TestHeartbeatListenerComponent(std::shared_ptr<threadsafe_queue<BVMessage>> _outMbx,
                                   std::shared_ptr<threadsafe_queue<BVMessage>> _inMbx,
                                   const int _hid);

    ~TestHeartbeatListenerComponent() override
    {}

    void Setup(void);

    BVStatus OnStart(std::unique_ptr<std::any>) override;
    BVStatus OnShutdown(std::unique_ptr<std::any>) override;
    BVStatus OnRestart(std::unique_ptr<std::any>) override;
    BVStatus OnPause(std::unique_ptr<std::any>) override;
};

class TCComponent : public BVComponent
{
private:
    bool ack = false;
public:
    TCComponent(std::shared_ptr<threadsafe_queue<BVMessage>> _outMbx,
                std::shared_ptr<threadsafe_queue<BVMessage>> _inMbx);

    ~TCComponent() override
    {}

    BVStatus CheckAck(void)
    {
        return ack ? BVStatus::BVSTATUS_OK : BVStatus::BVSTATUS_NOK;
    }

    BVStatus ResetAck(void)
    {
        this->ack = false;
    }

    BVStatus OnStart(std::unique_ptr<std::any>) override;
    BVStatus OnShutdown(std::unique_ptr<std::any>) override;
    BVStatus OnRestart(std::unique_ptr<std::any>) override;
    BVStatus OnPause(std::unique_ptr<std::any>) override;
};