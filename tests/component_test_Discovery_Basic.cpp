#include <gtest/gtest.h>
#include "component_mocks.hpp"
#include "threadsafequeue.hpp"
#include <boost/asio.hpp>

class DiscoveryMockBasicFixture : public ::testing::Test
{
protected:

    // simulating broker
    std::shared_ptr<threadsafe_queue<BVMessage>> outMailBox_p; // Component writes to it
    std::shared_ptr<threadsafe_queue<BVMessage>> inMailBox_p; // Component reads from it

    std::shared_ptr<std::queue<BVServiceBrowseInstance>> discoveryQueue_p;
    std::mutex discoveryQueueMutex;
    std::mutex messageQueueMutex;
    std::condition_variable discoveryQueueCV;
    boost::asio::io_context ioContext;
    bool isDiscoveryQueueReady = false;

    std::thread worker_thread; // thread for 'business' Discovery logic
    
    std::unique_ptr<MockDiscovery> discovery_mock_p;

    void SetUp() override {
        inMailBox_p = std::make_shared<threadsafe_queue<BVMessage>>();
        outMailBox_p = std::make_shared<threadsafe_queue<BVMessage>>();
        discoveryQueue_p = std::make_shared<std::queue<BVServiceBrowseInstance>>();

        BVServiceHostData hostDataMock{.port = PORT,
                                        .domain = ".local",
                                        .hostname = "mock",
                                        .regtype = "_localchathost._tcp"};
        discovery_mock_p = 
            std::make_unique<MockDiscovery>(hostDataMock,
                                            discoveryQueueMutex,
                                            ioContext,
                                            discoveryQueue_p,
                                            discoveryQueueCV,
                                            isDiscoveryQueueReady,
                                            outMailBox_p,
                                            inMailBox_p);
    }

    void TearDown() override {
        // worker_thread.join();
        discovery_mock_p.reset(nullptr);
    }
};

TEST_F(DiscoveryMockBasicFixture, CheckInit)
{   
    ASSERT_NE(outMailBox_p, nullptr);
    ASSERT_NE(inMailBox_p, nullptr);
    ASSERT_NE(discoveryQueue_p, nullptr);
    ASSERT_NE(discovery_mock_p, nullptr);
    ASSERT_EQ(discovery_mock_p->GetIsconnectionContextAlive(), true);
}

// First check maybe Component listening to mailbox first,
// so only component mailbox thread
TEST_F(DiscoveryMockBasicFixture, CheckDiscoveryMockListeningOnMailbox)
{
    std::string echoMsg{"Hi from CheckDiscoveryMockListeningOnMailbox"};
    const BVEventType expectedEventType = BVEventType::BVEVENTTYPE_TEST_ECHO;

    discovery_mock_p->StartListeningOnMailbox();
    inMailBox_p->push(
        BVMessage{BVEventType::BVEVENTTYPE_TEST_ECHO, 
            std::make_unique<std::any>(std::make_any<std::string>(echoMsg))});

    // Discovery should echo on outMailBox_p
    std::shared_ptr<BVMessage> responseFromDiscovery = outMailBox_p->wait_and_pop();

    const BVEventType responseFromDiscoveryEType = responseFromDiscovery->event_t;
    std::string responseFromDiscoveryMsg{"NONE"};

    try
    {
        /* code */
        responseFromDiscoveryMsg = std::any_cast<std::string>(*responseFromDiscovery->data_p);
    }
    catch(const std::bad_any_cast& ex)
    {
        std::cerr << ex.what() << std::endl;
        FAIL();
    }

    ASSERT_EQ(responseFromDiscoveryEType, expectedEventType);
    ASSERT_EQ(echoMsg, responseFromDiscoveryMsg);

    inMailBox_p->push(
        BVMessage{BVEventType::BVEVENTTYPE_TERMINATE_ALL, nullptr});
    discovery_mock_p->GetMailBoxThread().join();
}