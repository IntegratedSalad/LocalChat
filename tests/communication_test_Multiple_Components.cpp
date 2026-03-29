#include <gtest/gtest.h>
#include <filesystem>
#include "component_mocks.hpp"
#include "threadsafequeue.hpp"
#include "BVBroker.hpp"
#include "BVSubscriber.hpp"
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"

/*
 * @name communication_test_Multiple_Components
 * @brief Testcases for testing interaction
 * of Discovery and App components as their Mocks
 * with the Broker instantiated and routing messages between them
 * This test treats Discovery as a general abstraction
 * which periodically posts some test services.
 * It does not simulate a very important detail - 
 * whenever we wait for the daemon response in Discovery implementation
 * the worker thread is blocked when there are no new services discovered
 * via browsing. This might be implemented later in another test.
 * This might not be that important, as the test might not reflect
 * challenges to face while working with the concrete implementation.
 * TODO: Documentaiton
 * @test CheckInit
 * @brief ...
 * 
 * @test CheckInitAndDeInitSequences
 * @brief ...
*/

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

            std::string loggerName =
                std::string(test_info->test_suite_name()) + "_" + test_info->name() + "_logger";
            spdlog::drop(loggerName);

            fs::path logPath = logDir / fileName;

            fileLogger = spdlog::basic_logger_mt(
                loggerName,
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
        discovery_mock_p->SetLogger(fileLogger);

        app_mock_p = std::make_unique<MockApp>(std::make_shared<threadsafe_queue<BVMessage>>(),
                                               std::make_shared<threadsafe_queue<BVMessage>>(),
                                               ioContext);
        app_mock_p->SetLogger(fileLogger);
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
       5. Task Sleep
       6. Task Announce
       7. Task Quit - this should send termination message!
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

    // Verify logs? They perhaps have to be verified manually

    // Verify queue not empty
    ASSERT_NE(app_mock_p->GetServiceVectorCopy().size(), 0);

    // Verify flags
    ASSERT_FALSE(discovery_mock_p->GetIsListeningToMail());
    ASSERT_FALSE(discovery_mock_p->GetIsBrowsingActive());
    ASSERT_FALSE(app_mock_p->GetIsListeningToMail());

    // Verify app serviceV? What do we expect to be in serviceV?

    // For now, this suffices
}

