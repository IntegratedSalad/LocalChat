#pragma once
#include <mutex>
#include <memory>
#include <queue>
#include <condition_variable>
#include <boost/asio.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>
#include "BV.hpp"
#include "BVDiscovery.hpp"
#include "BVComponent.hpp"
#include "BVLoggable.hpp"
#include "dns_sd.h"
#include "BVLoggable.hpp"
#include "linked_list.h"
#include "bonjour_api.h"
#include "api_common.h"

/*
 *   This class is a Bonjour implementation of BV Discovery Component.
 *   It utilizes a LL of records that is copied to the shared queue.
 */

// This is different from ResolveCallbackContext - this is used in BVDiscovery_Bonjour!
struct Bonjour_ResolveContext
{
    ResolveCallbackContext callback_ctx{};
    DNSServiceRef sdRef = nullptr;
    boost::asio::posix::stream_descriptor resolveFD;

    Bonjour_ResolveContext(boost::asio::io_context& ioContext) :
    resolveFD(ioContext)
    {}
};

/* Excerpt from dns_sd.h
 * Clients explicitly wishing to discover *only* LocalOnly services during a
 * browse may do this, without flags, by inspecting the interfaceIndex of each
 * service reported to a DNSServiceBrowseReply() callback function, and
 * discarding those answers where the interface index is not set to
 * kDNSServiceInterfaceIndexLocalOnly.
*/
class BVDiscovery_Bonjour : public BVDiscovery,
                            public BVComponent,
                            public BVLoggable
{
private:
    // Is this really necessary to hold a shared pointer to service_p and not just a structure of needed params?
    // BVService_Bonjour component is alive in the main thread...
    // Probably not. Some manager class will have a pointer to the active service.
    DNSServiceRef dnsRef;
    boost::asio::posix::stream_descriptor browseFD;
    boost::asio::io_context& ioContext;
    // This is a workaround for work_guard.
    // The ioContext has to have a job scheduled, even if browsing is paused,
    // and browseFD is not awaited for readiness (the task is not put on the context loop)
    boost::asio::steady_timer pauseTimer;
    const int16_t pauseTimerDelayS = 3600;

    BVStatus ProcessDNSServiceBrowseResult(void);
    void AwaitFDForProcessingBrowseResult(void);

    void CreateConnectionContext(void) override; // private member function which actually starts the Discovery service
    std::shared_ptr<Bonjour_ResolveContext> CreateResolveContext(const BVServiceBrowseInstance& bI); // this is PER SERVICE to be resolved!
    void DestroyResolveContext(std::unique_ptr<std::any> rcp) override {/*unimplemented*/};

    void Setup(void) override;
    void Browse() override;

protected:
    BVStatus ResolveService(const BVServiceBrowseInstance& bI) override; 

public:
    BVDiscovery_Bonjour(const BVServiceData _hostData,
                        boost::asio::io_context& _ioContext,
                        std::shared_ptr<threadsafe_queue<BVMessage>> _outMbx,
                        std::shared_ptr<threadsafe_queue<BVMessage>> _inMbx);
    ~BVDiscovery_Bonjour() override;

    void SendMessage_API_INTERFACE(const BVEventType& et, std::any a_type)
    {
        this->SendMessage(BVMessage(
            et,
            std::make_unique<std::any>(a_type))
        );
    }

    BVStatus OnStart(std::unique_ptr<std::any>) override;
    BVStatus OnPause(std::unique_ptr<std::any>) override;
    BVStatus OnResume(std::unique_ptr<std::any>) override;
    BVStatus OnRestart(std::unique_ptr<std::any>) override;
    BVStatus OnShutdown(std::unique_ptr<std::any>) override;
};
