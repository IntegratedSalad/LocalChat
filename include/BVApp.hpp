#pragma once
#include <memory>
#include <vector>
#include <condition_variable>
#include <queue>
#include "BV.hpp"
#include "BVService_Bonjour.hpp"
#include "BVActor.hpp"

// enum class BVAppEvent_e
// {
// };
//?

/*
 * class BVApp
 * BVApp defines behavior and manages data related to
 * an application that user interacts with.
 * Behavior is simply a reaction to an event.
 * What event does a user do, that is abstract of a particular GUI lib?
 * It wants to send a message
 *
*/
class BVApp
{
private:
    bool isRunning = true;
    BVActor actor;
    std::shared_ptr<std::queue<BVServiceBrowseInstance>> discoveryQueue;
    std::vector<BVService> serviceV; // iterable for services e.g. to display
    // event queue?

    std::mutex& discoveryQueueMutex;
    std::condition_variable& discoveryQueueCV;

     // checking the serviceQueue
    virtual void ListenForUpcomingServices(void)
    {
    }

public:
    BVApp(std::shared_ptr<std::queue<BVServiceBrowseInstance>> _discoveryQueue,
          std::mutex& _discoveryQueueMutex,
          std::condition_variable& _discoveryQueueCV) :
          discoveryQueue(_discoveryQueue), discoveryQueueMutex(_discoveryQueueMutex), discoveryQueueCV(_discoveryQueueCV) {};

    virtual void Init(void) = 0;
    virtual void Run(void) = 0;
    virtual void Quit(void) = 0;

    virtual void HandleServicesDiscoveredUpdateEvent(void) = 0;
    virtual void HandleUserKeyboardInput(void) = 0;

    // virtual destructor??

    // void operator()(void) // TODO: maybe it shouldn't really be a function object
    // {
    //     this->Init();
    //     this->Run();
    // }
};

// LC_Client will be a derivation of BVApp
// BVApp has method run => is a functional object that can be run.
// It takes a pointer to the queue.
// Can somehow Discovery class send something to update the std::vector of services? Or maybe it should have a different thread that waits (blocks) on a queue.
