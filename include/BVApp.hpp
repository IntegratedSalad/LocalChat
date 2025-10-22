#pragma once
#include <memory>
#include <vector>
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
    // std::shared_ptr<std::vector> servicesV;
    // or servicesV should belong to Actor?
    // app should have only the app's params.
    // states,

    bool isRunning = true;
    BVActor actor;
    std::shared_ptr<std::queue> service_queue;
    // event queue?

public:
    BVApp::BVApp(std::shared_ptr<std::queue> _service_queue) : service_queue(_service_queue)

    virtual void Init(void) = 0;
    virtual void Run(void) = 0;
    virtual void Quit(void) = 0;

    virtual void HandleServicesDiscoveredUpdateEvent(void) = 0;
    virtual void HandleUserKeyboardInput(void) = 0;

    void operator()(void)
    {
        this->Init();
        this->Run();
    }
};

// LC_Client will be a derivation of BVApp
// BVApp has method run => is a functional object that can be run.
// It takes a pointer to the queue.
// Can somehow Discovery class send something to update the std::vector of services? Or maybe it should have a different thread that waits (blocks) on a queue.
