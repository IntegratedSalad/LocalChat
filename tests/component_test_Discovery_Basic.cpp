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

    // not needed
    std::shared_ptr<std::queue<BVServiceBrowseInstance>> discoveryQueue_p;
    std::mutex discoveryQueueMutex;
    std::mutex messageQueueMutex;
    std::condition_variable discoveryQueueCV;
    boost::asio::io_context ioContext;
    bool isDiscoveryQueueReady = false;
    // not needed

    std::thread worker_thread; // thread for 'business' Discovery logic. Probably should be as a member function in Discovery
    
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
                                            ioContext, // to delete
                                            discoveryQueue_p, // to delete
                                            discoveryQueueCV, // to delete
                                            isDiscoveryQueueReady, // to delete
                                            outMailBox_p,
                                            inMailBox_p);
    }

    void TearDown() override {
        // worker_thread.join();
        if (worker_thread.joinable())
            worker_thread.join();
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

// Check Component listening to mailbox first,
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
    discovery_mock_p->TryJoinMailBoxThread();

    ASSERT_TRUE(inMailBox_p->empty());
    ASSERT_TRUE(outMailBox_p->empty());
}

TEST_F(DiscoveryMockBasicFixture, CheckReceivingOneServiceFromMockDiscoveryWorkerThread)
{
    // At this point, maybe try to change from manual synchronization on std::queue to 
    // threadsafequeue
    // If succeeds:
    // Use throughout the product a threadsafequeue for sending the bvservice results
    // on the discoveryQueue => so just change the discoveryQueue to threadsafequeue
    // Maybe then we can actually get rid of the queue for this and send this in
    // message data.
    
    // 1. Launch discovery worker thread
    // 2. Listen to the BVEVENTTYPE_APP_PUBLISHED_SERVICE
    // 3. Retrieve the list from the outMailBox_p on receiving the message with above even type (topic)

    // the worker thread should be available, stopped and joined from the Discovery.
    const BVEventType expectedEventType = BVEventType::BVEVENTTYPE_APP_PUBLISHED_SERVICE;

    worker_thread = std::thread([&] {
        discovery_mock_p->RunOnce();
    });

    std::shared_ptr<BVMessage> responseFromDiscoveryMsg = outMailBox_p->wait_and_pop();
    const BVEventType responseFromDiscoveryEType = responseFromDiscoveryMsg->event_t;
    std::list<BVServiceBrowseInstance> responseFromDiscovery;

    try
    {
        responseFromDiscovery = std::any_cast<std::list<BVServiceBrowseInstance>>(*responseFromDiscoveryMsg->data_p);
    }
    catch(const std::bad_any_cast& ex)
    {
        std::cerr << ex.what() << std::endl;
        FAIL();
    }

    // verify that list contains 1 element - BVServiceBrowseInstance with serviceName==TESTSERVICE1

    ASSERT_EQ(responseFromDiscoveryEType, expectedEventType);
    ASSERT_EQ(responseFromDiscovery.size(), 1);

    BVServiceBrowseInstance bI = responseFromDiscovery.back();
    ASSERT_EQ(bI.serviceName, "TESTSERVICE1");

    ASSERT_TRUE(inMailBox_p->empty());
    ASSERT_TRUE(outMailBox_p->empty());
}

TEST_F(DiscoveryMockBasicFixture, CheckReceivingMultipleMessagesFromMockDiscoveryWorkerThread)
{
    // Simulate Discovery putting multiple messages ahead of consumer processing them.
    const int n = 5;
    const BVEventType expectedEventType = BVEventType::BVEVENTTYPE_APP_PUBLISHED_SERVICE;

    worker_thread = std::thread([&] {
        discovery_mock_p->RunNTimes(n);
    });

    worker_thread.join(); // wait for the thread to finish

    std::shared_ptr<BVMessage> responseFromDiscoveryMsg;
    BVEventType responseFromDiscoveryEType;
    std::list<BVServiceBrowseInstance> responseFromDiscovery;
    int n_loop = 0;
    while (!outMailBox_p->empty())
    {
        responseFromDiscoveryMsg = outMailBox_p->wait_and_pop();
        responseFromDiscoveryEType = responseFromDiscoveryMsg->event_t;
        try
        {
            responseFromDiscovery = std::any_cast<std::list<BVServiceBrowseInstance>>(*responseFromDiscoveryMsg->data_p);
            // there should be 'n' messages with one element each
            ASSERT_EQ(responseFromDiscoveryEType, expectedEventType);
            ASSERT_EQ(responseFromDiscovery.size(), 1);
            BVServiceBrowseInstance bI = responseFromDiscovery.front(); responseFromDiscovery.pop_front();
            const std::string nameToCheck("TESTSERVICE" + std::to_string(n_loop));
            ASSERT_EQ(bI.serviceName, nameToCheck);
        }
        catch(const std::bad_any_cast& ex)
        {
            std::cerr << ex.what() << std::endl;
            FAIL();
        }
        n_loop++;
    }
    ASSERT_TRUE(inMailBox_p->empty());
    ASSERT_TRUE(outMailBox_p->empty());
}

