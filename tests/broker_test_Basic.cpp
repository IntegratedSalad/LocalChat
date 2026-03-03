#include "component_mocks.hpp"
#include "threadsafequeue.hpp"
#include "BVBroker.hpp"
#include "BVSubscriber.hpp"
#include <gtest/gtest.h>
/*
    We can test broker and components initialization
    and communication with fake Discovery and App components "talking".
    We can simulate them doing their job (waiting) and scenarios.
    We can assert that their mailboxes are shared across threads.
    We can assert that no thread is starved.

    We also can derive from BVDiscovery and BVService and BVApp and
    override the functions that perform DNS-SD to simulate their behavior,
    not performing the actual DNS-SD, but simulating the outcome of 
    their functionality.

    1. Test attaching and detaching Components from Broker.
    2. Test communication

    // Test having N components trying to communicate with each other (push messages to their mailboxes)
*/

// std::shared_ptr<threadsafe_queue<BVMessage>> outMailBox_p; <- Component writes to it
// std::shared_ptr<threadsafe_queue<BVMessage>> inMailBox_p;  <- Component reads from it
class BrokerBasicFixture : public ::testing::Test
{
protected:
    std::unique_ptr<BVBroker> broker_p;
    std::shared_ptr<threadsafe_queue<BVMessage>> inMailBox_p;

    void SetUp() override
    {
        inMailBox_p = std::make_shared<threadsafe_queue<BVMessage>>();
        broker_p = std::make_unique<BVBroker>(inMailBox_p);
    }

    void TearDown() override {
        broker_p.reset(nullptr);
    }
};

TEST_F(BrokerBasicFixture, CheckInit)
{
    ASSERT_NE(broker_p, nullptr);
    ASSERT_NE(inMailBox_p, nullptr);
    ASSERT_NE(broker_p->GetInMailBoxP(), nullptr);
    ASSERT_TRUE(inMailBox_p->empty());
}

TEST_F(BrokerBasicFixture, CheckAttachingComponents)
{
    // Attach 4 components
    // check their SubscriberID

    boost::asio::io_context io_context;
    int hid = 0;
    TestHeartbeatComponent tc1{std::vector<BVEventType>{},
                               std::make_shared<threadsafe_queue<BVMessage>>(),
                               inMailBox_p,
                               io_context,
                               hid,
                               200};
    hid+=1;
    TestHeartbeatComponent tc2{std::vector<BVEventType>{},
                               std::make_shared<threadsafe_queue<BVMessage>>(),
                               inMailBox_p,
                               io_context,
                               hid,
                               200};
    hid+=1;
    TestHeartbeatComponent tc3{std::vector<BVEventType>{},
                               std::make_shared<threadsafe_queue<BVMessage>>(),
                               inMailBox_p,
                               io_context,
                               hid,
                               200};
    hid+=1;
    TestHeartbeatComponent tc4{std::vector<BVEventType>{},
                               std::make_shared<threadsafe_queue<BVMessage>>(),
                               inMailBox_p,
                               io_context,
                               hid,
                               200};
    BVStatus attachStatusTc1 = broker_p->Attach(tc1);
    BVStatus attachStatusTc2 = broker_p->Attach(tc2);
    BVStatus attachStatusTc3 = broker_p->Attach(tc3);
    BVStatus attachStatusTc4 = broker_p->Attach(tc4);
    
    ASSERT_EQ(attachStatusTc1, BVStatus::BVSTATUS_OK);
    ASSERT_EQ(attachStatusTc2, BVStatus::BVSTATUS_OK);
    ASSERT_EQ(attachStatusTc3, BVStatus::BVSTATUS_OK);
    ASSERT_EQ(attachStatusTc4, BVStatus::BVSTATUS_OK);

    ASSERT_EQ(tc1.GetSubscriberId(), (SubscriberID)0);
    ASSERT_EQ(tc2.GetSubscriberId(), (SubscriberID)1);
    ASSERT_EQ(tc3.GetSubscriberId(), (SubscriberID)2);
    ASSERT_EQ(tc4.GetSubscriberId(), (SubscriberID)3);

    ASSERT_NE(broker_p->GetOutMailBoxAtSubId(tc1.GetSubscriberId()), nullptr);
    ASSERT_NE(broker_p->GetOutMailBoxAtSubId(tc2.GetSubscriberId()), nullptr);
    ASSERT_NE(broker_p->GetOutMailBoxAtSubId(tc3.GetSubscriberId()), nullptr);
    ASSERT_NE(broker_p->GetOutMailBoxAtSubId(tc4.GetSubscriberId()), nullptr);
}

