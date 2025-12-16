#pragma once
#include <atomic>
#include <memory>
#include <mutex>
#include <vector>
#include <condition_variable>
#include <queue>
#include <algorithm>
#include <thread>
#include "BV.hpp"
#include "BVService_Bonjour.hpp"
#include "BVActor.hpp"

enum class BVAppEvent_e
{
    BVAPPEVENT_NEW_SERVICES
};
//?

/*
 * class BVApp
 * BVApp defines behavior and manages data related to
 * an application that user interacts with.
 * Behavior is simply a reaction to an event.
 * What event does a user do, that is abstract of a particular GUI lib.
 * They want to send a message, pause 
 *
*/
class BVApp
{
private:
    std::atomic<bool> isRunning = true;
    BVActor actor;
    std::thread listenerThread;

    // checking the serviceQueue
    // This can be implemented separately from any other
    // BVApp implementation - however it can be still overriden.
    // Stoppable? Probably will be, when isRunning = false
    // If we want to pause this and then resume we need a state machine.
    // For now - listening is until the app quits.
    // This is supposed to run on a separate worker thread
    // created by BVApp.

protected:
    std::vector<BVServiceBrowseInstance> serviceV; // iterable for services e.g. to display
    std::mutex serviceVectorMutex;
    // event queue?

    std::shared_ptr<std::queue<BVServiceBrowseInstance>> discoveryQueue;
    std::mutex& discoveryQueueMutex;
    std::condition_variable& discoveryQueueCV;
    bool& isDiscoveryQueueReady;

    std::queue<BVAppEvent_e> eventQueue;
    std::mutex eventQueueMutex;
    std::condition_variable eventQueueCV;

    virtual void ListenForUpcomingServices(void)
    {
        while (this->isRunning)
        {
            std::unique_lock lk(this->discoveryQueueMutex);
            discoveryQueueCV.wait(lk, [this]{return this->isDiscoveryQueueReady;}); // maybe only check on event from discovery component?

            // question is... will the Discovery_Bonjour component modify isDiscoveryQueueReady boolean?

            // funny thing -> we are now blocking the discovery component by app.
            // if app decides to block the serviceVectorMutex and doesn't release quickly, now discoveryQueueMutex is locked too.
            // if discovery component tries to update queue now, it cannot.
            // Also - when shutting down we have to make sure that neither of the threads
            // lock for eternity.
            // Instead of locking next time we can notify some different queue (send a message)
            std::unique_lock vlk(this->serviceVectorMutex);
            while (!discoveryQueue->empty())
            {
                BVServiceBrowseInstance bI = discoveryQueue->front();
                if (std::find(this->serviceV.begin(), this->serviceV.end(), bI) == serviceV.end())
                {
                    this->serviceV.push_back(bI);
                }
                discoveryQueue->pop(); // consume everything
            }

            // TODO: Another thread that's waiting on queue
            // Or -> make a flag, and put this before lk(this->discoveryQueueMutex), at the beginning
            // if updated => lock, push, unlock
            // now there are 3 locks in this place
            // Or better -> spawn thread which will print new services to the screen and utilize the method
            // for handling this event => BVAPPEVENT_NEW_SERVICES.

            // std::unique_lock elk(eventQueueMutex);
            // eventQueue.push(BVAppEvent_e::BVAPPEVENT_NEW_SERVICES); // do we need event queue? -> yes, but later
            // elk.unlock();

            vlk.unlock(); // main thread can now utilize vector
            this->isDiscoveryQueueReady = false; // meaning - it's not ready, nothing should be in there, queue empty
            lk.unlock();
            this->discoveryQueueCV.notify_one(); // now Discovery component can enter critical section
            this->HandleServicesDiscoveredUpdateEvent();
        }
    }

    void StartListenerThread(void)
    {
        this->listenerThread = std::thread([this](){this->ListenForUpcomingServices();});
    }

public:
    BVApp(std::shared_ptr<std::queue<BVServiceBrowseInstance>> _discoveryQueue,
          std::mutex& _discoveryQueueMutex,
          std::condition_variable& _discoveryQueueCV,
          bool& _isDiscoveryQueueReady) :
          discoveryQueue(_discoveryQueue),
          discoveryQueueMutex(_discoveryQueueMutex),
          discoveryQueueCV(_discoveryQueueCV),
          isDiscoveryQueueReady(_isDiscoveryQueueReady)
          {};

    virtual ~BVApp()
    {
        this->listenerThread.join();
    }

    virtual void Init(void) = 0;
    virtual void Run(void) = 0;
    virtual void Quit(void) = 0;

    virtual void HandleServicesDiscoveredUpdateEvent(void) = 0;
    virtual void HandleUserKeyboardInput(void) = 0;

    const std::atomic<bool>& GetIsRunning(void)
    {
        return this->isRunning;
    }

    void SetIsRunning(const bool state)
    {
        this->isRunning = state;
    }
};

// LC_Client will be a derivation of BVApp
// BVApp has method run => is a functional object that can be run.
// It takes a pointer to the queue.
// Can somehow Discovery class send something to update the std::vector of services? Or maybe it should have a different thread that waits (blocks) on a queue.
