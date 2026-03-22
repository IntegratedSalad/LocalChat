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

// MailBoxes are NOT shared!
// if this would be the case, components would pass every message to every component's mailbox
// directly, not via broker.
// std::shared_ptr<threadsafe_queue<BVMessage>> outMailBox_p; <- Component writes to it
// std::shared_ptr<threadsafe_queue<BVMessage>> inMailBox_p;  <- Component reads from it
class BrokerBasicFixture : public ::testing::Test
{
protected:
    std::unique_ptr<BVBroker> broker_p;

    void SetUp() override
    {
        broker_p = std::make_unique<BVBroker>(std::make_shared<threadsafe_queue<BVMessage>>());
    }

    void TearDown() override
    {
        broker_p.reset(nullptr);
    }
};

TEST_F(BrokerBasicFixture, CheckInit)
{
    ASSERT_NE(broker_p, nullptr);
    ASSERT_NE(broker_p->GetInMailBoxP(), nullptr);
    ASSERT_TRUE(broker_p->GetInMailBoxP()->empty());
}

TEST_F(BrokerBasicFixture, CheckAttachingComponents)
{
    // Attach 4 components
    // check their SubscriberID

    boost::asio::io_context io_context;
    int hid = 0;
    TestHeartbeatComponent tc1{std::vector<BVEventType>{},
                               std::make_shared<threadsafe_queue<BVMessage>>(),
                               std::make_shared<threadsafe_queue<BVMessage>>(),
                               io_context,
                               hid,
                               200};
    hid+=1;
    TestHeartbeatComponent tc2{std::vector<BVEventType>{},
                               std::make_shared<threadsafe_queue<BVMessage>>(),
                               std::make_shared<threadsafe_queue<BVMessage>>(),
                               io_context,
                               hid,
                               200};
    hid+=1;
    TestHeartbeatComponent tc3{std::vector<BVEventType>{},
                               std::make_shared<threadsafe_queue<BVMessage>>(),
                               std::make_shared<threadsafe_queue<BVMessage>>(),
                               io_context,
                               hid,
                               200};
    hid+=1;
    TestHeartbeatComponent tc4{std::vector<BVEventType>{},
                               std::make_shared<threadsafe_queue<BVMessage>>(),
                               std::make_shared<threadsafe_queue<BVMessage>>(),
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
                               std::make_shared<threadsafe_queue<BVMessage>>(),
                               io_context,
                               hid,
                               200};
    hid+=1;
    TestHeartbeatComponent tc2{std::vector<BVEventType>{},
                               std::make_shared<threadsafe_queue<BVMessage>>(),
                               std::make_shared<threadsafe_queue<BVMessage>>(),
                               io_context,
                               hid,
                               200};
    hid+=1;
    TestHeartbeatComponent tc3{std::vector<BVEventType>{},
                               std::make_shared<threadsafe_queue<BVMessage>>(),
                               std::make_shared<threadsafe_queue<BVMessage>>(),
                               io_context,
                               hid,
                               200};
    hid+=1;
    TestHeartbeatComponent tc4{std::vector<BVEventType>{},
                               std::make_shared<threadsafe_queue<BVMessage>>(),
                               std::make_shared<threadsafe_queue<BVMessage>>(),
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
    boost::asio::io_context io_context;
    TestHeartbeatComponent tc{std::vector<BVEventType>{},
                              std::make_shared<threadsafe_queue<BVMessage>>(),
                              std::make_shared<threadsafe_queue<BVMessage>>(),
                              io_context,
                              0,
                              200};
    TestHeartbeatComponent tc1{std::vector<BVEventType>{},
                            std::make_shared<threadsafe_queue<BVMessage>>(),
                            std::make_shared<threadsafe_queue<BVMessage>>(),
                            io_context,
                            0,
                            200}; // to test subscribing to the same event
    BVStatus attachStatusTc1 = broker_p->Attach(tc);
    BVStatus attachStatusTc2 = broker_p->Attach(tc1);
    BVStatus subStatus1 = broker_p->Subscribe(tc.GetSubscriberId(), BVEventType::BVEVENTTYPE_TEST_REQUEST_START);
    BVStatus subStatus2 = broker_p->Subscribe(tc.GetSubscriberId(), BVEventType::BVEVENTTYPE_TEST_REQUEST_PAUSE);
    BVStatus subStatus3 = broker_p->Subscribe(tc.GetSubscriberId(), BVEventType::BVEVENTTYPE_TERMINATE_ALL);
    BVStatus subStatus4 = broker_p->Subscribe(tc.GetSubscriberId(), BVEventType::BVEVENTTYPE_TEST_REQUEST_SHUTDOWN);
    BVStatus subStatus5 = broker_p->Subscribe(tc.GetSubscriberId(), BVEventType::BVEVENTTYPE_TEST_REQUEST_RESTART);
    BVStatus subStatus6 = broker_p->Subscribe(tc1.GetSubscriberId(), BVEventType::BVEVENTTYPE_TEST_REQUEST_START);
    BVStatus subStatus7 = broker_p->Subscribe(tc1.GetSubscriberId(), BVEventType::BVEVENTTYPE_TEST_REQUEST_PAUSE);
    BVStatus subStatus8 = broker_p->Subscribe(tc1.GetSubscriberId(), BVEventType::BVEVENTTYPE_TERMINATE_ALL);
    BVStatus subStatus9 = broker_p->Subscribe(tc1.GetSubscriberId(), BVEventType::BVEVENTTYPE_TEST_REQUEST_SHUTDOWN);
    BVStatus subStatus10 = broker_p->Subscribe(tc1.GetSubscriberId(), BVEventType::BVEVENTTYPE_TEST_REQUEST_RESTART);

    ASSERT_EQ(subStatus1, BVStatus::BVSTATUS_OK);
    ASSERT_EQ(subStatus2, BVStatus::BVSTATUS_OK);
    ASSERT_EQ(subStatus3, BVStatus::BVSTATUS_OK);
    ASSERT_EQ(subStatus4, BVStatus::BVSTATUS_OK);
    ASSERT_EQ(subStatus5, BVStatus::BVSTATUS_OK);
    ASSERT_EQ(subStatus6, BVStatus::BVSTATUS_OK);
    ASSERT_EQ(subStatus7, BVStatus::BVSTATUS_OK);
    ASSERT_EQ(subStatus8, BVStatus::BVSTATUS_OK);
    ASSERT_EQ(subStatus9, BVStatus::BVSTATUS_OK);
    ASSERT_EQ(subStatus10, BVStatus::BVSTATUS_OK);

    std::vector<SubscriberID> sidStart_v = broker_p->GetSubscriberIDVectorFromEventType(BVEventType::BVEVENTTYPE_TEST_REQUEST_START);
    std::vector<SubscriberID> sidPause_v = broker_p->GetSubscriberIDVectorFromEventType(BVEventType::BVEVENTTYPE_TEST_REQUEST_PAUSE);
    std::vector<SubscriberID> sidAll_v = broker_p->GetSubscriberIDVectorFromEventType(BVEventType::BVEVENTTYPE_TERMINATE_ALL);
    std::vector<SubscriberID> sidShutdown_v = broker_p->GetSubscriberIDVectorFromEventType(BVEventType::BVEVENTTYPE_TEST_REQUEST_SHUTDOWN);
    std::vector<SubscriberID> sidRestart_v = broker_p->GetSubscriberIDVectorFromEventType(BVEventType::BVEVENTTYPE_TEST_REQUEST_RESTART);

    ASSERT_EQ(sidStart_v.size(), 2);
    ASSERT_EQ(sidPause_v.size(), 2);
    ASSERT_EQ(sidAll_v.size(), 2);
    ASSERT_EQ(sidShutdown_v.size(), 2);
    ASSERT_EQ(sidRestart_v.size(), 2);

    ASSERT_EQ(sidStart_v[0], tc.GetSubscriberId());
    ASSERT_EQ(sidStart_v[1], tc1.GetSubscriberId());
    ASSERT_EQ(sidPause_v[0], tc.GetSubscriberId());
    ASSERT_EQ(sidPause_v[1], tc1.GetSubscriberId());
    ASSERT_EQ(sidAll_v[0], tc.GetSubscriberId());
    ASSERT_EQ(sidAll_v[1], tc1.GetSubscriberId());
    ASSERT_EQ(sidShutdown_v[0], tc.GetSubscriberId());
    ASSERT_EQ(sidShutdown_v[1], tc1.GetSubscriberId());
    ASSERT_EQ(sidRestart_v[0], tc.GetSubscriberId());
    ASSERT_EQ(sidRestart_v[1], tc1.GetSubscriberId());
}

TEST_F(BrokerBasicFixture, CheckSubscribingToMultipleEvents)
{
    // later
}

TEST_F(BrokerBasicFixture, CheckUnsubscribingToEvent)
{
    boost::asio::io_context io_context;
    TestHeartbeatComponent tc{std::vector<BVEventType>{},
                              std::make_shared<threadsafe_queue<BVMessage>>(),
                              std::make_shared<threadsafe_queue<BVMessage>>(),
                              io_context,
                              0,
                              200};
    TestHeartbeatComponent tc1{std::vector<BVEventType>{},
                            std::make_shared<threadsafe_queue<BVMessage>>(),
                            std::make_shared<threadsafe_queue<BVMessage>>(),
                            io_context,
                            0,
                            200}; // to test subscribing to the same event
    BVStatus attachStatusTc1 = broker_p->Attach(tc);
    BVStatus attachStatusTc2 = broker_p->Attach(tc1);
    BVStatus subStatus1 = broker_p->Subscribe(tc.GetSubscriberId(), BVEventType::BVEVENTTYPE_TEST_REQUEST_START);
    BVStatus subStatus2 = broker_p->Subscribe(tc.GetSubscriberId(), BVEventType::BVEVENTTYPE_TEST_REQUEST_PAUSE);

    BVStatus unsubStatus1 = broker_p->Unsubscribe(tc.GetSubscriberId(), BVEventType::BVEVENTTYPE_TEST_REQUEST_START);
    BVStatus unsubStatus2 = broker_p->Unsubscribe(tc.GetSubscriberId(), BVEventType::BVEVENTTYPE_TEST_REQUEST_PAUSE);

    ASSERT_EQ(unsubStatus1, BVStatus::BVSTATUS_OK);
    ASSERT_EQ(unsubStatus2, BVStatus::BVSTATUS_OK);

    std::vector<SubscriberID> sidStart_v = broker_p->GetSubscriberIDVectorFromEventType(BVEventType::BVEVENTTYPE_TEST_REQUEST_START);
    std::vector<SubscriberID> sidPause_v = broker_p->GetSubscriberIDVectorFromEventType(BVEventType::BVEVENTTYPE_TEST_REQUEST_PAUSE);
    ASSERT_EQ(sidStart_v.size(), 0);
    ASSERT_EQ(sidPause_v.size(), 0);
}

TEST_F(BrokerBasicFixture, CheckUnsubscribingToMultipleEvents)
{
    // later
}

TEST_F(BrokerBasicFixture, CheckBasicRouting)
{
    boost::asio::io_context io_context;
    TestHeartbeatComponent tc{std::vector<BVEventType>{},
                                std::make_shared<threadsafe_queue<BVMessage>>(),
                                std::make_shared<threadsafe_queue<BVMessage>>(),
                                io_context,
                                0,
                                200};
    TestHeartbeatListenerComponent tcl{std::make_shared<threadsafe_queue<BVMessage>>(),
                                       std::make_shared<threadsafe_queue<BVMessage>>(),
                                       0};
    TCComponent tcComponent{std::make_shared<threadsafe_queue<BVMessage>>(),
                            std::make_shared<threadsafe_queue<BVMessage>>()};
    // We can subscribe to Broker. TestHeartbeatListenerComponent can send ack
    BVStatus attachStatusTc = broker_p->Attach(tc);
    BVStatus attachStatusTcl = broker_p->Attach(tcl);
    BVStatus attachStatusTcComponent = broker_p->Attach(tcComponent);
    ASSERT_EQ(attachStatusTcl, BVStatus::BVSTATUS_OK);

    // make sure that the TCComponent has corresponding callbacks for this messages!
    // Producer
    BVStatus subStatus1 = broker_p->Subscribe(tc.GetSubscriberId(), BVEventType::BVEVENTTYPE_TEST_REQUEST_SHUTDOWN);
    BVStatus subStatus2 = broker_p->Subscribe(tc.GetSubscriberId(), BVEventType::BVEVENTTYPE_TEST_REQUEST_START);
    BVStatus subStatus3 = broker_p->Subscribe(tc.GetSubscriberId(), BVEventType::BVEVENTTYPE_TEST_REQUEST_PAUSE);
    BVStatus subStatus4 = broker_p->Subscribe(tc.GetSubscriberId(), BVEventType::BVEVENTTYPE_TEST_REQUEST_RESUME);
    BVStatus subStatus5 = broker_p->Subscribe(tc.GetSubscriberId(), BVEventType::BVEVENTTYPE_TEST_REQUEST_RESTART);
    BVStatus subStatus6 = broker_p->Subscribe(tc.GetSubscriberId(), BVEventType::BVEVENTTYPE_TERMINATE_ALL);

    // Listener (Consumer)
    BVStatus subStatus7 = broker_p->Subscribe(tcl.GetSubscriberId(), BVEventType::BVEVENTTYPE_TEST_REQUEST_SHUTDOWN);
    BVStatus subStatus8 = broker_p->Subscribe(tcl.GetSubscriberId(), BVEventType::BVEVENTTYPE_TEST_REQUEST_START);
    BVStatus subStatus9 = broker_p->Subscribe(tcl.GetSubscriberId(), BVEventType::BVEVENTTYPE_TEST_REQUEST_PAUSE);
    BVStatus subStatus10 = broker_p->Subscribe(tcl.GetSubscriberId(), BVEventType::BVEVENTTYPE_TEST_REQUEST_RESUME);
    BVStatus subStatus11 = broker_p->Subscribe(tcl.GetSubscriberId(), BVEventType::BVEVENTTYPE_TEST_REQUEST_RESTART);
    BVStatus subStatus12 = broker_p->Subscribe(tcl.GetSubscriberId(), BVEventType::BVEVENTTYPE_TEST_HEARTBEAT);
    BVStatus subStatus13 = broker_p->Subscribe(tcl.GetSubscriberId(), BVEventType::BVEVENTTYPE_TERMINATE_ALL);

    // This thread (test) component
    BVStatus subStatus14 = broker_p->Subscribe(tcComponent.GetSubscriberId(), BVEventType::BVEVENTTYPE_TEST_REQUEST_SHUTDOWN);
    BVStatus subStatus15 = broker_p->Subscribe(tcComponent.GetSubscriberId(), BVEventType::BVEVENTTYPE_TEST_REQUEST_START);
    BVStatus subStatus16 = broker_p->Subscribe(tcComponent.GetSubscriberId(), BVEventType::BVEVENTTYPE_TEST_REQUEST_PAUSE);
    BVStatus subStatus17 = broker_p->Subscribe(tcComponent.GetSubscriberId(), BVEventType::BVEVENTTYPE_TEST_REQUEST_RESUME);
    BVStatus subStatus18 = broker_p->Subscribe(tcComponent.GetSubscriberId(), BVEventType::BVEVENTTYPE_TEST_REQUEST_RESTART);
    BVStatus subStatus19 = broker_p->Subscribe(tcComponent.GetSubscriberId(), BVEventType::BVEVENTTYPE_TEST_HEARTBEAT_ACK);
    BVStatus subStatus20 = broker_p->Subscribe(tcComponent.GetSubscriberId(), BVEventType::BVEVENTTYPE_TERMINATE_ALL);

    // TODO: ASSERT_EQ all above

    // start all
    broker_p->LaunchWorkerThread();
    tc.StartListeningOnMailbox();
    tc.LaunchWorkerThread();
    tcl.StartListeningOnMailbox();
    tcComponent.StartListeningOnMailbox();

    // wait for tc Component to have ack
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    ASSERT_EQ(BVStatus::BVSTATUS_OK, tcComponent.CheckAck()); 

    // send a terminate message
    broker_p->GetInMailBoxP()->push(
        BVMessage{BVEventType::BVEVENTTYPE_TERMINATE_ALL, nullptr});
    // please check if messages are really copied or just moved! how's the broadcast doing?

    // IMPORTANT TODO: What if the mailbox thread starts and joins the worker thread?
    // It is shutdown first. Then, the worker thread stops.

    // TODO: STANDARDIZE SHUTDOWN AND STARTUP PROCEDURE - At least try to summarize what needs to be done no matter the implementation
    // TODO: These components are not detached at the end. Is this a mistake?

    // join all
    tcComponent.TryJoinMailBoxThread();
    tc.TryJoinMailBoxThread();
    tc.JoinWorkerThread();
    tcl.TryJoinMailBoxThread();
    broker_p->TryJoinWorkerThread();
}

// This is checked when sending a terminate message
// TEST_F(BrokerBasicFixture, CheckTheSameMessageBeingDeliveredToMultipleSubscribers)
// {
//     // Utilize multiple TestHeartbeatComponent and TestHeartbeatListenerComponent
// }

TEST_F(BrokerBasicFixture, CheckMessageNotRoutedAfterUnsubscribing)
{
    TCComponent tcComponent{std::make_shared<threadsafe_queue<BVMessage>>(),
                            std::make_shared<threadsafe_queue<BVMessage>>()};
    
    BVStatus attachStatusTcComponent = broker_p->Attach(tcComponent);
    ASSERT_EQ(attachStatusTcComponent, BVStatus::BVSTATUS_OK);

    BVStatus subStatus = broker_p->Subscribe(tcComponent.GetSubscriberId(), BVEventType::BVEVENTTYPE_TEST_HEARTBEAT_ACK);
    BVStatus subStatus14 = broker_p->Subscribe(tcComponent.GetSubscriberId(), BVEventType::BVEVENTTYPE_TEST_REQUEST_SHUTDOWN);
    BVStatus subStatus20 = broker_p->Subscribe(tcComponent.GetSubscriberId(), BVEventType::BVEVENTTYPE_TERMINATE_ALL);
    ASSERT_EQ(subStatus, BVStatus::BVSTATUS_OK);
    ASSERT_EQ(BVStatus::BVSTATUS_NOK, tcComponent.CheckAck());

    // start all
    broker_p->LaunchWorkerThread();
    tcComponent.StartListeningOnMailbox();

    // unsubscribe
    BVStatus unSubStatus = broker_p->Unsubscribe(tcComponent.GetSubscriberId(), BVEventType::BVEVENTTYPE_TEST_HEARTBEAT_ACK);
    ASSERT_EQ(unSubStatus, BVStatus::BVSTATUS_OK);

    // send heartbeat_ack - route manually
    std::shared_ptr<BVMessage> msg_p = 
        std::make_shared<BVMessage>(
            BVEventType::BVEVENTTYPE_TEST_HEARTBEAT_ACK, nullptr);
    broker_p->GetInMailBoxP()->push(msg_p);
    std::this_thread::sleep_for(std::chrono::milliseconds(500)); // wait a bit

    // verify nothing's changed
    ASSERT_EQ(BVStatus::BVSTATUS_NOK, tcComponent.CheckAck());

    // Send termination message
    std::shared_ptr<BVMessage> tmsg_p = 
        std::make_shared<BVMessage>(
            BVEventType::BVEVENTTYPE_TERMINATE_ALL, nullptr);
    broker_p->GetInMailBoxP()->push(tmsg_p);

    // join all
    tcComponent.TryJoinMailBoxThread();
    broker_p->TryJoinWorkerThread();
}

TEST_F(BrokerBasicFixture, CheckMessageNotRoutedAfterDetaching)
{
    // 1. Attach Component
    // 2. Subscribe to a message type
    // 3. Route - verify successful routing
    // 4. Detach
    // 5. Route - verify message not being routed
    TCComponent tcComponent{std::make_shared<threadsafe_queue<BVMessage>>(),
                            std::make_shared<threadsafe_queue<BVMessage>>()};

    BVStatus attachStatusTcComponent = broker_p->Attach(tcComponent);
    BVStatus subStatus1 = broker_p->Subscribe(tcComponent.GetSubscriberId(), BVEventType::BVEVENTTYPE_TEST_REQUEST_SHUTDOWN);
    BVStatus subStatus2 = broker_p->Subscribe(tcComponent.GetSubscriberId(), BVEventType::BVEVENTTYPE_TEST_REQUEST_START);
    BVStatus subStatus3 = broker_p->Subscribe(tcComponent.GetSubscriberId(), BVEventType::BVEVENTTYPE_TEST_REQUEST_PAUSE);
    BVStatus subStatus4 = broker_p->Subscribe(tcComponent.GetSubscriberId(), BVEventType::BVEVENTTYPE_TEST_REQUEST_RESUME);
    BVStatus subStatus5 = broker_p->Subscribe(tcComponent.GetSubscriberId(), BVEventType::BVEVENTTYPE_TEST_REQUEST_RESTART);
    BVStatus subStatus6 = broker_p->Subscribe(tcComponent.GetSubscriberId(), BVEventType::BVEVENTTYPE_TEST_HEARTBEAT_ACK);
    BVStatus subStatus7 = broker_p->Subscribe(tcComponent.GetSubscriberId(), BVEventType::BVEVENTTYPE_TERMINATE_ALL);
    
    ASSERT_EQ(BVStatus::BVSTATUS_NOK, tcComponent.CheckAck());

    broker_p->LaunchWorkerThread();
    tcComponent.StartListeningOnMailbox();
    
    std::shared_ptr<BVMessage> msg_p = 
        std::make_shared<BVMessage>(
            BVEventType::BVEVENTTYPE_TEST_HEARTBEAT_ACK, nullptr);

    broker_p->GetInMailBoxP()->push(msg_p);
    std::this_thread::sleep_for(std::chrono::milliseconds(500)); // wait a bit
    ASSERT_EQ(BVStatus::BVSTATUS_OK, tcComponent.CheckAck()); // confirm working

    tcComponent.ResetAck();
    ASSERT_EQ(BVStatus::BVSTATUS_NOK, tcComponent.CheckAck());

    // Request shutdown IMPORTANT! Detaching is ALWAYS after shutdown.
    // It doesn't make sense that we detach a living component (we stop routing messages to it)
    // Although TCComponent doesn't have any worker threads, if there are present - they should be joined before detaching.
    std::shared_ptr<BVMessage> msg0_p = 
        std::make_shared<BVMessage>(
            BVEventType::BVEVENTTYPE_TEST_REQUEST_SHUTDOWN, nullptr); // this will stop TCComponent listening on mailbox
    broker_p->GetInMailBoxP()->push(msg0_p);
    std::this_thread::sleep_for(std::chrono::milliseconds(200)); // wait a bit

    // join TCComponent!
    tcComponent.TryJoinMailBoxThread();

    // Detach
    BVStatus dstatus = broker_p->Detach(tcComponent.GetSubscriberId());
    ASSERT_EQ(dstatus, BVStatus::BVSTATUS_OK);

    // Send message again
    std::shared_ptr<BVMessage> msg1_p = 
        std::make_shared<BVMessage>(
            BVEventType::BVEVENTTYPE_TEST_HEARTBEAT_ACK, nullptr);

    broker_p->GetInMailBoxP()->push(msg1_p);
    std::this_thread::sleep_for(std::chrono::milliseconds(200)); // wait a bit
    ASSERT_EQ(BVStatus::BVSTATUS_NOK, tcComponent.CheckAck());

    // Send termination message
    std::shared_ptr<BVMessage> tmsg_p = 
        std::make_shared<BVMessage>(
            BVEventType::BVEVENTTYPE_TERMINATE_ALL, nullptr);
    broker_p->GetInMailBoxP()->push(tmsg_p);

    // join broker
    broker_p->TryJoinWorkerThread();
}

// TEST_F(BrokerBasicFixture, CheckReceivingMessageNotSubscribedTo)
// {

// }

// TEST_F(BrokerBasicFixture, CheckComponentBeingDetachedMidAction)
// {
    // 1. Attach and Subscribe components:
    // 1a. TestHeartbeatComponent
    // 1b. TestHeartbeatListenerComponent
    // 1c. TCComponent
    //     These will perform operations in CheckBasicRouring for X times.
    // 2.  After X times, detach TestHeartbeatListenerComponent
    // 3. Verify no fault

    // SKIP THIS AND GO TO THE COMMUNICATION TESTS!
// }
