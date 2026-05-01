#include "BVDiscovery_Bonjour.hpp"

BVDiscovery_Bonjour::BVDiscovery_Bonjour(const BVServiceData _hostData,
                                         boost::asio::io_context& _ioContext,
                                         std::shared_ptr<threadsafe_queue<BVMessage>> _outMbx,
                                         std::shared_ptr<threadsafe_queue<BVMessage>> _inMbx) :
ioContext(_ioContext),
browseFD(_ioContext),
pauseTimer(_ioContext),
BVDiscovery(_hostData),
BVComponent(_outMbx, _inMbx)
{
    RegisterCallback(BVEventType::BVEVENTTYPE_DISCOVERY_REQUEST_START,
                    std::bind(&BVDiscovery_Bonjour::OnStart, this, std::placeholders::_1));

    RegisterCallback(BVEventType::BVEVENTTYPE_DISCOVERY_REQUEST_PAUSE, 
                     std::bind(&BVDiscovery_Bonjour::OnPause, this, std::placeholders::_1));
    
    RegisterCallback(BVEventType::BVEVENTTYPE_DISCOVERY_REQUEST_RESUME, 
                     std::bind(&BVDiscovery_Bonjour::OnResume, this, std::placeholders::_1));

    RegisterCallback(BVEventType::BVEVENTTYPE_TERMINATE_ALL,
                     std::bind(&BVDiscovery_Bonjour::OnShutdown, this, std::placeholders::_1));

    RegisterCallback(BVEventType::BVEVENTTYPE_DISCOVERY_REQUEST_SHUTDOWN,
                     std::bind(&BVDiscovery_Bonjour::OnShutdown, this, std::placeholders::_1));

    RegisterCallback(BVEventType::BVEVENTTYPE_DISCOVERY_REQUEST_RESTART,
                     std::bind(&BVDiscovery_Bonjour::OnRestart, this, std::placeholders::_1));

    RegisterCallback(BVEventType::BVEVENTTYPE_DISCOVERY_REQUEST_RESOLVE,
                     std::bind(&BVDiscovery_Bonjour::OnResolveRequest, this, std::placeholders::_1));
    this->dnsRef = nullptr;
    Setup();
}

void BVDiscovery_Bonjour::Setup(void)
{
    this->CreateConnectionContext();
    SetStatus(BVStatus::BVSTATUS_OK);
}

BVStatus BVDiscovery_Bonjour::OnStart(std::unique_ptr<std::any>)
{
    return BVStatus::BVSTATUS_OK;
}

// After pause timer, the browsing is resumed
BVStatus BVDiscovery_Bonjour::OnPause(std::unique_ptr<std::any>)
{
    // Start timer
    if (!this->GetIsBrowsingActive())
    {
        return BVStatus::BVSTATUS_OK; // this shouldn't happen, but don't do anything.
    }
    this->SetIsBrowsingActive(false);
    LogTrace("Discovery: Pausing...");

    boost::system::error_code ec;
    browseFD.cancel(ec);

    this->pauseTimer.expires_after(std::chrono::seconds(this->pauseTimerDelayS));
    this->pauseTimer.async_wait([this](const boost::system::error_code& ec)
    {
        if (ec == boost::asio::error::operation_aborted)
        {
            LogTrace("Pause timer cancelled.");
            return;
        }
        this->OnResume(nullptr);
    });

    return BVStatus::BVSTATUS_OK;
}

BVStatus BVDiscovery_Bonjour::OnResume(std::unique_ptr<std::any>)
{
    // Cancel timer
    const std::size_t opCancelledNum = this->pauseTimer.cancel();
    LogTrace("Discovery: pauseTimer has timed out.");
    this->SetIsBrowsingActive(true);
    LogTrace("Discovery: Resuming...");
    AwaitFDForProcessingBrowseResult();
    return BVStatus::BVSTATUS_OK;
}

BVStatus BVDiscovery_Bonjour::OnRestart(std::unique_ptr<std::any>)
{
    return BVStatus::BVSTATUS_OK;
}

BVStatus BVDiscovery_Bonjour::OnShutdown(std::unique_ptr<std::any>)
{
    this->SetIsBrowsingActive(false);
    this->browseFD.cancel();
    this->pauseTimer.cancel();
    this->ioContext.stop();
    LogTrace("Discovery: Shutting down...");
    return BVStatus::BVSTATUS_OK;
}

