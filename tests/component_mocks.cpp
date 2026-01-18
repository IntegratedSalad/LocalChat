#include "component_mocks.hpp"

MockDiscovery::MockDiscovery(const BVServiceHostData _hostData,
                std::mutex& _discoveryQueueMutex,
                boost::asio::io_context& _ioContext,
                std::shared_ptr<std::queue<BVServiceBrowseInstance>> _discoveryQueue,
                std::condition_variable& _discoveryQueueCV,
                bool& _isDiscoveryQueueReady) :
    ioContext(_ioContext),
    discoveryTimer(_ioContext),
    BVDiscovery(_hostData, 
                _discoveryQueueMutex,
                _discoveryQueue,
                _discoveryQueueCV,
                _isDiscoveryQueueReady)
{

}

MockDiscovery::~MockDiscovery()
{

}

void MockDiscovery::CreateConnectionContext(void)
{

}

void MockDiscovery::Setup(void)
{

}

void MockDiscovery::run(void)
{

}

BVStatus MockDiscovery::OnStart(void)
{

}

BVStatus MockDiscovery::OnShutdown(void)
{

}

BVStatus MockDiscovery::OnRestart(void)
{

}
