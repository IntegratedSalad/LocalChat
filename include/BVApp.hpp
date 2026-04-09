#pragma once
#include <atomic>
#include <memory>
#include <mutex>
#include <vector>
#include <list>
#include <queue>
#include <algorithm>
#include <thread>
#include <boost/asio.hpp>
#include "BV.hpp"
#include "BVService_Bonjour.hpp"

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
 * This is put on the main thread.
 *
*/
class BVApp
{
private:
    std::atomic<bool> isRunning = true;
    std::thread worker_thread;
    std::thread io_thread; // ? Maybe on this thread we run the ioContext loop.
    // Should BVApp be an owner of a ioContext? and BVDiscovery just an entity that posts jobs to it?
    // If yes, then there should be maybe another thread onto which we put ioContext.run().
    // And we join it with app (still a main thread, but not in main.cpp but at the end of Run()).
    boost::asio::io_context& ioContext;

protected:
    std::vector<BVServiceBrowseInstance> serviceV; // iterable for services e.g. to display
    std::mutex serviceVectorMutex;

    BVStatus ResolveService(const BVServiceBrowseInstance& bI)
    {
        // can I .run() the shared ioContext here?

        return BVStatus::BVSTATUS_OK;
    }

public:
    BVApp(boost::asio::io_context& _ioContext) :
    ioContext(_ioContext)
    {};

    virtual ~BVApp()
    {}

    void LaunchWorkerThread(void)
    {
        this->worker_thread = std::thread([this](){this->Run();});
    }

    void TryJoinWorkerThread(void)
    {
        if (this->worker_thread.joinable())
        {
            this->worker_thread.join();
        }
    }

    std::thread& GetWorkerThread(void)
    {
        return this->worker_thread;
    }

    std::vector<BVServiceBrowseInstance> GetServiceVectorCopy(void)
    {
        return this->serviceV;
    }

    virtual void Run(void) = 0; // start procedure

    // virtual void HandleServicesDiscoveredUpdateEvent(void) = 0;
    // virtual void HandleUserKeyboardInput(void) = 0;
    // Probably has to be guarded with a mutex!
    virtual BVStatus HandlePublishedServices(std::unique_ptr<std::any>) = 0;

    const std::atomic<bool>& GetIsRunning(void)
    {
        return this->isRunning;
    }

    void SetIsRunning(const bool state)
    {
        this->isRunning = state;
    }
};
