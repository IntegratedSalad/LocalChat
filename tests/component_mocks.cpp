#include "component_mocks.hpp"
#include <any>

MockDiscovery::MockDiscovery(const BVServiceHostData _hostData,
                             std::shared_ptr<threadsafe_queue<BVMessage>> _outMbx,
                             std::shared_ptr<threadsafe_queue<BVMessage>> _inMbx) :
    BVDiscovery(_hostData),
    BVComponent(_outMbx, _inMbx)
{
    // // RegisterCallbacks for interesting events to Discovery
    // RegisterCallback(BVEventType::..., 
    //     [this](std::unique_ptr<std::any>) -> BVStatus {
    //         this->SetIsBrowsingActive(false);
    //         return BVStatus::BVSTATUS_OK;
    // }); 

    RegisterCallback(BVEventType::BVEVENTTYPE_DISCOVERY_REQUEST_START,
                     std::bind(&MockDiscovery::OnStart, this, std::placeholders::_1));

    RegisterCallback(BVEventType::BVEVENTTYPE_DISCOVERY_REQUEST_PAUSE, 
                     std::bind(&MockDiscovery::OnPause, this, std::placeholders::_1));

    RegisterCallback(BVEventType::BVEVENTTYPE_TERMINATE_ALL,
                     std::bind(&MockDiscovery::OnShutdown, this, std::placeholders::_1));

    RegisterCallback(BVEventType::BVEVENTTYPE_DISCOVERY_REQUEST_SHUTDOWN,
                     std::bind(&MockDiscovery::OnShutdown, this, std::placeholders::_1));

    RegisterCallback(BVEventType::BVEVENTTYPE_DISCOVERY_REQUEST_RESTART,
                     std::bind(&MockDiscovery::OnRestart, this, std::placeholders::_1));

    RegisterCallback(BVEventType::BVEVENTTYPE_TEST_ECHO,
        [this](std::unique_ptr<std::any> dp) -> BVStatus {

            if (dp == nullptr)
            {
                std::cerr << "Bad cast in BVEventType::BVEVENTTYPE_TEST_ECHO callback. " <<
                             "No data has been passed into the callback!" << std::endl;
                return BVStatus::BVSTATUS_FATAL_ERROR;
            }
            std::string s;
            try
            {
                s = std::any_cast<std::string>(*dp);
            }
            catch(const std::bad_any_cast& e)
            {
                std::cerr << "Bad cast in BVEventType::BVEVENTTYPE_TEST_ECHO callback. " 
                          << e.what() << std::endl;
                return BVStatus::BVSTATUS_FATAL_ERROR;
            }
            std::cout << "Echo! Event: BVEVENTTYPE_TEST_ECHO, payload from callback: " 
                      << s << std::endl;
            SendMessage(BVMessage(
                    BVEventType::BVEVENTTYPE_TEST_ECHO, 
                        std::make_unique<std::any>(
                            std::make_any<std::string>(s))));
            return BVStatus::BVSTATUS_OK;
    });
    Setup();
}

MockDiscovery::~MockDiscovery()
{
    this->isConnectionContextAlive = false;
}

void MockDiscovery::CreateConnectionContext(void)
{
    this->isConnectionContextAlive = true;
}

void MockDiscovery::Setup(void)
{
    // TODO: this maybe has to be tied with the creation and start of the worker thread
    this->CreateConnectionContext();
}