TEST_F(CommunicationFixture, CheckAppPausingDiscoveryAndResumingLater)
{
    // This cancels being able to discover.
    // The announce result before pause and after pause and a little bit of waiting
    // should be the same, because discovery had not been active.
    /* Tasks:
       1. Task Sleep
       2. Task Sleep
       3. Task Sleep
       4. Task Pause Discovery
       5. Task Announce
       6. Task Sleep
       7. Task Sleep
       8. Task Sleep
       9. Task Sleep
       10. Task Announce
       11. Task Sleep
       12. Task Sleep
       13. Task Sleep
       14. Task Sleep
       15. Task Announce
       16. Task Resume Discovery
       17. Task Sleep
       18. Task Sleep
       19. Task Sleep
       20. Task Sleep
       21. Task Announce
       22. Task Quit
    */
    discovery_mock_p->SetSleepMs(1300);
    app_mock_p->SetTaskSleepMs(500);
    app_mock_p->SubmitTask(std::bind(&MockApp::TaskSleep, app_mock_p.get()));
    app_mock_p->SubmitTask(std::bind(&MockApp::TaskSleep, app_mock_p.get()));
    app_mock_p->SubmitTask(std::bind(&MockApp::TaskSleep, app_mock_p.get()));
    app_mock_p->SubmitTask(std::bind(&MockApp::TaskPauseDiscovery, app_mock_p.get()));
    app_mock_p->SubmitTask(std::bind(&MockApp::TaskAnnounce, app_mock_p.get()));
    app_mock_p->SubmitTask(std::bind(&MockApp::TaskSleep, app_mock_p.get()));
    app_mock_p->SubmitTask(std::bind(&MockApp::TaskSleep, app_mock_p.get()));
    app_mock_p->SubmitTask(std::bind(&MockApp::TaskSleep, app_mock_p.get()));
    app_mock_p->SubmitTask(std::bind(&MockApp::TaskSleep, app_mock_p.get()));
    app_mock_p->SubmitTask(std::bind(&MockApp::TaskAnnounce, app_mock_p.get()));
    app_mock_p->SubmitTask(std::bind(&MockApp::TaskSleep, app_mock_p.get()));
    app_mock_p->SubmitTask(std::bind(&MockApp::TaskSleep, app_mock_p.get()));
    app_mock_p->SubmitTask(std::bind(&MockApp::TaskSleep, app_mock_p.get()));
    app_mock_p->SubmitTask(std::bind(&MockApp::TaskSleep, app_mock_p.get()));
    app_mock_p->SubmitTask(std::bind(&MockApp::TaskAnnounce, app_mock_p.get()));
    app_mock_p->SubmitTask(std::bind(&MockApp::TaskResumeDiscovery, app_mock_p.get()));
    app_mock_p->SubmitTask(std::bind(&MockApp::TaskSleep, app_mock_p.get()));
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
    BVStatus subStatus3 = broker_p->Subscribe(discovery_mock_p->GetSubscriberId(), BVEventType::BVEVENTTYPE_DISCOVERY_REQUEST_RESUME);
    BVStatus subStatus4 = broker_p->Subscribe(discovery_mock_p->GetSubscriberId(), BVEventType::BVEVENTTYPE_TERMINATE_ALL);
    BVStatus subStatus5 = broker_p->Subscribe(discovery_mock_p->GetSubscriberId(), BVEventType::BVEVENTTYPE_DISCOVERY_REQUEST_SHUTDOWN);
    BVStatus subStatus6 = broker_p->Subscribe(discovery_mock_p->GetSubscriberId(), BVEventType::BVEVENTTYPE_DISCOVERY_REQUEST_RESTART);
    BVStatus subStatus7 = broker_p->Subscribe(app_mock_p->GetSubscriberId(), BVEventType::BVEVENTTYPE_APP_PUBLISHED_SERVICE);
    BVStatus subStatus8 = broker_p->Subscribe(app_mock_p->GetSubscriberId(), BVEventType::BVEVENTTYPE_TERMINATE_ALL);

    ASSERT_EQ(subStatus1, BVStatus::BVSTATUS_OK);
    ASSERT_EQ(subStatus2, BVStatus::BVSTATUS_OK);
    ASSERT_EQ(subStatus3, BVStatus::BVSTATUS_OK);
    ASSERT_EQ(subStatus4, BVStatus::BVSTATUS_OK);
    ASSERT_EQ(subStatus5, BVStatus::BVSTATUS_OK);
    ASSERT_EQ(subStatus6, BVStatus::BVSTATUS_OK);
    ASSERT_EQ(subStatus7, BVStatus::BVSTATUS_OK);
    ASSERT_EQ(subStatus8, BVStatus::BVSTATUS_OK);

    // Launch
    broker_p->LaunchWorkerThread();
    discovery_mock_p->StartListeningOnMailbox();
    discovery_mock_p->LaunchWorkingThread();
    app_mock_p->StartListeningOnMailbox();
    app_mock_p->LaunchWorkerThread();

    // sleep until app_mock queue is non empty! (or just wait for now...)
    std::this_thread::sleep_for(std::chrono::milliseconds(23000));

    // Join all
    discovery_mock_p->TryJoinMailBoxThread();
    discovery_mock_p->TryJoinWorkerThread();
    app_mock_p->TryJoinMailBoxThread();
    app_mock_p->TryJoinWorkerThread();
    broker_p->TryJoinWorkerThread();

    // Verify logs? They perhaps have to be verified manually
    // It probably will show another services, when timings are off
    // (app does not pause discovery before it publishing)
    // because PauseDiscovery comes after/while discovery is sleeping.

    // Verify queue not empty
    ASSERT_NE(app_mock_p->GetServiceVectorCopy().size(), 0);

    // Verify flags
    ASSERT_FALSE(discovery_mock_p->GetIsListeningToMail());
    ASSERT_FALSE(discovery_mock_p->GetIsBrowsingActive());
    ASSERT_FALSE(app_mock_p->GetIsListeningToMail());
}

// TEST_F(CommunicationFixture, CheckAppShuttingDownDiscovery)
// {
//     // This should never happen, as this means that Discovery will no longer
//     // be able to listen to messages.
//     // Hard reset later implemented maybe?
// }

// Maybe test doing tasks without delay?

// These will have to be tested later
// TEST_F(CommunicationFixture, CheckAppShuttingDownService)
// {
    
// }

// TEST_F(CommunicationFixture, CheckAppRestartingService)
// {
    
// }
