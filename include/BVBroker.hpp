#pragma once
#include "BV.hpp"
#include "BVEvent.hpp"
#include "BVMessage.hpp"
#include "BVSubscriber.hpp"
#include "threadsafequeue.hpp"
#include <vector>
#include <map>
#include <utility>

// typedef uint8_t SubscriberID;

// should BVBroker be a Component?
class BVComponent;
class BVBroker
{
private:
    std::map<SubscriberID, 
        std::shared_ptr<threadsafe_queue<BVMessage>> mailbox_m;
    std::map<BVEventType, std::vector<SubscriberID>> subs_m;
    std::shared_ptr<threadsafe_queue<BVMessage>> inMailBox_p;

    uint8_t numOfSubscribersRegistered;
    void Listen(void);
    BVStatus Stop(void);

    BVStatus SendMessage(const BVMessage message,
                         const std::shared_ptr<threadsafe_queue<BVMessage>> mailbox_p);
public:
    BVBroker(std::shared_ptr<threadsafe_queue<BVMessage>> _inMailBox_p);
    ~BVBroker();

    BVBroker(const BVBroker& other) = delete;
    BVBroker& operator=(const BVBroker& other) = delete;

    // Registers a subscriberID. Used at the setup.
    // This will be handy when TCPConnection, which is dynamically
    // created/deleted is implemented
    BVStatus Attach(BVComponent& component);
    // Remove this subID from map. OR remove the queue
    BVStatus Detach(const SubscriberID id);

    BVStatus Subscribe(const SubscriberID id, const BVEventType event);
    BVStatus Subscribe(const SubscriberID id, const std::vector<BVEventType> events_v);
    BVStatus Unsubscribe(const SubscriberID id, const BVEventType event);
    BVStatus Unsubscribe(const SubscriberID id, const std::vector<BVEventType> events_v);
};