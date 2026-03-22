#include "component_mocks.hpp"
#include "threadsafequeue.hpp"
#include "BVBroker.hpp"
#include "BVSubscriber.hpp"
#include <gtest/gtest.h>

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

    void SetUp() override
    {
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
                                               ioContext);
    }

    void TearDown() override
    {
        broker_p.reset(nullptr);
        discovery_mock_p.reset(nullptr);
        app_mock_p.reset(nullptr);
        ioContext.stop();
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

TEST_F(CommunicationFixture, CheckBasicContinuousCommunicationAndDeinit)
{
    // App
}

TEST_F(CommunicationFixture, CheckAppShuttingDownDiscovery)
{
    // This should never happen, as this means that Discovery will no longer
    // be able to listen to messages.
    // Hard reset later implemented maybe?
}

TEST_F(CommunicationFixture, CheckAppPausingDiscovery)
{
    // This cancels being able to be discovered.
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