void BVDiscovery_Bonjour::CreateConnectionContext(void)
{
    /*
        DNSServiceBrowse is needed to be called exactly once.
        Browsing goes indefinitely, until the DNSServiceRef is passed to
        DNSServiceRefDeallocate.
    */
    const BVServiceData hd = this->GetHostData();
    LogTrace("Discovery: Browsing for {}.{}", hd.regtype, hd.domain);
    /* 
     *  We probably have to create an outer structure to fit
     *  isRemoved flag.
     *  That would be the most non-invasive and coherent solution
     *  to return information telling us that the browse callback
     *  have been invoked because of service being either registered
     *  or deregistered. However, I might actually take a shortcut here
     *  and put a flag inside LinkedList_str struct.
     *  This is nonsense, as it should not contain application
     *  specific information. However, it will do for now.  
    */ 
    DNSServiceErrorType error = DNSServiceBrowse(&this->dnsRef,
                                                0,
                                                0,
                                                hd.regtype.c_str(),
                                                hd.domain.c_str(),
                                                C_ServiceBrowseReply,
                                                &this->GetLinkedList_p());
    if (!(error == kDNSServiceErr_NoError))
    {
        this->SetStatus(BVStatus::BVSTATUS_FATAL_ERROR);
        LogError("Discovery - couldn't initialize dnsRef by DNSServiceBrowse {}", error);
        return;
    }
    LogTrace("Discovery: Connection context created.");
}

// Resolve context is a more complex type than simple DNSServiceRef.
// The posix_descriptor has to outlive the ResolveService too
std::shared_ptr<Bonjour_ResolveContext> BVDiscovery_Bonjour::CreateResolveContext(const BVServiceBrowseInstance& bI)
{
    auto ctx = std::make_shared<Bonjour_ResolveContext>(ioContext);
    ctx->callback_ctx.discovery_p = this; 
    // this has to outlive this function, so we have to put
    // ResolveCallbackContext into Bonjour_ResolveContext.
    // ctx has to be a shared pointer
    ::memcpy(ctx->callback_ctx.serviceName, bI.serviceName.c_str(), N_BYTES_SERVNAME_MAX);
    ::memcpy(ctx->callback_ctx.regType, bI.regType.c_str(), N_BYTES_REGTYPE_MAX);
    ::memcpy(ctx->callback_ctx.replyDomain, bI.replyDomain.c_str(), N_BYTES_REPLDOMN_MAX);
    DNSServiceRef resolveDnsRef = nullptr;
    DNSServiceErrorType error = DNSServiceResolve(&resolveDnsRef,
                                                  0,
                                                  0,
                                                  bI.serviceName.c_str(), 
                                                  bI.regType.c_str(),
                                                  bI.replyDomain.c_str(),
                                                  C_ResolveReply,
                                                  &ctx->callback_ctx);
    ctx->sdRef = resolveDnsRef;

    if (!(error == kDNSServiceErr_NoError))
    {
        this->SetStatus(BVStatus::BVSTATUS_FATAL_ERROR);
        LogError("Discovery - couldn't initialize resolveDnsRef by DNSServiceResolve {}", error);
        return nullptr;        
    }
    LogTrace("Discovery: Resolve context created.");
    return ctx;
}

BVStatus BVDiscovery_Bonjour::ProcessDNSServiceBrowseResult(void)
{
    using BVServiceBrowseInstanceList = std::list<BVServiceBrowseInstance>;
    if (!this->GetIsBrowsingActive() || this->dnsRef == nullptr)
    {
        return BVStatus::BVSTATUS_OK;
    }
    // Now, this shouldn't block, as this handler is only caled when the socked is ready to read.
    DNSServiceErrorType error = DNSServiceProcessResult(this->dnsRef);
    if (error != kDNSServiceErr_NoError) {
        std::cerr << "[ProcessDNSServiceBrowseResult] Encountered an error in DNSServiceBrowseResult: " << error << std::endl;
        this->SetIsBrowsingActive(false);
        return BVStatus::BVSTATUS_NOK; // setup a flag maybe?
    }
    BVServiceBrowseInstanceList browseInstanceList = ReturnListFromBrowseResults();
    // Very weird. We receive this on disconnecting the other host.
    // Apparently, there is a flag that tells us if
    // callback has been called for adding or removing a service:
    /* From dns_sd.h:
    /* Flags for domain enumeration and browse/query reply callbacks.
     * ""Default" applies only to enumeration and is only valid in
     * conjunction with "Add". An enumeration callback with the "Add"
     * flag NOT set indicates a "Remove", i.e. the domain is no longer
     * valid."
     */
    // So service deregistration can be handled from the mDNS side...

    if (GetDidServiceRegister())
    {
        LogTrace("Discovery: DNSServiceProcessResult returned. Sending BVEVENTTYPE_APP_PUBLISHED_SERVICE to App...");
        SendMessage(BVMessage(
                        BVEventType::BVEVENTTYPE_APP_PUBLISHED_SERVICE, 
                            std::make_unique<std::any>(std::make_any<BVServiceBrowseInstanceList>(browseInstanceList))));
    } else
    {
        LogTrace("Discovery: DNSServiceProcessResult returned. Sending BVEVENTTYPE_APP_DEREGISTERED_SERVICE to App...");
        SendMessage(BVMessage(
                BVEventType::BVEVENTTYPE_APP_DEREGISTERED_SERVICE, 
                    std::make_unique<std::any>(std::make_any<BVServiceBrowseInstanceList>(browseInstanceList))));
    }
    LinkedList_str_ClearList(this->GetLinkedList_p());
    // What will happen if multiple 
    // services register? Will they be added to this list, or
    // this will be called multiple times?
    AwaitFDForProcessingBrowseResult();
    return BVStatus::BVSTATUS_OK;
}