void MockDiscovery::RunOnce(void) // this is run in the main worker thread
{
    using BVServiceBrowseInstanceList = std::list<BVServiceBrowseInstance>;
    this->SetIsBrowsingActive(true);
    LinkedListElement_str* lle1_p = LinkedListElement_str_Constructor((char*)"TESTSERVICE1", NULL);
    LinkedList_str_AddElement(this->GetLinkedList_p(), lle1_p);
    BVServiceBrowseInstanceList browseInstanceList = ReturnListFromBrowseResults();
    SendMessage(BVMessage(
                    BVEventType::BVEVENTTYPE_APP_PUBLISHED_SERVICE, 
                        std::make_unique<std::any>(std::make_any<BVServiceBrowseInstanceList>(browseInstanceList))));
    LinkedList_str_ClearList(this->GetLinkedList_p());

    // Should we just in the BVMessage's body of event type BVEVENTTYPE_APP_PUBLISHED_SERVICE pass
    // a newly created std::list from C linked list?

    // This whole logic is used to sync the queue...
    // std::unique_lock lk(this->GetDiscoveryQueueMutex());
    // this->GetDiscoveryQueueCV().wait(lk, [this]{return !this->GetIsDiscoveryQueueReady();});

    // this->PushBrowsedServicesToQueue(); // critical section

    // this->SetIsDiscoveryQueueReady(true);
    // lk.unlock();
    // this->GetDiscoveryQueueCV().notify_one();

    // // Clear list after appending to queue.
    // LinkedList_str_ClearList(this->GetLinkedList_p());
}

// Send n Messages with lists of size==1
void MockDiscovery::RunNTimes(const int n) // this is run in the main worker thread
{
    using BVServiceBrowseInstanceList = std::list<BVServiceBrowseInstance>;
    this->SetIsBrowsingActive(true);
    for (int i = 0; i < n; i++)
    {
        std::string s("TESTSERVICE" + std::to_string(i));
        LinkedListElement_str* lle1_p = LinkedListElement_str_Constructor((char*)s.c_str(), NULL);
        LinkedList_str_AddElement(this->GetLinkedList_p(), lle1_p);
        BVServiceBrowseInstanceList browseInstanceList = ReturnListFromBrowseResults();
        SendMessage(BVMessage(
                        BVEventType::BVEVENTTYPE_APP_PUBLISHED_SERVICE, 
                            std::make_unique<std::any>(std::make_any<BVServiceBrowseInstanceList>(browseInstanceList))));
        LinkedList_str_ClearList(this->GetLinkedList_p());
    }
}

// Send n messages with lists of size==k
void MockDiscovery::RunNTimesWithKElements(const int n, const int k) // this is run in the main worker thread
{
    using BVServiceBrowseInstanceList = std::list<BVServiceBrowseInstance>;
    this->SetIsBrowsingActive(true);
    for (int i = 0; i < n; i++)
    {
        BVServiceBrowseInstanceList browseInstanceList;
        for (int j = 0; j < k; j++)
        {
            std::string s("TESTSERVICE" + std::to_string(j));
            LinkedListElement_str* lle1_p = LinkedListElement_str_Constructor((char*)s.c_str(), NULL);
            LinkedList_str_AddElement(this->GetLinkedList_p(), lle1_p);
        }
        browseInstanceList = ReturnListFromBrowseResults();
        SendMessage(BVMessage(
                        BVEventType::BVEVENTTYPE_APP_PUBLISHED_SERVICE, 
                            std::make_unique<std::any>(std::make_any<BVServiceBrowseInstanceList>(browseInstanceList))));
        LinkedList_str_ClearList(this->GetLinkedList_p());
    }
}

// TODO: Get rid of it.
// update component_test_Discovery_Basic
void MockDiscovery::RunContinuously(void) // this is run in the main worker thread
{
    this->SetIsBrowsingActive(true);
    run();
}

