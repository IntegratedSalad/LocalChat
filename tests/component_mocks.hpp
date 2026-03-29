#include "BV.hpp"
#include "BVApp.hpp"
#include "BVComponent.hpp"
#include "BVBroker.hpp"
#include "BVDiscovery.hpp"
#include "BVService.hpp"
#include "const.h"
#include "linked_list.h"
#include "stdio.h"
#include "spdlog/spdlog.h"
#include "BVLoggable.hpp"
#include <boost/asio.hpp>
#include <condition_variable>

class MockDiscovery : public BVDiscovery, 
                      public BVComponent,
                      public BVLoggable
{
private:
    bool isConnectionContextAlive;

    std::atomic<bool> isPaused = false;
    std::condition_variable pausedCv;
    std::mutex pauseMutex;
    int sleepMs = 1000;

    void CreateConnectionContext(void) override;
    void Setup(void) override;
    void Browse(void) override;

    void Pause(void)
    {
        std::unique_lock<std::mutex> lk(pauseMutex);
        pausedCv.wait(lk, [this]{return isPaused==false;});       
    }

    // maybe iocontext for simulating Bonjour?
    // OR just create another subclass!
    // TODO: iocontext in MockDiscoveryBonjour
    // IF NEEDED

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

    int GetSleepMs(void)
    {
        return this->sleepMs;
    }
    void SetSleepMs(const int _ms)
    {
        this->sleepMs = _ms;
    }

    // Define callbacks for other events
    BVStatus OnStart(std::unique_ptr<std::any>) override;
    BVStatus OnPause(std::unique_ptr<std::any>) override;
    BVStatus OnResume(std::unique_ptr<std::any>) override;
    BVStatus OnRestart(std::unique_ptr<std::any>) override;
    BVStatus OnShutdown(std::unique_ptr<std::any>) override;
};

class MockService : public BVService,
                    public BVComponent,
                    public BVLoggable
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
    BVStatus OnResume(std::unique_ptr<std::any>) override;
    BVStatus OnStart(std::unique_ptr<std::any>) override;
    BVStatus OnShutdown(std::unique_ptr<std::any>) override;
    BVStatus OnRestart(std::unique_ptr<std::any>) override;
};

class MockApp : public BVApp,
                public BVComponent,
                public BVLoggable
{
private:
    using TaskFunction = std::function<void(void)>;
    // This is for delay simulation 
    boost::asio::steady_timer announceTimer;
    boost::asio::steady_timer pauseDiscoveryTimer;
    // ...
    boost::asio::io_context& ioContext;

    // task queue - what will App do while running
    // (simulate user behavior)
    std::queue<TaskFunction> tasks_q;

    int taskSleepMs = 1000;

public:
    MockApp(std::shared_ptr<threadsafe_queue<BVMessage>> _outMbx,
            std::shared_ptr<threadsafe_queue<BVMessage>> _inMbx,
            boost::asio::io_context& _ioContext);

    ~MockApp() override
    {}
    
    // Constant work - put on the worker_thread
    void Run(void) override;

    /* Tasks
       Tasks are simulations of user input.
    */  

    // Announce (log) registered services in serviceV
    void TaskAnnounce(void);
    void TaskPauseDiscovery(void);
    // Starting means starting the worker AND mailbox thread.
    void TaskStartDiscovery(void);
    // Resuming means ...
    void TaskResumeDiscovery(void);
    void TaskQuit(void);
    void TaskSleep(void);
    // void 

    void SubmitTask(TaskFunction f)
    {
        this->tasks_q.push(f);
    }

    int GetTaskSleepMs(void)
    {
        return this->taskSleepMs;
    }
    void SetTaskSleepMs(const int _ms)
    {
        this->taskSleepMs = _ms;
    }    
    
    BVStatus HandlePublishedServices(std::unique_ptr<std::any>) override;
    // void HandleServicesDiscoveredUpdateEvent(void) override;
    // void HandleUserKeyboardInput(void) override;

    BVStatus OnStart(std::unique_ptr<std::any>) override;
    BVStatus OnResume(std::unique_ptr<std::any>) override;
    BVStatus OnShutdown(std::unique_ptr<std::any>) override;
    BVStatus OnRestart(std::unique_ptr<std::any>) override;
    BVStatus OnPause(std::unique_ptr<std::any>) override;
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
    void StartAnnouncingHeartbeat(void);
    void Beat(void);
    void TryJoinWorkerThread(void)
    {
        this->worker_thread.join();
    }

    int GetHid(void) const
    {
        return this->hid;
    }

    BVStatus OnStart(std::unique_ptr<std::any>) override;
    BVStatus OnShutdown(std::unique_ptr<std::any>) override;
    BVStatus OnResume(std::unique_ptr<std::any>) override;
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
    BVStatus OnResume(std::unique_ptr<std::any>) override;
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
    BVStatus OnResume(std::unique_ptr<std::any>) override;
    BVStatus OnRestart(std::unique_ptr<std::any>) override;
    BVStatus OnPause(std::unique_ptr<std::any>) override;
};

// Listens for BVEVENTTYPE_TEST_APP_ANNOUNCE_SERVICES
// class TCComponentCommunication : public BVComponent
// {
// private:
//     bool ack = false;
// public:
//     TCComponent(std::shared_ptr<threadsafe_queue<BVMessage>> _outMbx,
//                 std::shared_ptr<threadsafe_queue<BVMessage>> _inMbx);

//     ~TCComponent() override
//     {}

//     BVStatus CheckAck(void)
//     {
//         return ack ? BVStatus::BVSTATUS_OK : BVStatus::BVSTATUS_NOK;
//     }

//     BVStatus ResetAck(void)
//     {
//         this->ack = false;
//     }

//     BVStatus OnStart(std::unique_ptr<std::any>) override;
//     BVStatus OnShutdown(std::unique_ptr<std::any>) override;
//     BVStatus OnRestart(std::unique_ptr<std::any>) override;
//     BVStatus OnPause(std::unique_ptr<std::any>) override;
// };
