#include "component_mocks.hpp"

MockDiscovery::MockDiscovery(const BVServiceHostData _hostData,
                std::mutex& _discoveryQueueMutex,
                boost::asio::io_context& _ioContext,
                std::shared_ptr<std::queue<BVServiceBrowseInstance>> _discoveryQueue,
                std::condition_variable& _discoveryQueueCV,
                bool& _isDiscoveryQueueReady) :
    ioContext(_ioContext),
    discoveryTimer(_ioContext),
    BVDiscovery(_hostData, 
                _discoveryQueueMutex,
                _discoveryQueue,
                _discoveryQueueCV,
                _isDiscoveryQueueReady)
{
    // // RegisterCallbacks for interesting events to Discovery
    // RegisterCallback(BVEventType::..., 
    //     [this](std::unique_ptr<std::any>) -> BVStatus {
    //         this->SetIsBrowsingActive(false);
    //         return BVStatus::BVSTATUS_OK;
    // }); 

    RegisterCallback(BVEventType::BVEVENTTYPE_DISCOVERY_REQUEST_SHUTDOWN, 
                     std::bind(&MockDiscovery::OnShutdown, this, std::placeholders::_1));

    RegisterCallback(BVEventType::BVEVENTTYPE_SERVICE_REQUEST_START,
                     std::bind(&MockDiscovery::OnStart, this, std::placeholders::_1));
}

MockDiscovery::~MockDiscovery()
{

}

void MockDiscovery::CreateConnectionContext(void)
{
    this->isConnectionContextAlive = true;
}

void MockDiscovery::Setup(void)
{

}

void MockDiscovery::run(void) // this is run in the main worker thread
{
    // TODO: Think through what the MockDiscovery implementation will be doing to mock DNS-SD functionality

    // Append one Service
    LinkedListElement_str* lle1_p = LinkedListElement_str_Constructor((char*)"TESTSERVICE1", NULL);
    LinkedList_str_AddElement(this->GetLinkedList_p(), lle1_p);
    PushBrowsedServicesToQueue();
    SendMessage(BVMessage(
                    BVEventType::BVEVENTTYPE_DISCOVERY_PUBLISHED_SERVICE, nullptr));

    // TODO: Important: can we just send a list now??? We don't need the synchronization on discoveryQueue
    // between app and discovery components, if we attach the list in message!
    LinkedList_str_ClearList(this->GetLinkedList_p());

    // TODO: ^ do entire linked_list putting and pushing to queue, and notifying the App component
    // Probably we have to create a test abstraction for the App component, as it is not connected
    // in functional tests (or just append to queue and consume outside in the mocked app thread)

    // Wait 2 s
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Append three Services
    LinkedListElement_str* lle2_p = LinkedListElement_str_Constructor((char*)"TESTSERVICE2", NULL);
    LinkedListElement_str* lle3_p = LinkedListElement_str_Constructor((char*)"TESTSERVICE3", NULL);
    LinkedListElement_str* lle4_p = LinkedListElement_str_Constructor((char*)"TESTSERVICE4", NULL);
    LinkedList_str_AddElement(this->GetLinkedList_p(), lle2_p);
    LinkedList_str_AddElement(this->GetLinkedList_p(), lle3_p);
    LinkedList_str_AddElement(this->GetLinkedList_p(), lle4_p);

    // This should be embedded with announcing to the broker that we pushed a service
    // at least for now remember that this is tied to an event!
    PushBrowsedServicesToQueue();
    SendMessage(BVMessage(
                    BVEventType::BVEVENTTYPE_DISCOVERY_PUBLISHED_SERVICE, nullptr));
    LinkedList_str_ClearList(this->GetLinkedList_p());

    // Wait 2 s
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // Append four Services
    LinkedListElement_str* lle5_p = LinkedListElement_str_Constructor((char*)"TESTSERVICE5", NULL);
    LinkedListElement_str* lle6_p = LinkedListElement_str_Constructor((char*)"TESTSERVICE6", NULL);
    LinkedListElement_str* lle7_p = LinkedListElement_str_Constructor((char*)"TESTSERVICE7", NULL);
    LinkedListElement_str* lle8_p = LinkedListElement_str_Constructor((char*)"TESTSERVICE8", NULL);
    LinkedList_str_AddElement(this->GetLinkedList_p(), lle5_p);
    LinkedList_str_AddElement(this->GetLinkedList_p(), lle6_p);
    LinkedList_str_AddElement(this->GetLinkedList_p(), lle7_p);
    LinkedList_str_AddElement(this->GetLinkedList_p(), lle8_p);
    PushBrowsedServicesToQueue();
    SendMessage(BVMessage(
                    BVEventType::BVEVENTTYPE_DISCOVERY_PUBLISHED_SERVICE, nullptr));
    LinkedList_str_ClearList(this->GetLinkedList_p());

    // Wait 2 s
    std::this_thread::sleep_for(std::chrono::seconds(2));

    int serviceNum = 0;
    while (this->GetIsBrowsingActive())
    {
        // wait 10 s 
        std::this_thread::sleep_for(std::chrono::seconds(10));
        char str[4+11] = "TESTSERVICE";
        str[14] = '\0';
        char numstr[4];
        numstr[3] = '\0';
        snprintf(numstr, 3, "%d", serviceNum);
        strncat(str, numstr, 3);
        LinkedListElement_str* llen_p = LinkedListElement_str_Constructor(numstr, NULL);
        LinkedList_str_AddElement(this->GetLinkedList_p(), llen_p);
        PushBrowsedServicesToQueue();
        SendMessage(BVMessage(
                        BVEventType::BVEVENTTYPE_DISCOVERY_PUBLISHED_SERVICE, nullptr));
        LinkedList_str_ClearList(this->GetLinkedList_p());
        serviceNum++;
    }
}

BVStatus MockDiscovery::OnStart(std::unique_ptr<std::any> dp) // test passing dp
{
    this->SetIsBrowsingActive(true);
}

BVStatus MockDiscovery::OnShutdown(std::unique_ptr<std::any>)
{
    // TODO: Think through what the MockDiscovery implementation will be doing to mock DNS-SD functionality
    // and write a queue push procedure

    this->SetIsBrowsingActive(false);
    //Get MailBoxThread  // this->mailbox_thread.join(); // when not listening to mailbox, thread should terminate naturally
}

BVStatus MockDiscovery::OnRestart(std::unique_ptr<std::any>)
{
}

// void AddServiceToList(LinkedList_str* ll_p, const char* str)
// {

// }