TEST_F(BrokerBasicFixture, CheckDetachingComponents)
{
    boost::asio::io_context io_context;
    int hid = 0;
    TestHeartbeatComponent tc1{std::vector<BVEventType>{},
                               std::make_shared<threadsafe_queue<BVMessage>>(),
                               inMailBox_p,
                               io_context,
                               hid,
                               200};
    hid+=1;
    TestHeartbeatComponent tc2{std::vector<BVEventType>{},
                               std::make_shared<threadsafe_queue<BVMessage>>(),
                               inMailBox_p,
                               io_context,
                               hid,
                               200};
    hid+=1;
    TestHeartbeatComponent tc3{std::vector<BVEventType>{},
                               std::make_shared<threadsafe_queue<BVMessage>>(),
                               inMailBox_p,
                               io_context,
                               hid,
                               200};
    hid+=1;
    TestHeartbeatComponent tc4{std::vector<BVEventType>{},
                               std::make_shared<threadsafe_queue<BVMessage>>(),
                               inMailBox_p,
                               io_context,
                               hid,
                               200};
    BVStatus attachStatusTc1 = broker_p->Attach(tc1);
    BVStatus attachStatusTc2 = broker_p->Attach(tc2);
    BVStatus attachStatusTc3 = broker_p->Attach(tc3);
    BVStatus attachStatusTc4 = broker_p->Attach(tc4);

    BVStatus detachStatusTc1 = broker_p->Detach(tc1.GetSubscriberId());
    BVStatus detachStatusTc2 = broker_p->Detach(tc2.GetSubscriberId());
    BVStatus detachStatusTc3 = broker_p->Detach(tc3.GetSubscriberId());
    BVStatus detachStatusTc4 = broker_p->Detach(tc4.GetSubscriberId());

    ASSERT_EQ(detachStatusTc1, BVStatus::BVSTATUS_OK);
    ASSERT_EQ(detachStatusTc2, BVStatus::BVSTATUS_OK);
    ASSERT_EQ(detachStatusTc3, BVStatus::BVSTATUS_OK);
    ASSERT_EQ(detachStatusTc4, BVStatus::BVSTATUS_OK);

    ASSERT_EQ(broker_p->GetOutMailBoxAtSubId(tc1.GetSubscriberId()), nullptr);
    ASSERT_EQ(broker_p->GetOutMailBoxAtSubId(tc2.GetSubscriberId()), nullptr);
    ASSERT_EQ(broker_p->GetOutMailBoxAtSubId(tc3.GetSubscriberId()), nullptr);
    ASSERT_EQ(broker_p->GetOutMailBoxAtSubId(tc4.GetSubscriberId()), nullptr);
}

TEST_F(BrokerBasicFixture, CheckSubscribingToEvent)
{

}

TEST_F(BrokerBasicFixture, CheckSubscribingToMultipleEvents)
{

}

TEST_F(BrokerBasicFixture, CheckUnsubscribingToEvent)
{

}

TEST_F(BrokerBasicFixture, CheckUnsubscribingToMultipleEvents)
{

}

TEST_F(BrokerBasicFixture, CheckBasicRouting)
{

}

TEST_F(BrokerBasicFixture, CheckMessageNotRoutedAfterUnsubscribing)
{

}