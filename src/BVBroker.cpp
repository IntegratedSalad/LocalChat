#include "BVBroker.hpp"

BVBroker::BVBroker(std::shared_ptr<threadsafe_queue<BVMessage >> _inMailBox_p)
{
    this->inMailBox_p = _inMailBox_p;
    for (int i = 0; i < UINT8_MAX; i++)
    {
        this->mailbox_m.emplace(std::make_pair(i, nullptr));
    }
}

void BVBroker::Run()
{
    while (this->isRunning)
    {
        std::shared_ptr<BVMessage> msg_p = this->inMailBox_p->wait_and_pop();
        if (!msg_p)
        {
            // should not 
            continue;
        }
        if (msg_p->event_t == BVEventType::BVEVENTTYPE_TERMINATE_ALL)
        {
            this->isRunning = false;
        }
        this->Route(msg_p);
    }
}

// Maybe Broker should be just an object having shared pointer to mailboxes.
// If Broker is put in a separate thread, we need to synchronize access, which
// is probably redundant, as this is just a simple router msg -> mailbox
BVStatus BVBroker::Route(const std::shared_ptr<BVMessage>& msg_p)
{
    const BVEventType etype = msg_p->event_t;
    auto it = subs_m.find(etype);
    if (it == subs_m.end() || it->second.empty())
        return BVStatus::BVSTATUS_OK; // BVNO_SUBSCRIBERS?
        // BVSTATUS_NOK

    const auto& subscribers_v = it->second; // no copy
    for (SubscriberID sid : subscribers_v)
    {
        auto mit = mailbox_m.find(sid);
        if (mit != mailbox_m.end() && mit->second)
            mit->second->push(msg_p);
        // else: stale subscription
    }
    return BVStatus::BVSTATUS_OK;
}

BVStatus BVBroker::Stop()
{
    for (auto& [sid, q] : mailbox_m)
        q.reset();

    mailbox_m.clear();
    subs_m.clear();

    // Consider NOT resetting inMailBox_p here; or reset only after
    // all components are stopped/joined.
    // inMailBox_p.reset();

    return BVStatus::BVSTATUS_OK;
}

// Every component sends msgs to the broker, so
// sets this mailbox as their out box.
// but the Broker ISN'T put in a separate thread.
// It just holds pointers to the mailboxes of every component that
// is active and listens
BVStatus BVBroker::Attach(BVComponent& component)
{
    if (numOfSubscribersRegistered >= UINT8_MAX + 1u) // if you allow 256
        return BVStatus::BVSTATUS_MAX_COMPONENTS;

    // ensure currentSubscriberId points to a free slot
    if (mailbox_m.find(currentSubscriberId) != mailbox_m.end() &&
        mailbox_m[currentSubscriberId] != nullptr)
    {
        if (!CycleCurrentSubscriberId())
            return BVStatus::BVSTATUS_MAX_COMPONENTS;
    }

    component.SetSubscriberId(currentSubscriberId);
    // their in mail box is our mail box at their SubscriberID
    // their out mail box is our in mail box
    component.SetOutMailBox(inMailBox_p);

    auto q = std::make_shared<threadsafe_queue<BVMessage>>();
    mailbox_m[currentSubscriberId] = q;
    component.SetInMailBox(q);

    ++numOfSubscribersRegistered;

    // prepare next id for the *next* Attach, but don’t fail this Attach
    CycleCurrentSubscriberId();

    return BVStatus::BVSTATUS_OK;
}

// This should be done ALWAYS after terminating the component
// It doesn't make sense that we detach a living component (we stop routing messages to it)
BVStatus BVBroker::Detach(const SubscriberID sid)
{
    auto it = mailbox_m.find(sid);
    if (it == mailbox_m.end() || !it->second)
        return BVStatus::BVSTATUS_FATAL_ERROR;

    it->second.reset();
    --numOfSubscribersRegistered;

    // remove sid from all events
    for (auto iter = subs_m.begin(); iter != subs_m.end(); )
    {
        auto& vec = iter->second;
        vec.erase(std::remove(vec.begin(), vec.end(), sid), vec.end());
        if (vec.empty()) iter = subs_m.erase(iter);
        else ++iter;
    }

    return BVStatus::BVSTATUS_OK;
}

BVStatus BVBroker::Subscribe(const SubscriberID sid, const BVEventType event)
{
    auto mit = mailbox_m.find(sid);
    if (mit == mailbox_m.end() || !mit->second)
        return BVStatus::BVSTATUS_FATAL_ERROR;

    auto& vec = subs_m[event];

    if (std::find(vec.begin(), vec.end(), sid) == vec.end())
        vec.push_back(sid);

    return BVStatus::BVSTATUS_OK;
}

BVStatus BVBroker::Subscribe(const SubscriberID sid, const std::vector<BVEventType>& events_v)
{
    auto mit = mailbox_m.find(sid);
    if (mit == mailbox_m.end() || !mit->second)
        return BVStatus::BVSTATUS_FATAL_ERROR;

    for (const auto& ev : events_v)
    {
        auto& vec = subs_m[ev];
        if (std::find(vec.begin(), vec.end(), sid) == vec.end())
            vec.push_back(sid);
    }

    return BVStatus::BVSTATUS_OK;
}

BVStatus BVBroker::Unsubscribe(const SubscriberID sid, const BVEventType event)
{
    auto it = subs_m.find(event);
    if (it == subs_m.end())
        return BVStatus::BVSTATUS_OK;

    auto& vec = it->second;
    vec.erase(std::remove(vec.begin(), vec.end(), sid), vec.end());

    if (vec.empty())
        subs_m.erase(it);

    return BVStatus::BVSTATUS_OK;
}

BVStatus BVBroker::Unsubscribe(const SubscriberID sid, const std::vector<BVEventType>& events_v)
{
    for (const auto& ev : events_v)
    {
        auto it = subs_m.find(ev);
        if (it == subs_m.end())
            continue;

        auto& vec = it->second;
        vec.erase(std::remove(vec.begin(), vec.end(), sid), vec.end());

        if (vec.empty())
            subs_m.erase(it);
    }

    return BVStatus::BVSTATUS_OK;
}
// Cycle through mailbox to find a subscriber id with nullptr mailbox
// (the one that haven't had any mailbox, or their mailbox was freed).
bool BVBroker::CycleCurrentSubscriberId()
{
    const uint8_t start = currentSubscriberId;
    for (uint16_t i = 0; i <= UINT8_MAX; ++i)
    {
        currentSubscriberId =
            static_cast<SubscriberID>((start + 1 + i) % (UINT8_MAX + 1));
        if (mailbox_m[currentSubscriberId] == nullptr)
        {
            return true;
        }
    }
    return false;
}
