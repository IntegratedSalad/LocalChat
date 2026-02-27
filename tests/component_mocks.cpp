#include "component_mocks.hpp"
#include <any>

MockDiscovery::MockDiscovery(const BVServiceHostData _hostData,
                std::mutex& _discoveryQueueMutex,
                boost::asio::io_context& _ioContext,
                std::shared_ptr<std::queue<BVServiceBrowseInstance>> _discoveryQueue,
                std::condition_variable& _discoveryQueueCV,
                bool& _isDiscoveryQueueReady,
                std::shared_ptr<threadsafe_queue<BVMessage>> _outMbx,
                std::shared_ptr<threadsafe_queue<BVMessage>> _inMbx) :
    ioContext(_ioContext),
    discoveryTimer(_ioContext),
    BVDiscovery(_hostData, 
                _discoveryQueueMutex,
                _discoveryQueue,
                _discoveryQueueCV,
                _isDiscoveryQueueReady),
    BVComponent(_outMbx, _inMbx)
{
    // // RegisterCallbacks for interesting events to Discovery
    // RegisterCallback(BVEventType::..., 
    //     [this](std::unique_ptr<std::any>) -> BVStatus {
    //         this->SetIsBrowsingActive(false);
    //         return BVStatus::BVSTATUS_OK;
    // }); 

    RegisterCallback(BVEventType::BVEVENTTYPE_SERVICE_REQUEST_START,
                     std::bind(&MockDiscovery::OnStart, this, std::placeholders::_1));

    RegisterCallback(BVEventType::BVEVENTTYPE_DISCOVERY_REQUEST_PAUSE, 
                     std::bind(&MockDiscovery::OnPause, this, std::placeholders::_1));

    RegisterCallback(BVEventType::BVEVENTTYPE_TERMINATE_ALL,
                     std::bind(&MockDiscovery::OnShutdown, this, std::placeholders::_1));

    RegisterCallback(BVEventType::BVEVENTTYPE_SERVICE_REQUEST_SHUTDOWN,
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
void MockDiscovery::RunNTimes(const int n)
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
void MockDiscovery::RunNTimesWithKElements(const int n, const int k)
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

void MockDiscovery::RunContinuously(void) // this is run in the main worker thread
{
    this->SetIsBrowsingActive(true);
    run();
}

void MockDiscovery::run(void) // this is run in the main worker thread
{
    // Append one Service
    // LinkedListElement_str* lle1_p = LinkedListElement_str_Constructor((char*)"TESTSERVICE1", NULL);
    // LinkedList_str_AddElement(this->GetLinkedList_p(), lle1_p);
    // PushBrowsedServicesToQueue();
    // SendMessage(BVMessage(
    //                 BVEventType::BVEVENTTYPE_APP_PUBLISHED_SERVICE, nullptr));

    // // TODO: Important: can we just send a list now??? We don't need the synchronization on discoveryQueue
    // // between app and discovery components, if we attach the list in message!

    // // Yes, this is a next thing to focus on. Because the previous
    // // async model hasn't have defined a communication method (message passing now), App and Discovery communicated on
    // // a shared, non-threadsafe queue. Now we can just send a list of discovery results over mailbox (broker passes)
    // // message about a topic that App subscribed to 'BVEVENTTYPE_APP_PUBLISHED_SERVICE'
    // LinkedList_str_ClearList(this->GetLinkedList_p());

    // // TODO: ^ do entire linked_list putting and pushing to queue, and notifying the App component
    // // Probably we have to create a test abstraction for the App component, as it is not connected
    // // in functional tests (or just append to queue and consume outside in the mocked app thread)

    // // Wait 2 s
    // std::this_thread::sleep_for(std::chrono::seconds(2));

    // // Append three Services
    // LinkedListElement_str* lle2_p = LinkedListElement_str_Constructor((char*)"TESTSERVICE2", NULL);
    // LinkedListElement_str* lle3_p = LinkedListElement_str_Constructor((char*)"TESTSERVICE3", NULL);
    // LinkedListElement_str* lle4_p = LinkedListElement_str_Constructor((char*)"TESTSERVICE4", NULL);
    // LinkedList_str_AddElement(this->GetLinkedList_p(), lle2_p);
    // LinkedList_str_AddElement(this->GetLinkedList_p(), lle3_p);
    // LinkedList_str_AddElement(this->GetLinkedList_p(), lle4_p);

    // // This should be embedded with announcing to the broker that we pushed a service
    // // at least for now remember that this is tied to an event!
    // PushBrowsedServicesToQueue();
    // SendMessage(BVMessage(
    //                 BVEventType::BVEVENTTYPE_APP_PUBLISHED_SERVICE, nullptr));
    // LinkedList_str_ClearList(this->GetLinkedList_p());

    // // Wait 2 s
    // std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // // Append four Services
    // LinkedListElement_str* lle5_p = LinkedListElement_str_Constructor((char*)"TESTSERVICE5", NULL);
    // LinkedListElement_str* lle6_p = LinkedListElement_str_Constructor((char*)"TESTSERVICE6", NULL);
    // LinkedListElement_str* lle7_p = LinkedListElement_str_Constructor((char*)"TESTSERVICE7", NULL);
    // LinkedListElement_str* lle8_p = LinkedListElement_str_Constructor((char*)"TESTSERVICE8", NULL);
    // LinkedList_str_AddElement(this->GetLinkedList_p(), lle5_p);
    // LinkedList_str_AddElement(this->GetLinkedList_p(), lle6_p);
    // LinkedList_str_AddElement(this->GetLinkedList_p(), lle7_p);
    // LinkedList_str_AddElement(this->GetLinkedList_p(), lle8_p);
    // PushBrowsedServicesToQueue();
    // SendMessage(BVMessage(
    //                 BVEventType::BVEVENTTYPE_APP_PUBLISHED_SERVICE, nullptr));
    // LinkedList_str_ClearList(this->GetLinkedList_p());

    // // Wait 2 s
    // std::this_thread::sleep_for(std::chrono::seconds(2));
    using BVServiceBrowseInstanceList = std::list<BVServiceBrowseInstance>;

    int serviceNum = 0;

    while (this->GetIsBrowsingActive())
    {
        // wait 3 s - simulate waiting for daemon response
        std::this_thread::sleep_for(std::chrono::seconds(3));

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

BVStatus MockDiscovery::OnPause(std::unique_ptr<std::any> dp)
{
    return BVStatus::BVSTATUS_OK;
}

BVStatus MockDiscovery::OnStart(std::unique_ptr<std::any> dp) // test passing dp
{
    Setup();
    return BVStatus::BVSTATUS_OK;
}

BVStatus MockDiscovery::OnShutdown(std::unique_ptr<std::any>)
{
    // TODO: Think through what the MockDiscovery implementation will be doing to mock DNS-SD functionality
    // and write a queue push procedure

    // Remember that this is still mailbox thread if it is called from within mailbox thread!
    this->SetIsBrowsingActive(false);
    this->SetIsListeningToMail(false);
    return BVStatus::BVSTATUS_OK;
}

BVStatus MockDiscovery::OnRestart(std::unique_ptr<std::any>)
{
    // clear everything and start
}

// void AddServiceToList(LinkedList_str* ll_p, const char* str)
// {

// }
