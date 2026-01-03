#pragma once
#include <memory>
#include "threadsafequeue.hpp"
#include "BVMessage.hpp"
#include "BVSubscriber.hpp"
#include "BV.hpp"

class BVComponent
{
private:
    // this queue is shared with all components. Only to push!
    std::shared_ptr<threadsafe_queue<const BVMessage>> outMailBox_p;
    std::shared_ptr<threadsafe_queue<const BVMessage>> inMailBox_p;

    SubscriberID id;

    virtual BVStatus OnStart(void) = 0;
    virtual BVStatus OnShutdown(void) = 0;
    virtual BVStatus OnRestart(void) = 0;
    virtual BVStatus SendMessage(const BVMessage msg) noexcept
    {
        this->outMailBox_p->push(msg);
    }
public:

    BVComponent() {};
    virtual ~BVComponent() {};
};