#pragma once
#include "BV.hpp"
#include "BVEvent.hpp"
#include "BVMessage.hpp"
#include "BVSubscriber.hpp"
#include "threadsafequeue.hpp"
#include "BVComponent.hpp"
#include <vector>
#include <map>
#include <utility>

// typedef uint8_t SubscriberID;

// should BVBroker be a Component?
// class BVComponent;
class BVBroker
{
private:
    std::map<SubscriberID, 
        std::shared_ptr<threadsafe_queue<BVMessage>>> mailbox_m;
    std::map<BVEventType, std::vector<SubscriberID>> subs_m;
    std::shared_ptr<threadsafe_queue<BVMessage>> inMailBox_p;

    uint8_t numOfSubscribersRegistered = 0;
    SubscriberID currentSubscriberId = 0;

    bool isRunning = true;
    void Run(void);
    BVStatus Stop(void);

    // BVStatus SendMessage(BVMessage message,
                        //  const std::shared_ptr<threadsafe_queue<BVMessage >> mailbox_p);
public:
    BVBroker(std::shared_ptr<threadsafe_queue<BVMessage >> _inMailBox_p);
    ~BVBroker(){};

    BVBroker(const BVBroker& other) = delete;
    BVBroker& operator=(const BVBroker& other) = delete;

    // Registers a subscriberID. Used at the setup.
    // This will be handy when TCPConnection, which is dynamically
    // created/deleted is implemented
    [[nodiscard]] BVStatus Attach(BVComponent& component);
    // Remove this subID from map. OR remove the queue
    [[nodiscard]] BVStatus Detach(const SubscriberID id);

    [[nodiscard]] BVStatus Subscribe(const SubscriberID sid, const BVEventType event);
    [[nodiscard]] BVStatus Subscribe(const SubscriberID sid, const std::vector<BVEventType> events_v);
    [[nodiscard]] BVStatus Unsubscribe(const SubscriberID sid, const BVEventType event);
    [[nodiscard]] BVStatus Unsubscribe(const SubscriberID sid, const std::vector<BVEventType> events_v);

    void CycleCurrentSubscriberId(void);
};