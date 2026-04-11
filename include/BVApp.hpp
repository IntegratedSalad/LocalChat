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

struct BVUser
{
    std::string name;
    BVHost serviceResolvedData;
};

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
    std::thread io_thread;

    boost::asio::io_context& ioContext; // App owns ioContext
    // workGuard is needed so we don't stop ioContext when no job has been scheduled early enough.
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> workGuard;

    BVStatus status = BVStatus::BVSTATUS_IN_PROGRESS;

protected:
    std::vector<BVServiceBrowseInstance> serviceV; // iterable for services e.g. to display
    std::vector<BVHost> hostsV; // after resolving
    std::mutex serviceVectorMutex;
    // Maybe connection manager which gets a reference to io_context?
    // He will be making connections to users.
    // What is a connection?
    // Can the host that wants to communicate just open a socket themselves?
    // Or the endpoints need to be opened after browsing and resolution to hostname then IP?

    // Find IP
    virtual BVHost ResolveServiceToEndpoint(const std::string& hosttarget, const std::string& serviceName, const int port) = 0;

public:
    BVApp(boost::asio::io_context& _ioContext) :
    ioContext(_ioContext),
    workGuard(boost::asio::make_work_guard(_ioContext))
    {};

    virtual ~BVApp()
    {}

    // App is the owner and manager of the ioContext loop.
    void LaunchIOThread(void)
    {
        this->io_thread = std::thread([this](){
            this->ioContext.run();
            status = BVStatus::BVSTATUS_DEAD;
        });
    }

    void StopIOContext(void)
    {
        workGuard.reset();
        this->ioContext.stop();
    }

    void TryJoinIOThread(void)
    {
        if (this->io_thread.joinable())
        {
            io_thread.join();
        }
    }

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

    std::thread& GetIOThread(void)
    {
        return this->io_thread;
    }

    boost::asio::io_context& GetIoContext(void)
    {
        return this->ioContext;
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
    virtual BVStatus HandleResolvedServices(std::unique_ptr<std::any>) = 0;

    const std::atomic<bool>& GetIsRunning(void)
    {
        return this->isRunning;
    }

    void SetIsRunning(const bool state)
    {
        this->isRunning = state;
    }
};