void MockDiscovery::run(void)
{
    using BVServiceBrowseInstanceList = std::list<BVServiceBrowseInstance>;

    this->SetIsBrowsingActive(true);
    int serviceNum = 0;

    while (this->GetIsBrowsingActive())
    {
        // wait 3 s - simulate waiting for daemon response
        // TODO: maybe random interval 1-5s?
        std::this_thread::sleep_for(std::chrono::seconds(1));

        // replace with std::to_string
        std::string s("TESTSERVICE" + std::to_string(serviceNum));
        LinkedListElement_str* llen_p = LinkedListElement_str_Constructor((char*)s.c_str(), NULL);
        LinkedList_str_AddElement(this->GetLinkedList_p(), llen_p);
        serviceNum++;
        {
            // Add second element
            std::string s("TESTSERVICE" + std::to_string(serviceNum));
            LinkedListElement_str* llen_p = LinkedListElement_str_Constructor((char*)s.c_str(), NULL);
            LinkedList_str_AddElement(this->GetLinkedList_p(), llen_p);
            serviceNum++;
        }
        // PushBrowsedServicesToQueue(); // deprecated!
        BVServiceBrowseInstanceList browseInstanceList = ReturnListFromBrowseResults();
        SendMessage(BVMessage(
                        BVEventType::BVEVENTTYPE_APP_PUBLISHED_SERVICE, 
                            std::make_unique<std::any>(std::make_any<BVServiceBrowseInstanceList>(browseInstanceList))));
        LinkedList_str_ClearList(this->GetLinkedList_p());
    }
}

// TODO: Add helper function to populate more fields, other than serviceName

BVStatus MockDiscovery::OnStart(std::unique_ptr<std::any> dp) // test passing dp
{
    // In real implementation starting probably means putting worker thread to work.
    if (this->GetIsBrowsingActive()) // cannot start twice
    {
        return BVStatus::BVSTATUS_NOK;
    }
    Setup();
    // I think that in real implementation this should start the worker thread.
    return BVStatus::BVSTATUS_OK;
}

BVStatus MockDiscovery::OnPause(std::unique_ptr<std::any> dp)
{
    this->isConnectionContextAlive = false;
    this->SetIsBrowsingActive(false);

    // TODO: if this is mailbox thread, should we join worker thread here?
    return BVStatus::BVSTATUS_OK;
}

BVStatus MockDiscovery::OnShutdown(std::unique_ptr<std::any>)
{
    // TODO: Think through what the MockDiscovery implementation will be doing to mock DNS-SD functionality
    // and write a queue push procedure

    // Remember that this is still mailbox thread if it is called from within mailbox thread!
    this->isConnectionContextAlive = false;
    this->SetIsBrowsingActive(false);
    // Complete shutdown - we don't want to listen to mail anymore
    this->SetIsListeningToMail(false);

    // in real implementation either join here or outside.
    return BVStatus::BVSTATUS_OK;
}

BVStatus MockDiscovery::OnRestart(std::unique_ptr<std::any>)
{
    // clear everything and start
}

MockApp::MockApp(std::shared_ptr<threadsafe_queue<BVMessage>> _outMbx,
        std::shared_ptr<threadsafe_queue<BVMessage>> _inMbx,
        boost::asio::io_context& _ioContext,
        std::shared_ptr<spdlog::logger> _fileLogger) :
    BVComponent(_outMbx, _inMbx),
    ioContext(_ioContext),
    announceTimer(_ioContext),
    pauseDiscoveryTimer(_ioContext),
    fileLogger(_fileLogger)
{
    RegisterCallback(BVEventType::BVEVENTTYPE_APP_PUBLISHED_SERVICE,
                     std::bind(&MockApp::HandlePublishedServices, this, std::placeholders::_1));
    RegisterCallback(BVEventType::BVEVENTTYPE_TERMINATE_ALL,
                    std::bind(&MockApp::OnShutdown, this, std::placeholders::_1));
    // Maybe some messages that are announcements of failures?
}

// worker thread
// App will announce their services 'at will',
// and do other things.
// This is implemented by processing the std::queue<TaskFunction> tasks_q.
void MockApp::Run(void)
{
    while (this->GetIsRunning() && !this->tasks_q.empty())
    {
        // Wait a while
        // process queue (launch a task)
        // loop
        auto f = this->tasks_q.front();
        f();
        this->tasks_q.pop();
    }
    if (this->tasks_q.empty())
    {
        // notify main thread that it can go, but it will just sleep for now...
    }
}

