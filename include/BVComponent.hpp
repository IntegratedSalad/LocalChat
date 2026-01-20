#pragma once
#include <memory>
#include <atomic>
#include <functional>
#include <unordered_map>
#include <thread>
#include "threadsafequeue.hpp"
#include "BVMessage.hpp"
#include "BVSubscriber.hpp"
#include "BV.hpp"

class BVComponent
{
private:
    using MessageHandler = std::function<BVStatus(std::unique_ptr<std::any>)>;

    // This queue is shared with all components and the broker. Only to push to!
    std::shared_ptr<threadsafe_queue<BVMessage>> outMailBox_p;

    // This queue is shared with the broker. Only to read from!
    std::shared_ptr<threadsafe_queue<BVMessage>> inMailBox_p;

    std::unordered_map<BVEventType, MessageHandler> callback_m;

    std::thread mailbox_thread;
    std::atomic_bool isListeningToMail = false;

    SubscriberID id;

    // Callbacks
    virtual BVStatus OnStart(std::unique_ptr<std::any>) = 0;
    virtual BVStatus OnShutdown(std::unique_ptr<std::any>) = 0;
    virtual BVStatus OnRestart(std::unique_ptr<std::any>) = 0;

    // BVStatus React(void); // Check State and react

    // FSM->TryChangeState(State), but for now without FSM
    void ListenToMail(void)
    {
        // Check inMailBox_p
        // If there's a message, Dispatch a handler
        while (isListeningToMail)
        {
            if (!inMailBox_p->empty())
            {
                std::shared_ptr<BVMessage> msg = inMailBox_p->try_pop();
                try
                {
                    callback_m.at(msg->event_t);
                }
                catch(const std::out_of_range& ex)
                {
                    std::cerr << "[BVComponent]::ListenToMail, mailbox_thread for " 
                     << " out_of_range::what(): " << ex.what() << std::endl;
                     exit(-1);
                }
                callback_m[msg->event_t](std::move(msg->data_p));
            }
            // if shutdown: isListeningToMail = false -> main thread will join
        }
    }

protected:
    void RegisterCallback(BVEventType event_type, MessageHandler msg_handler)
    {
        this->callback_m[event_type] = msg_handler;
    }

    virtual BVStatus SendMessage(BVMessage&& msg) noexcept
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