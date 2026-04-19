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

    // This queue is shared with the broker. Only to push to!
    std::shared_ptr<threadsafe_queue<BVMessage>> outMailBox_p;

    // This queue is shared with the broker. Only to read from!
    std::shared_ptr<threadsafe_queue<BVMessage>> inMailBox_p;

    std::unordered_map<BVEventType, MessageHandler> callback_m;

    std::thread mailbox_thread;
    std::atomic_bool isListeningToMail = false;

    SubscriberID id;

    virtual BVStatus OnStart(std::unique_ptr<std::any>) = 0;
    virtual BVStatus OnPause(std::unique_ptr<std::any>) = 0;
    virtual BVStatus OnResume(std::unique_ptr<std::any>) = 0;
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
            // think of a way that would force terminate a thread
            // when hanging here
            std::shared_ptr<BVMessage> msg = inMailBox_p->wait_and_pop();
            try
            {
                callback_m.at(msg->event_t);
            }
            catch(const std::out_of_range& ex) // TODO: Maybe log this too...
            {
                std::cerr << "[BVComponent]::ListenToMail, mailbox_thread for " 
                            << "out_of_range::what(): " << ex.what() << std::endl;
                std::cerr << "[BVComponent]::ListenToMail, probably no handler for a message!"
                            << std::endl;
                const std::string err_s("No callback to handle event for Subscriber ID: ", id);
                throw std::runtime_error(err_s);
            }
            const BVStatus callback_status = callback_m[msg->event_t](std::move(msg->data_p));
            if (callback_status != BVStatus::BVSTATUS_OK) 
            {
                const std::string err_s("Callback failed with Subscriber ID: " + std::to_string(id));
                throw std::runtime_error(err_s);
            }
            // if shutdown: isListeningToMail = false -> main thread will join
            if (msg->event_t == BVEventType::BVEVENTTYPE_TERMINATE_ALL || 
                msg->event_t == BVEventType::BVEVENTTYPE_TEST_REQUEST_SHUTDOWN) // !! TODO: || or other
            {
                return;
            }
        }
    }

protected:
    virtual BVStatus SendMessage(BVMessage&& msg) noexcept
    {
        this->outMailBox_p->push(std::move(msg));
        return BVStatus::BVSTATUS_OK;
    }

public:
    BVComponent(std::shared_ptr<threadsafe_queue<BVMessage>> _outMbx,
                std::shared_ptr<threadsafe_queue<BVMessage>> _inMbx )
                : outMailBox_p(_outMbx), inMailBox_p(_inMbx) {};

    virtual ~BVComponent() {};

    void RegisterCallback(BVEventType event_type, MessageHandler msg_handler)
    {
        this->callback_m[event_type] = msg_handler;
    }

    void StartListeningOnMailbox(void)
    {
        this->isListeningToMail = true;
        mailbox_thread = std::thread([this] {
            this->ListenToMail();
        });
    }

    std::thread& GetMailBoxThread(void)
    {
        return this->mailbox_thread;
    }

    void TryJoinMailBoxThread(void)
    {
        if (this->GetMailBoxThread().joinable())
        {
            this->GetMailBoxThread().join();
        }
    }

    bool GetIsListeningToMail(void)
    {
        return this->isListeningToMail;
    }

    void SetIsListeningToMail(const bool val)
    {
        this->isListeningToMail = val;
    }

    // These are initialized when attaching to a broker
    void SetSubscriberId(const SubscriberID _id)
    {
        this->id = _id;
    }

    SubscriberID GetSubscriberId(void) const
    {
        return this->id;
    }

    // TODO: Functions to get pointers to mailbox queues.
    //       This will allow components to get a redirection point
    //       and eliminate the need for instantiating another broker/making component
    //       listen to multiple brokers.
    //       This is utilized in BVApp, when we do not pass the Broker but want to share 

    void SetOutMailBox(std::shared_ptr<threadsafe_queue<BVMessage>> _outMailBox)
    {
        this->outMailBox_p = _outMailBox;
    }

    void SetInMailBox(std::shared_ptr<threadsafe_queue<BVMessage>> _inMailBox_p)
    {
        this->inMailBox_p = _inMailBox_p;
    }
};