BVStatus MockApp::HandlePublishedServices(std::unique_ptr<std::any> dp)
{
    using BVServiceBrowseInstanceList = std::list<BVServiceBrowseInstance>;
    if (dp == nullptr)
    {
        std::cerr << "Expected new services, got NULL" << std::endl;
        return BVStatus::BVSTATUS_FATAL_ERROR;
    }
    BVServiceBrowseInstanceList newServicesList;
    try
    {
        newServicesList = std::any_cast<BVServiceBrowseInstanceList>(*dp);    
    }
    catch(const std::bad_any_cast& e)
    {
        std::cerr << "Bad cast in BVEventType::BVEVENTTYPE_APP_PUBLISHED_SERVICE callback. " 
            << e.what() << std::endl;
        return BVStatus::BVSTATUS_FATAL_ERROR;
    }
    // Update service vector.
    // Does it need to be guarded? I think so, because here we are modifying it.
    // Mock client will periodically read it, but real user in the product implementation
    // will try to read it and they might do it when this is updated here

    std::lock_guard<std::mutex> l(this->serviceVectorMutex);
    for (auto& lElem : newServicesList)
    {
        if ((std::find(this->serviceV.begin(), this->serviceV.end(), lElem) == this->serviceV.end()))
        {
            this->serviceV.push_back(lElem);
        }
    }
    return BVStatus::BVSTATUS_OK;
}

BVStatus MockApp::OnStart(std::unique_ptr<std::any> dp)
{
    return BVStatus::BVSTATUS_OK;
}

BVStatus MockApp::OnShutdown(std::unique_ptr<std::any> dp)
{
    this->SetIsListeningToMail(false);
    this->SetIsRunning(false);
    return BVStatus::BVSTATUS_OK;
}

BVStatus MockApp::OnRestart(std::unique_ptr<std::any> dp)
{
    return BVStatus::BVSTATUS_OK;
}

BVStatus MockApp::OnPause(std::unique_ptr<std::any> dp)
{
    return BVStatus::BVSTATUS_OK;
}

void MockApp::TaskAnnounce(void)
{
    // Guard the shared resource (serviceV)!
    std::lock_guard<std::mutex> l(this->serviceVectorMutex);

    std::string logTrace;
    for (auto& lElem : this->serviceV)
    {
        logTrace = "App announces " + lElem.serviceName;
        SPDLOG_LOGGER_TRACE(this->fileLogger, logTrace);
        fileLogger->flush();
    }
}

void MockApp::TaskPauseDiscovery(void)
{
    // TODO: send message
}

void MockApp::TaskQuit(void)
{
    SPDLOG_LOGGER_TRACE(this->fileLogger, "App quits.");
    fileLogger->flush();
    SendMessage(BVMessage(
                BVEventType::BVEVENTTYPE_TERMINATE_ALL, nullptr));
}

void MockApp::TaskSleep(void)
{
    SPDLOG_LOGGER_TRACE(this->fileLogger, "App sleeps...");
    fileLogger->flush();
    std::this_thread::sleep_for(std::chrono::milliseconds(this->taskSleepMs));
}

TestHeartbeatComponent::TestHeartbeatComponent(std::vector<BVEventType> _eventTypesOfInterest,
                        std::shared_ptr<threadsafe_queue<BVMessage>> _outMbx,
                        std::shared_ptr<threadsafe_queue<BVMessage>> _inMbx,
                        boost::asio::io_context& _iocontext,
                        const int _hid,
                        const size_t _heartbeatMs) :
