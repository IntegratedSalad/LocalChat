#pragma once
#include <memory>
#include "threadsafequeue.hpp"
#include "BVMessage.hpp"
#include "BVSubscriber.hpp"
#include "BV.hpp"

class BVComponent
{
private:
    // This queue is shared with all components and the broker. Only to push to!
    std::shared_ptr<threadsafe_queue<BVMessage>> outMailBox_p;

    // This queue is shared with the broker. Only to read from!
    std::shared_ptr<threadsafe_queue<BVMessage>> inMailBox_p;

    SubscriberID id;

    virtual BVStatus OnStart(void) = 0;
    virtual BVStatus OnShutdown(void) = 0;
    virtual BVStatus OnRestart(void) = 0;
    virtual BVStatus SendMessage(BVMessage msg) noexcept
    {
        this->outMailBox_p->push(std::move(msg));
        return BVStatus::BVSTATUS_OK;
    }
public:
    BVComponent() {};
    virtual ~BVComponent() {};

    // These are initialized when attaching to a broker

    void SetId(const SubscriberID _id)
    {
        this->id = _id;
    }

    void SetOutMailBox(std::shared_ptr<threadsafe_queue<BVMessage>> _outMailBox)
    {
        this->outMailBox_p = _outMailBox;
    }

    void SetInMailBox(std::shared_ptr<threadsafe_queue<BVMessage>> _inMailBox_p)
    {
        this->inMailBox_p = _inMailBox_p;
    }
};