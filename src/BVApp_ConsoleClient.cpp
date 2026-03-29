#include "BVApp_ConsoleClient.hpp"

BVApp_ConsoleClient::BVApp_ConsoleClient(std::shared_ptr<threadsafe_queue<BVMessage>> _outMbx,
                                         std::shared_ptr<threadsafe_queue<BVMessage>> _inMbx) :
BVComponent(_outMbx, _inMbx)
{
    RegisterCallback(BVEventType::BVEVENTTYPE_APP_PUBLISHED_SERVICE,
                     std::bind(&BVApp_ConsoleClient::HandlePublishedServices, this, std::placeholders::_1));
    RegisterCallback(BVEventType::BVEVENTTYPE_TERMINATE_ALL,
                     std::bind(&BVApp_ConsoleClient::OnShutdown, this, std::placeholders::_1));
}

void BVApp_ConsoleClient::Run(void)
{
    while (this->GetIsRunning())
    {
        // print console
    }
}

// I think that any event that needs to draw something
// must redraw everything
void BVApp_ConsoleClient::PrintAll(void)
{
    for (int i = 0; i < 100; i++) {std::cout << std::endl;}
    std::cout << "LocalChat console client v0.2.1.2.2a" << std::endl;
    std::cout << "(P)rint services" << std::endl;
    std::cout << "(S)witch to conversation" << std::endl;
    std::cout << "(E)xit" << std::endl;
    std::cout << "-----------------------------" << std::endl;
    std::cout << "Available services:" << std::endl;
    this->PrintServices();
    std::cout << "=============================" << std::endl;
    std::cout << ">> " << std::endl;
}

BVStatus BVApp_ConsoleClient::PrintServices(void)
{
    // TODO: Different implementation!
    // std::lock_guard vlk(this->serviceVectorMutex);
    // BVStatus status = BVStatus::BVSTATUS_OK;
    // if (this->serviceV.size() == 0)
    // {
    //     std::cout << "None available" << std::endl;
    // }
    // int i = 1;
    // for (BVServiceBrowseInstance& bI : this->serviceV)
    // {
    //     std::cout << i++ << ":" << std::endl;
    //     bI.print();
    //     std::cout << "+-+-+-+-" << std::endl;
    // }
    // return status;
}

BVStatus BVApp_ConsoleClient::HandlePublishedServices(std::unique_ptr<std::any> dp)
{
    return BVStatus::BVSTATUS_OK;
}

BVStatus BVApp_ConsoleClient::OnStart(std::unique_ptr<std::any>)
{
    return BVStatus::BVSTATUS_OK;
}

BVStatus BVApp_ConsoleClient::OnResume(std::unique_ptr<std::any>)
{
    return BVStatus::BVSTATUS_OK;
}

BVStatus BVApp_ConsoleClient::OnShutdown(std::unique_ptr<std::any>)
{
    return BVStatus::BVSTATUS_OK;
}

BVStatus BVApp_ConsoleClient::OnRestart(std::unique_ptr<std::any>)
{
    return BVStatus::BVSTATUS_OK;
}

BVStatus BVApp_ConsoleClient::OnPause(std::unique_ptr<std::any>)
{
    return BVStatus::BVSTATUS_OK;
}