BVComponent(_outMbx, _inMbx),
eventTypesOfInterest(_eventTypesOfInterest),
ioContext(_iocontext),
timer(_iocontext),
hid(_hid),
heartbeatMs(_heartbeatMs)
{
    RegisterCallback(BVEventType::BVEVENTTYPE_TEST_REQUEST_START,
                     std::bind(&TestHeartbeatComponent::OnStart, this, std::placeholders::_1));

    RegisterCallback(BVEventType::BVEVENTTYPE_TEST_REQUEST_PAUSE, 
                     std::bind(&TestHeartbeatComponent::OnPause, this, std::placeholders::_1));

    RegisterCallback(BVEventType::BVEVENTTYPE_TERMINATE_ALL,
                     std::bind(&TestHeartbeatComponent::OnShutdown, this, std::placeholders::_1));

    RegisterCallback(BVEventType::BVEVENTTYPE_TEST_REQUEST_SHUTDOWN,
                     std::bind(&TestHeartbeatComponent::OnShutdown, this, std::placeholders::_1));

    RegisterCallback(BVEventType::BVEVENTTYPE_TEST_REQUEST_RESTART,
                     std::bind(&TestHeartbeatComponent::OnRestart, this, std::placeholders::_1));

    RegisterCallback(BVEventType::BVEVENTTYPE_TERMINATE_ALL,
                     std::bind(&TestHeartbeatComponent::OnShutdown, this, std::placeholders::_1));

    Setup();
}

void TestHeartbeatComponent::Setup(void)
{
}

// this has to be on another thread just to simulate another component working
void TestHeartbeatComponent::LaunchWorkerThread(void)
{
    worker_thread = std::thread([&] {
        this->StartAnnouncingHeartbeat();
    });
}

void TestHeartbeatComponent::StartAnnouncingHeartbeat(void)
{
    // send message after x ms
    this->timer.expires_after(std::chrono::milliseconds(this->heartbeatMs));
    this->timer.async_wait([this](const boost::system::error_code& /*e*/) {this->Beat();});
    this->ioContext.run();
}

void TestHeartbeatComponent::Beat(void)
{
    if (this->working)
    {
        SendMessage(BVMessage(
                    BVEventType::BVEVENTTYPE_TEST_HEARTBEAT, 
                        std::make_unique<std::any>(std::make_any<int>(this->hid))));
        this->timer.expires_after(std::chrono::milliseconds(this->heartbeatMs));
        this->timer.async_wait([this](const boost::system::error_code& e) {
            if (e != boost::asio::error::operation_aborted)
                this->Beat();
        });
    }
}

BVStatus TestHeartbeatComponent::OnStart(std::unique_ptr<std::any>)
{
    this->LaunchWorkerThread(); // this should be launched by the main thread
}

BVStatus TestHeartbeatComponent::OnShutdown(std::unique_ptr<std::any>)
{
    this->working = false;
    this->timer.cancel();
    this->ioContext.stop();
    return BVStatus::BVSTATUS_OK;
}

BVStatus TestHeartbeatComponent::OnRestart(std::unique_ptr<std::any>)
{

}

BVStatus TestHeartbeatComponent::OnPause(std::unique_ptr<std::any>)
{
}

TestHeartbeatListenerComponent::TestHeartbeatListenerComponent(
                                   std::shared_ptr<threadsafe_queue<BVMessage>> _outMbx,
                                   std::shared_ptr<threadsafe_queue<BVMessage>> _inMbx,
                                   const int _hid) :
