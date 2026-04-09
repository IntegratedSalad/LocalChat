#include "BVDiscovery_Bonjour.hpp"

BVDiscovery_Bonjour::BVDiscovery_Bonjour(const BVServiceHostData _hostData,
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
    this->dnsRef = nullptr;
    Setup();
}

void BVDiscovery_Bonjour::Setup(void)
{
    this->CreateConnectionContext();
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
    AwaitFD();
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
    const BVServiceHostData hd = this->GetHostData();
    LogTrace("Discovery: Browsing for {}.{}", hd.regtype, hd.domain);
    // std::cout << "Browsing for: ";
    // std::cout << hd.regtype << ".";
    // std::cout << hd.domain  << std::endl; // Do not pass the pointer to service
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
    SendMessage(BVMessage(
                    BVEventType::BVEVENTTYPE_APP_PUBLISHED_SERVICE, 
                        std::make_unique<std::any>(std::make_any<BVServiceBrowseInstanceList>(browseInstanceList))));
    LinkedList_str_ClearList(this->GetLinkedList_p());
    AwaitFD();
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
    AwaitFD(); // this puts work to ioContext
    LogTrace("Discovery: Browsing active...");
    this->ioContext.run();
}

void BVDiscovery_Bonjour::AwaitFD(void)
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
    BVStatus::BVSTATUS_OK;
}

BVDiscovery_Bonjour::~BVDiscovery_Bonjour()
{
    // When dnsRef is deallocated, browsing stops.
    // TODO: Think of it maybe being deallocated in a separate method for control
    DNSServiceRefDeallocate(this->dnsRef); 
}
