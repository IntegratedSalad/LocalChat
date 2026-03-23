#include <gtest/gtest.h>
#include <filesystem>
#include "component_mocks.hpp"
#include "threadsafequeue.hpp"
#include "BVBroker.hpp"
#include "BVSubscriber.hpp"
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"

class CommunicationFixture : public ::testing::Test
{
protected:
    // Broker - Routes messages
    // and manages components being subscribed to different messages (topics)
    std::unique_ptr<BVBroker> broker_p;

    // Discovery Mock - continuously publishes a mock service
    // Producer
    std::unique_ptr<MockDiscovery> discovery_mock_p;

    // App Mock - continuously receives mock services
    // and acts as a controller of the state of the application
    // Consumer
    std::unique_ptr<MockApp> app_mock_p;
    boost::asio::io_context ioContext;

    // Logging
    std::shared_ptr<spdlog::logger> fileLogger;

    void SetUp() override
    {
        // logging
        namespace fs = std::filesystem;
        const ::testing::TestInfo* test_info =
            ::testing::UnitTest::GetInstance()->current_test_info();
        spdlog::set_level(spdlog::level::trace);
        spdlog::set_pattern("[%H:%M:%S %z] [logger %n] [thread %t] %v");
        try 
        {
            fs::path logDir = "logs";
            fs::create_directories(logDir);

            std::string fileName = 
                std::string(test_info->test_suite_name()) + "_" + test_info->name() + ".log";

            fs::path logPath = logDir / fileName;

            fileLogger = spdlog::basic_logger_mt(
                "communication_test_Multiple_Components_Logger",
                logPath.string(),
                true);
            fileLogger->set_level(spdlog::level::trace);
            fileLogger->flush_on(spdlog::level::trace);
            SPDLOG_LOGGER_TRACE(this->fileLogger, "Logger set up.");

        }
        catch (const spdlog::spdlog_ex &ex)
        {
            std::cout << "Log init failed: " << ex.what() << std::endl;
            throw std::runtime_error("Cannot init logging handle...");
        }

        BVServiceHostData hostDataMock{.port = PORT,
                                        .domain = ".local",
                                        .hostname = "mock",
                                        .regtype = "_localchathost._tcp"};
        
        broker_p = std::make_unique<BVBroker>(std::make_shared<threadsafe_queue<BVMessage>>());
        discovery_mock_p = 
            std::make_unique<MockDiscovery>(hostDataMock,
                                            std::make_shared<threadsafe_queue<BVMessage>>(),
                                            std::make_shared<threadsafe_queue<BVMessage>>());

        app_mock_p = std::make_unique<MockApp>(std::make_shared<threadsafe_queue<BVMessage>>(),
                                               std::make_shared<threadsafe_queue<BVMessage>>(),
                                               ioContext,
                                               fileLogger);


    }

    void TearDown() override
    {
        broker_p.reset(nullptr);
        discovery_mock_p.reset(nullptr);
        app_mock_p.reset(nullptr);
        ioContext.stop();
        fileLogger.reset();
    }
};

TEST_F(CommunicationFixture, CheckInit)
{
    ASSERT_NE(broker_p, nullptr);
    ASSERT_NE(discovery_mock_p, nullptr);
    ASSERT_NE(app_mock_p, nullptr);
}