TEST_F(DiscoveryMockBasicFixture, CheckReceivingMultipleMessagesWithMultipleElementsFromMockDiscoveryWorkerThread)
{
    const int n = 5;
    const int k = 10;
    const BVEventType expectedEventType = BVEventType::BVEVENTTYPE_APP_PUBLISHED_SERVICE;

    worker_thread = std::thread([&] {
        discovery_mock_p->RunNTimesWithKElements(n, k);
    });

    worker_thread.join(); // wait for the thread to finish
    
    std::shared_ptr<BVMessage> responseFromDiscoveryMsg;
    BVEventType responseFromDiscoveryEType;
    std::list<BVServiceBrowseInstance> responseFromDiscovery;
    int n_loop = 0;
    int k_element = 0;
    
    while (!outMailBox_p->empty())
    {
        responseFromDiscoveryMsg = outMailBox_p->wait_and_pop();
        responseFromDiscoveryEType = responseFromDiscoveryMsg->event_t;
        try
        {
            responseFromDiscovery = std::any_cast<std::list<BVServiceBrowseInstance>>(*responseFromDiscoveryMsg->data_p);
            // there should be 'n' messages with one element each
            ASSERT_EQ(responseFromDiscoveryEType, expectedEventType);
            ASSERT_EQ(responseFromDiscovery.size(), k);
            for (auto& bI : responseFromDiscovery)
            {
                const std::string nameToCheck("TESTSERVICE" + std::to_string(k_element));
                ASSERT_EQ(bI.serviceName, nameToCheck);
                k_element++;
            }
            responseFromDiscovery.clear();
            k_element = 0;
        }
        catch(const std::bad_any_cast& ex)
        {
            std::cerr << ex.what() << std::endl;
            FAIL();
        }
        n_loop++;
    }
    ASSERT_TRUE(inMailBox_p->empty());
    ASSERT_TRUE(outMailBox_p->empty());
}

TEST_F(DiscoveryMockBasicFixture, CheckReceivingContinuousServiceMockDiscoveryResults)
{
    // 1. Start listening on the mailbox
    // 2. Start worker thread 'Run' job
    //  2a. Worker waits 3s (simulating waiting for daemon response - someone registered)
    //  2b. Worker populates the list with 2 TESTSERVICES and sends a message to outMailbox
    // 3. 
    // 4. After 10-15 received TESTSERVICES terminate MockDiscovery (send BVEventType::BVEVENTTYPE_TERMINATE_ALL):
    //  4.1 Terminate mailbox thread (it should be joined by Discovery object!)
    //  4.2 Terminate worker thread <- this always happens from within BVComponent,
    //     an object that checks mailbox.
    // 5. Check if mailboxes are empty
    // 6. Check if isListeningToMail and isBrowsingActive are false.

    /*
        This is a very important test that looks at the Discovery object
        as an object performing Discovery ('business' logic) and listening to mailbox.
    */

    // This has to be started within Discovery, as this is something that happens on start,
    // and later restart if requested.
    // Also - we have to make sure that every flag/Setup is carried out in BVDiscovery
    const BVEventType expectedEventType = BVEventType::BVEVENTTYPE_APP_PUBLISHED_SERVICE;

    discovery_mock_p->StartListeningOnMailbox();
    // Shouldn't this be started by a message? and shouldn't worker thread be in Discovery?
    worker_thread = std::thread([&] {
        discovery_mock_p->RunContinuously();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(200)); // wait for IsBrowsingActive to be set by the worker thread
    ASSERT_TRUE(discovery_mock_p->GetIsBrowsingActive());

    // we aren't checking here pushing multiple messages

    int receivedServicesNum = 0;
    const int maxToReceiveServicesNum = 15;
    std::shared_ptr<BVMessage> responseFromDiscoveryMsg;
    BVEventType responseFromDiscoveryEType;
    std::list<BVServiceBrowseInstance> responseFromDiscovery;
    
    while (receivedServicesNum < maxToReceiveServicesNum)
    {
        if (!outMailBox_p->empty())
        {
            responseFromDiscoveryMsg = outMailBox_p->wait_and_pop();
            responseFromDiscoveryEType = responseFromDiscoveryMsg->event_t;
            ASSERT_EQ(responseFromDiscoveryEType, expectedEventType);
            try
            {
                responseFromDiscovery = std::any_cast<std::list<BVServiceBrowseInstance>>(*responseFromDiscoveryMsg->data_p);
                ASSERT_EQ(responseFromDiscovery.size(), 2);
                for (auto& bI : responseFromDiscovery)
                {
                    const std::string nameToCheck("TESTSERVICE" + std::to_string(receivedServicesNum));
                    ASSERT_EQ(bI.serviceName, nameToCheck);
                    receivedServicesNum++;
                }
                responseFromDiscovery.clear();
            }
            catch(const std::bad_any_cast& ex)
            {
                std::cerr << ex.what() << std::endl;
                FAIL();
            }
        }
    }

    inMailBox_p->push(
        BVMessage{BVEventType::BVEVENTTYPE_TERMINATE_ALL, nullptr});
    discovery_mock_p->TryJoinMailBoxThread();
    ASSERT_FALSE(discovery_mock_p->GetIsListeningToMail());
    ASSERT_FALSE(discovery_mock_p->GetIsBrowsingActive());

    ASSERT_TRUE(inMailBox_p->empty());
    ASSERT_TRUE(outMailBox_p->empty());
}

// Below tests are still without Broker - to test callbacks/functions 
// Maybe do them later - the most important test is CheckReceivingContinuousServiceMockDiscoveryResults

TEST_F(DiscoveryMockBasicFixture, CheckDiscoveryStartRequested)
{
    // Test that you cannot start twice - or that should be from FSM?
    ASSERT_FALSE(discovery_mock_p->GetIsBrowsingActive());
    ASSERT_FALSE(discovery_mock_p->GetIsListeningToMail());
}

TEST_F(DiscoveryMockBasicFixture, CheckDiscoveryPauseRequested)
{

}

TEST_F(DiscoveryMockBasicFixture, CheckDiscoveryShutdownRequested)
{

}

TEST_F(DiscoveryMockBasicFixture, CheckDiscoveryRestartRequested)
{

}