// Put on Worker thread
// This starts AwaitingFD, which triggers a handler whenever the socket is marked
// as ready to read from.
void BVDiscovery_Bonjour::Browse(void)
{
    const int fd = DNSServiceRefSockFD(this->dnsRef);
    if (fd < 0)
    {
        DNSServiceRefDeallocate(this->dnsRef);
        this->dnsRef = nullptr;
        LogError("Discovery: fd for daemon socket returned < 0.");
        return;
    }

    boost::system::error_code ec;
    browseFD.assign(fd, ec);
    if (ec) 
    {
        DNSServiceRefDeallocate(this->dnsRef);
        this->dnsRef = nullptr;
        LogError("Discovery: couldn't assign the posix stream descriptor.");
        return;
    }
    this->SetIsBrowsingActive(true);
    AwaitFDForProcessingBrowseResult(); // this puts work to ioContext
    LogTrace("Discovery: Browsing active...");
}

void BVDiscovery_Bonjour::AwaitFDForProcessingBrowseResult(void)
{
    if (!this->GetIsBrowsingActive() || this->dnsRef == nullptr)
    {
        return;
    }
    LogTrace("Discovery: Awaiting read readiness on the socket...");
    this->browseFD.async_wait(boost::asio::posix::stream_descriptor::wait_read,
                              [this](const boost::system::error_code& e)
                              {
                                if (e == boost::asio::error::operation_aborted)
                                    return;
                                if (e)
                                {
                                    SetIsBrowsingActive(false);
                                    return;
                                }
                                this->ProcessDNSServiceBrowseResult();
                              });
}

BVStatus BVDiscovery_Bonjour::ResolveService(const BVServiceBrowseInstance& bI)
{
    LogTrace("Discovery: Received request to resolve a service...");
    std::shared_ptr<Bonjour_ResolveContext> ctx_p = CreateResolveContext(bI);
    if (ctx_p == nullptr)
    {
        return BVStatus::BVSTATUS_FATAL_ERROR;
    }
    const int fd = DNSServiceRefSockFD(ctx_p->sdRef);
    if (fd < 0)
    {
        DNSServiceRefDeallocate(ctx_p->sdRef);
        ctx_p->sdRef = nullptr;
        LogError("Discovery: fd for resolve daemon socket returned < 0.");
        return BVStatus::BVSTATUS_FATAL_ERROR;
    }

    boost::system::error_code ec;
    ctx_p->resolveFD.assign(fd, ec);
    if (ec) 
    {
        DNSServiceRefDeallocate(this->dnsRef);
        this->dnsRef = nullptr;
        LogError("Discovery: couldn't assign the posix stream descriptor.");
        return BVStatus::BVSTATUS_FATAL_ERROR;
    }

    ctx_p->resolveFD.async_wait(boost::asio::posix::stream_descriptor::wait_read,
                         [this, ctx_p](const boost::system::error_code& e){
                          if (e == boost::asio::error::operation_aborted)
                            return;
                          DNSServiceErrorType error = DNSServiceProcessResult(ctx_p->sdRef);
                          if (error != kDNSServiceErr_NoError)
                          {
                            SetIsBrowsingActive(false);
                            this->LogError("Error in completion handler of resolveFD!");
                            this->LogError("Error in DNSServiceProcessResult! {}", error);
                            return;
                          }
                          LogTrace("Discovery: Resolve result has been processed.");
                          DNSServiceRefDeallocate(ctx_p->sdRef);
                          LogTrace("Discovery: Resolve ref has been deallocated.");
                        });

    LogTrace("Discovery: Resolve job scheduled...");
    return BVStatus::BVSTATUS_OK;
}

BVDiscovery_Bonjour::~BVDiscovery_Bonjour()
{
    // When dnsRef is deallocated, browsing stops.
    // TODO: Think of it maybe being deallocated in a separate method for control
    LogTrace("Discovery dies.");
    SetStatus(BVStatus::BVSTATUS_DEAD);
    DNSServiceRefDeallocate(this->dnsRef); 
}