TEST_F(CommunicationFixture, CheckInitAndDeInitSequences)
{
    // Attach all components
    // Subscribe to 
    // Start all threads
    // Send BVEVENTTYPE_TERMINATE_ALL
    // join all threads
    // Detach all components
    // verify threads non-joinable
    // This tests doesn't verify MockApp simulating user input
    BVStatus attachStatusDiscovery = broker_p->Attach(*discovery_mock_p);
    BVStatus attachStatusApp = broker_p->Attach(*app_mock_p);

    ASSERT_EQ(attachStatusDiscovery, BVStatus::BVSTATUS_OK);
    ASSERT_EQ(attachStatusApp, BVStatus::BVSTATUS_OK);

    BVStatus subStatus1 = broker_p->Subscribe(discovery_mock_p->GetSubscriberId(), BVEventType::BVEVENTTYPE_DISCOVERY_REQUEST_START);
    BVStatus subStatus2 = broker_p->Subscribe(discovery_mock_p->GetSubscriberId(), BVEventType::BVEVENTTYPE_DISCOVERY_REQUEST_PAUSE);
    BVStatus subStatus3 = broker_p->Subscribe(discovery_mock_p->GetSubscriberId(), BVEventType::BVEVENTTYPE_TERMINATE_ALL);
    BVStatus subStatus4 = broker_p->Subscribe(discovery_mock_p->GetSubscriberId(), BVEventType::BVEVENTTYPE_DISCOVERY_REQUEST_SHUTDOWN);
    BVStatus subStatus5 = broker_p->Subscribe(discovery_mock_p->GetSubscriberId(), BVEventType::BVEVENTTYPE_DISCOVERY_REQUEST_RESTART);
    BVStatus subStatus6 = broker_p->Subscribe(app_mock_p->GetSubscriberId(), BVEventType::BVEVENTTYPE_APP_PUBLISHED_SERVICE);
    BVStatus subStatus7 = broker_p->Subscribe(app_mock_p->GetSubscriberId(), BVEventType::BVEVENTTYPE_TERMINATE_ALL);

    ASSERT_EQ(subStatus1, BVStatus::BVSTATUS_OK);
    ASSERT_EQ(subStatus2, BVStatus::BVSTATUS_OK);
    ASSERT_EQ(subStatus3, BVStatus::BVSTATUS_OK);
    ASSERT_EQ(subStatus4, BVStatus::BVSTATUS_OK);
    ASSERT_EQ(subStatus5, BVStatus::BVSTATUS_OK);
    ASSERT_EQ(subStatus6, BVStatus::BVSTATUS_OK);
    ASSERT_EQ(subStatus7, BVStatus::BVSTATUS_OK);

    // Launch
    broker_p->LaunchWorkerThread();
    discovery_mock_p->StartListeningOnMailbox();
    discovery_mock_p->LaunchWorkingThread();
    app_mock_p->StartListeningOnMailbox();
    app_mock_p->LaunchWorkerThread();

    // Send termination message
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    broker_p->GetInMailBoxP()->push(
        BVMessage{BVEventType::BVEVENTTYPE_TERMINATE_ALL, nullptr});

    // Join all
    discovery_mock_p->TryJoinMailBoxThread();
    discovery_mock_p->TryJoinWorkerThread();
    app_mock_p->TryJoinMailBoxThread();
    app_mock_p->TryJoinWorkerThread();
    broker_p->TryJoinWorkerThread();

    // Detach and verify
    BVStatus dDiscoveryStatus = broker_p->Detach(discovery_mock_p->GetSubscriberId());
    BVStatus dAppStatus = broker_p->Detach(app_mock_p->GetSubscriberId());
    ASSERT_EQ(dDiscoveryStatus, BVStatus::BVSTATUS_OK);
    ASSERT_EQ(dAppStatus, BVStatus::BVSTATUS_OK);

    // Verify threads are not joinable
    ASSERT_FALSE(discovery_mock_p->GetWorkerThread().joinable());
    ASSERT_FALSE(app_mock_p->GetWorkerThread().joinable());
}

