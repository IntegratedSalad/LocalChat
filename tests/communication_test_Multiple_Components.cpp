#include "component_mocks.hpp"
#include "threadsafequeue.hpp"
#include "BVBroker.hpp"
#include "BVSubscriber.hpp"
#include <gtest/gtest.h>

class CommunicationFixture : public ::testing::Test
{
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

    void SetUp() override
    {
        BVServiceHostData hostDataMock{.port = PORT,
                                        .domain = ".local",
                                        .hostname = "mock",
                                        .regtype = "_localchathost._tcp"};
        
        broker_p = std::make_unique<BVBroker>(std::make_shared<threadsafe_queue<BVMessage>>());
    }

    void TearDown() override
    {
        broker_p.reset(nullptr);
        discovery_mock_p.reset(nullptr);
        app_mock_p.reset(nullptr);
    }
};

TEST_F(CommunicationFixture, CheckInit)
{
}
