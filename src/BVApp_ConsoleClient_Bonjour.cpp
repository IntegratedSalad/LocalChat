#include "BVApp_ConsoleClient_Bonjour.hpp"

void BVApp_ConsoleClient_Bonjour::Init(void)
{
}
void BVApp_ConsoleClient_Bonjour::Run(void)
{
    this->Init();
    this->StartListenerThread();
    while (this->GetIsRunning())
    {
        // print console

        // Now -> if app wants tu utilize 'serviceV' it has to acquire a lock
        // on condition that this->isDiscoveryQueueReady = false; meaning that the queue is empty,
        // it's not being processed.
        // It cannot read from this->isDiscoveryQueueReady without mutex!
        // Or other mutex => because use here doesn't concern the queue, it concerns the vector.
    }
}
void BVApp_ConsoleClient_Bonjour::Quit(void)
{
    // Also DeInit
}

BVStatus BVApp_ConsoleClient_Bonjour::PrintServices(void)
{
    BVStatus status = BVStatus::BVSTATUS_OK;
    std::lock_guard vlk(this->serviceVectorMutex);


}

void BVApp_ConsoleClient_Bonjour::HandleServicesDiscoveredUpdateEvent(void)
{

}

void BVApp_ConsoleClient_Bonjour::HandleUserKeyboardInput(void)
{

}

// BVStatus SendMessage(const asio::const_buffer);

// BVStatus BVApp_ConsoleClient_Bonjour::ParseAction(const std::string&)
// BVStatus BVApp_ConsoleClient_Bonjour::SendMessage(const std::string&);
// BVStatus BVApp_ConsoleClient_Bonjour::ReadMessages(void);
// BVStatus BVApp_ConsoleClient_Bonjour::PrintServices(void);
// void BVApp_ConsoleClient_Bonjour::PrintScreen(void);
// BVApp_ConsoleClient_Bonjour::~BVApp_ConsoleClient_Bonjour();