TEST_F(CommunicationFixture, CheckBasicContinuousCommunicationAndDeInit)
{
    // Attach all components
    // Subscribe to 
    // Start all threads
    // Send BVEVENTTYPE_TERMINATE_ALL
    // join all threads
    // Detach all components
    // verify threads non-joinable
    // This tests doesn't verify MockApp simulating user input
    // In the product, main thread will be the app worker thread.
    // It will send BVEVENTTYPE_TERMINATE_ALL and join all threads.

    /* Tasks:
       1. Task Sleep
       2. Task Announce
       3. Task Sleep
       4. Task Sleep
       5. Task Announce
       6. Task Quit - this should send termination message!
    */
    app_mock_p->SubmitTask(std::bind(&MockApp::TaskSleep, app_mock_p.get()));
    app_mock_p->SubmitTask(std::bind(&MockApp::TaskAnnounce, app_mock_p.get()));
    app_mock_p->SubmitTask(std::bind(&MockApp::TaskSleep, app_mock_p.get()));
    app_mock_p->SubmitTask(std::bind(&MockApp::TaskSleep, app_mock_p.get()));
    app_mock_p->SubmitTask(std::bind(&MockApp::TaskSleep, app_mock_p.get()));
    app_mock_p->SubmitTask(std::bind(&MockApp::TaskAnnounce, app_mock_p.get()));
    app_mock_p->SubmitTask(std::bind(&MockApp::TaskQuit, app_mock_p.get()));

    BVStatus attachStatusDiscovery = broker_p->Attach(*discovery_mock_p);
    BVStatus attachStatusApp = broker_p->Attach(*app_mock_p);

    ASSERT_EQ(attachStatusDiscovery, BVStatus::BVSTATUS_OK);
    ASSERT_EQ(attachStatusApp, BVStatus::BVSTATUS_OK);

    BVStatus subStatus1 = broker_p->Subscribe(discovery_mock_p->GetSubscriberId(), BVEventType::BVEVENTTYPE_DISCOVERY_REQUEST_START);
    BVStatus subStatus2 = broker_p->Subscribe(discovery_mock_p->GetSubscriberId(), BVEventType::BVEVENTTYPE_DISCOVERY_REQUEST_PAUSE);
    BVStatus subStatus3 = broker_p->Subscribe(discovery_mock_p->GetSubscriberId(), BVEventType::BVEVENTTYPE_TERMINATE_ALL);
    BVStatus subStatus4 = broker_p->Subscribe(discovery_mock_p->GetSubscriberId(), BVEventType::BVEVENTTYPE_DISCOVERY_REQUEST_SHUTDOWN);
    BVStatus subStatus5 = broker_p->Subscribe(discovery_mock_p->GetSubscriberId(), BVEventType::BVEVENTTYPE_DISCOVERY_REQUEST_RESTART);
    BVStatus subStatus6 = broker_p->Subscribe(app_mock_p->GetSubscriberId(), BVEventType::BVEVENTTYPE_APP_PUBLISHED_SERVICE);
    BVStatus subStatus7 = broker_p->Subscribe(app_mock_p->GetSubscriberId(), BVEventType::BVEVENTTYPE_TERMINATE_ALL);

    ASSERT_EQ(subStatus1, BVStatus::BVSTATUS_OK);
    ASSERT_EQ(subStatus2, BVStatus::BVSTATUS_OK);
    ASSERT_EQ(subStatus3, BVStatus::BVSTATUS_OK);
    ASSERT_EQ(subStatus4, BVStatus::BVSTATUS_OK);
    ASSERT_EQ(subStatus5, BVStatus::BVSTATUS_OK);
    ASSERT_EQ(subStatus6, BVStatus::BVSTATUS_OK);
    ASSERT_EQ(subStatus7, BVStatus::BVSTATUS_OK);

    // Launch
    broker_p->LaunchWorkerThread();
    discovery_mock_p->StartListeningOnMailbox();
    discovery_mock_p->LaunchWorkingThread();
    app_mock_p->StartListeningOnMailbox();
    app_mock_p->LaunchWorkerThread();

    // sleep until app_mock queue is non empty! (or just wait for now...)
    std::this_thread::sleep_for(std::chrono::milliseconds(10000));

    // Join all
    discovery_mock_p->TryJoinMailBoxThread();
    discovery_mock_p->TryJoinWorkerThread();
    app_mock_p->TryJoinMailBoxThread();
    app_mock_p->TryJoinWorkerThread();
    broker_p->TryJoinWorkerThread();

    ASSERT_NE(app_mock_p->GetServiceVectorCopy().size(), 0);

    // Verify logs?
    // Verify queue empty
    // Verify is browsing active
    // Verify each component is not listening to mail anymore!
    // Verify app serviceV?
}


TEST_F(CommunicationFixture, CheckAppPausingDiscoveryAndResumingLater)
{
    // This cancels being able to be discovered.
}


TEST_F(CommunicationFixture, CheckAppShuttingDownDiscovery)
{
    // This should never happen, as this means that Discovery will no longer
    // be able to listen to messages.
    // Hard reset later implemented maybe?
}

TEST_F(CommunicationFixture, CheckAppRestartingDiscovery)
{

}

// These will have to be tested later
// TEST_F(CommunicationFixture, CheckAppShuttingDownService)
// {
    
// }

// TEST_F(CommunicationFixture, CheckAppRestartingService)
// {
    
// }
