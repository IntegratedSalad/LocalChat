#include "BVBroker.hpp"

BVBroker::BVBroker(std::shared_ptr<threadsafe_queue<BVMessage >> _inMailBox_p)
{
    this->inMailBox_p = _inMailBox_p;
    for (int i = 0; i < UINT8_MAX; i++)
    {
        this->mailbox_m.emplace(std::make_pair(i, nullptr));
    }
}

void BVBroker::Run(void)
{
    while (this->isRunning)
    {
       std::shared_ptr<BVMessage> message_p = this->inMailBox_p->try_pop();
       const BVEventType etype = message_p->event_t;
       if (etype == BVEventType::BVEVENTTYPE_TERMINATE_ALL)
       {
           this->isRunning = false;
       }
       // everyone should be subscribed to BVEVENTTYPE_TERMINATE_ALL
       const std::vector<SubscriberID> subscribers = subs_m[etype];
       for (auto& sub : subscribers)
       {
           mailbox_m[sub]->push(message_p);
       }
    }
}

BVStatus BVBroker::Stop(void)
{
    for (const auto& [k, v] : this->mailbox_m)
    {
        this->mailbox_m[k] = nullptr;
    }
    this->inMailBox_p = nullptr;
    return BVStatus::BVSTATUS_OK;
}

// // Needed?
// BVStatus BVBroker::SendMessage(BVMessage message,
//                                const std::shared_ptr<threadsafe_queue<BVMessage >> mailbox_p)
// {

// }

BVStatus BVBroker::Attach(BVComponent& component)
{
    if (numOfSubscribersRegistered == UINT8_MAX)
    {
        return BVStatus::BVSTATUS_MAX_COMPONENTS;
    }
    component.SetId(currentSubscriberId);

    // their out mail box is our in mail box
    component.SetOutMailBox(this->inMailBox_p);

    // their in mail box is our mail box at their SubscriberID
    this->mailbox_m[currentSubscriberId] = 
        std::make_shared<threadsafe_queue<BVMessage>>();
    component.SetInMailBox(this->mailbox_m[currentSubscriberId]);

    numOfSubscribersRegistered += 1;
    this->CycleCurrentSubscriberId();
    return BVStatus::BVSTATUS_OK;
}

BVStatus BVBroker::Detach(const SubscriberID sid)
{
    try
    {
        this->mailbox_m.at(sid);
    } catch (const std::out_of_range& ex)
    {
        std::cerr << "[BVBroker]::Detach for " << sid 
                  << " out_of_range::what(): " << ex.what() << std::endl;
        return BVStatus::BVSTATUS_FATAL_ERROR;
    }
    if (this->mailbox_m.at(sid) == nullptr)
    {
        return BVStatus::BVSTATUS_NOK;
    }
    this->mailbox_m[sid] = nullptr;
    numOfSubscribersRegistered -= 1;
    return BVStatus::BVSTATUS_OK;
}

BVStatus BVBroker::Subscribe(const SubscriberID sid, const BVEventType event)
{
    try
    {
        this->subs_m.at(event);
    } catch (const std::out_of_range& ex)
    {
        std::cerr << "[BVBroker]::Subscribe for " << sid 
                  << " out_of_range::what(): "    << ex.what() << std::endl;
        return BVStatus::BVSTATUS_FATAL_ERROR;
    }
    this->subs_m[event].push_back(sid);
    return BVStatus::BVSTATUS_OK;
}

BVStatus BVBroker::Subscribe(const SubscriberID sid, const std::vector<BVEventType> events_v)
{
    for (const auto& ev : events_v)
    {
        subs_m[ev].push_back(sid); 
    }
    return BVStatus::BVSTATUS_OK;
}

BVStatus BVBroker::Unsubscribe(const SubscriberID sid, const BVEventType event)
{
    auto& v = subs_m[event];
    v.erase(std::remove(v.begin(), v.end(), sid), v.end());
    return BVStatus::BVSTATUS_OK;
}

BVStatus BVBroker::Unsubscribe(const SubscriberID sid, const std::vector<BVEventType> events_v)
{
    for (const auto& ev : events_v)
    {
        auto& v = subs_m[ev];
        v.erase(std::remove(v.begin(), v.end(), sid), v.end());
    }
    return BVStatus::BVSTATUS_OK;
}

// Cycle through mailbox to find a subscriber id with nullptr mailbox
// (the one that haven't had any mailbox, or their mailbox was freed).
void BVBroker::CycleCurrentSubscriberId(void)
{
    while (this->mailbox_m[
          (currentSubscriberId += 1)%UINT8_MAX] != nullptr);
}