BVComponent(_outMbx, _inMbx),
hid(_hid)
{
    RegisterCallback(BVEventType::BVEVENTTYPE_TEST_REQUEST_START,
                     std::bind(&TestHeartbeatListenerComponent::OnStart, this, std::placeholders::_1));

    RegisterCallback(BVEventType::BVEVENTTYPE_TEST_REQUEST_PAUSE, 
                     std::bind(&TestHeartbeatListenerComponent::OnPause, this, std::placeholders::_1));

    RegisterCallback(BVEventType::BVEVENTTYPE_TEST_REQUEST_SHUTDOWN,
                     std::bind(&TestHeartbeatListenerComponent::OnShutdown, this, std::placeholders::_1));

    RegisterCallback(BVEventType::BVEVENTTYPE_TEST_REQUEST_RESTART,
                     std::bind(&TestHeartbeatListenerComponent::OnRestart, this, std::placeholders::_1));
                                          
    RegisterCallback(BVEventType::BVEVENTTYPE_TERMINATE_ALL,
                     std::bind(&TestHeartbeatListenerComponent::OnShutdown, this, std::placeholders::_1));

    RegisterCallback(BVEventType::BVEVENTTYPE_TEST_HEARTBEAT,
                        [this](std::unique_ptr<std::any> dp) -> BVStatus {
                        // unpack hid
                        int _hid;
                        try
                        {
                            _hid = std::any_cast<int>(*dp);
                        } catch(const std::bad_any_cast& e)
                        {
                            std::cerr << "Bad cast in BVEventType::BVEVENTTYPE_TEST_HEARTBEAT callback. " 
                                    << e.what() << std::endl;
                            return BVStatus::BVSTATUS_FATAL_ERROR;
                        }
                        if (this->hid == _hid)
                        {
                            SendMessage(BVMessage(
                                BVEventType::BVEVENTTYPE_TEST_HEARTBEAT_ACK, 
                                    std::make_unique<std::any>(
                                        std::make_any<int>(_hid))));
                        }
                        return BVStatus::BVSTATUS_OK;
                    });
}

void TestHeartbeatListenerComponent::Setup(void)
{

}

BVStatus TestHeartbeatListenerComponent::OnStart(std::unique_ptr<std::any>)
{

}

BVStatus TestHeartbeatListenerComponent::OnShutdown(std::unique_ptr<std::any>)
{
    return BVStatus::BVSTATUS_OK;
}

BVStatus TestHeartbeatListenerComponent::OnRestart(std::unique_ptr<std::any>)
{

}

BVStatus TestHeartbeatListenerComponent::OnPause(std::unique_ptr<std::any>)
{

}

TCComponent::TCComponent(std::shared_ptr<threadsafe_queue<BVMessage>> _outMbx,
                         std::shared_ptr<threadsafe_queue<BVMessage>> _inMbx) :
BVComponent(_outMbx, _inMbx)
{
    RegisterCallback(BVEventType::BVEVENTTYPE_TEST_REQUEST_START,
                     std::bind(&TCComponent::OnStart, this, std::placeholders::_1));

    RegisterCallback(BVEventType::BVEVENTTYPE_TEST_REQUEST_PAUSE, 
                     std::bind(&TCComponent::OnPause, this, std::placeholders::_1));

    RegisterCallback(BVEventType::BVEVENTTYPE_TERMINATE_ALL,
                     std::bind(&TCComponent::OnShutdown, this, std::placeholders::_1));

    RegisterCallback(BVEventType::BVEVENTTYPE_TEST_REQUEST_SHUTDOWN,
                     std::bind(&TCComponent::OnShutdown, this, std::placeholders::_1));

    RegisterCallback(BVEventType::BVEVENTTYPE_TEST_REQUEST_RESTART,
                     std::bind(&TCComponent::OnRestart, this, std::placeholders::_1));

    RegisterCallback(BVEventType::BVEVENTTYPE_TEST_HEARTBEAT_ACK,
        [this](std::unique_ptr<std::any> dp) -> BVStatus {
            ack = true;
            return BVStatus::BVSTATUS_OK;
        });
}

BVStatus TCComponent::OnStart(std::unique_ptr<std::any>)
{
}

BVStatus TCComponent::OnShutdown(std::unique_ptr<std::any>)
{
    return BVStatus::BVSTATUS_OK;
}

BVStatus TCComponent::OnRestart(std::unique_ptr<std::any>)
{
}

BVStatus TCComponent::OnPause(std::unique_ptr<std::any>)
{
}