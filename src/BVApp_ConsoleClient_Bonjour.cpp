#include "BVApp_ConsoleClient_Bonjour.hpp"

void BVApp_ConsoleClient_Bonjour::Init(void)
{
}
void BVApp_ConsoleClient_Bonjour::Run(void)
{
    this->StartListenerThread();
    // Wow! Always make sure that you're not calling functions that lock resources
    // where they shouldn't be called
    // this->PrintAll();
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

// I think that any event that needs to draw something
// must redraw everything
void BVApp_ConsoleClient_Bonjour::PrintAll(void)
{
    for (int i = 0; i < 100; i++) {std::cout << std::endl;}
    std::cout << "LocalChat console client v0.2.1.2a" << std::endl;
    std::cout << "(P)rint services" << std::endl;
    std::cout << "(S)witch to conversation" << std::endl;
    std::cout << "(E)xit" << std::endl;
    std::cout << "-----------------------------" << std::endl;
    std::cout << "Available services:" << std::endl;
    this->PrintServices();
    std::cout << "=============================" << std::endl;
    std::cout << ">> " << std::endl;
}

BVStatus BVApp_ConsoleClient_Bonjour::PrintServices(void)
{
    std::lock_guard vlk(this->serviceVectorMutex);
    BVStatus status = BVStatus::BVSTATUS_OK;
    if (this->serviceV.size() == 0)
    {
        std::cout << "None available" << std::endl;
    }
    int i = 1;
    for (BVServiceBrowseInstance& bI : this->serviceV)
    {
        std::cout << i++ << ":" << std::endl;
        bI.print();
        std::cout << "+-+-+-+-" << std::endl;
    }
    return status;
}

void BVApp_ConsoleClient_Bonjour::HandleServicesDiscoveredUpdateEvent(void)
{
    this->PrintAll();
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