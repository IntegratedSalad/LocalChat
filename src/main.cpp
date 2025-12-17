#include <iostream>
#include <boost/asio.hpp>
#include <thread>
#include <future>
#include <condition_variable>
#include <chrono>
#include <queue>
#include <memory>
#include "BVDiscovery.hpp"
#if __APPLE__
#include "dns_sd.h"
#include "BVService_Bonjour.hpp"
#include "BVDiscovery_Bonjour.hpp"
#elif __linux__
#include "BVService_Avahi.hpp"
#include "BVDiscovery_Avahi.hpp"
#endif
#include "BVApp_ConsoleClient_Bonjour.hpp"

std::mutex discoveryQueueMutex;
std::mutex messageQueueMutex;
std::condition_variable discoveryQueueCV;
bool isDiscoveryQueueReady = false;
int main(int argc, char** argv)
{
    boost::asio::io_context ioContext;

#if __APPLE__
    /* Put this in a test */
    uint32_t v;
    uint32_t size = sizeof(v);
    DNSServiceErrorType err = DNSServiceGetProperty(kDNSServiceProperty_DaemonVersion, &v, &size);
    if (err)
    {
        std::cerr << "DNSServiceGetProperty failed " << (long int)err << std::endl;
        return -1;
    } else
    {
        std::cout << "Currently running daemon is version "
                  << v / 10000 << "." << v / 100 % 100 << "." << v % 100 << std::endl;
    }
    /* */
#endif
// for linux maybe write a simple function that communicates with the avahi-daemon?
    std::string hostname = boost::asio::ip::host_name();  // Use an appropriate host name or retrieve it
    std::string domain = "local";

    /*
        Let's keep registration procedure synchronous, at least for now.
        It really is a crucial step in order for the application to function.
    */

    // This has to be a parent class BVService, not dependent on implementation
#if __APPLE__
    BVService_Bonjour service{hostname, domain, PORT};
#elif __linux__
    // AvahiSimplePoll will be used by avahi service registration and then service browsing
    // TODO: shouldn't it be unique ptr?
    // Even if service does not go out of scope/is destroyed,
    // it doesn't mean that there should be one simple poll shared by all of the objects.
    // fix it later...
    std::shared_ptr<AvahiSimplePoll> simple_poll_p = BVService_Avahi::MakeSimplePoll(avahi_simple_poll_new());
    if (simple_poll_p == nullptr)
    {
        std::cerr << "Failed to create simple poll object." << std::endl;
        std::exit(-1);
    }
    BVService_Avahi service{hostname, domain, PORT, simple_poll_p};
#endif
    BVStatus status = service.Register(); // blocks
    if (status == BVStatus::BVSTATUS_OK)
    {
        std::cout << "Setup successful" << std::endl;
    } else
    {
        std::cerr << "Setup failed" << std::endl;
        std::exit(-1);
    }
    // From this point, the context pointers of both implementations are alive, and the services are discoverable

    std::shared_ptr<std::queue<BVServiceBrowseInstance>> discoveryQueue_p =
        std::make_shared<std::queue<BVServiceBrowseInstance>>();
    std::shared_ptr<std::queue<BVThrMessage>> messageQueue_p =
        std::make_shared<std::queue<BVThrMessage>>();

    // BVDiscovery_XXX class doesn't need a pointer to the service class.
    // It needs only:
    // hostname, domain, port and context pointer:
    // In case of Bonjour, it's null, because DNSServiceRef is allocated upon browsing
    // In case of Avahi, a pointer to AvahiClient
    // Furthermore, Bonjour Discovery class is NOT utilizing any data that's specific to the BVService class,
    // because hostname, domain and type are known ahead of the creation of the object

    /* 
     TODO:
     Do we require an additional manager object?
     What if client wants to reconnect?
     After the service is registered, the discovery object gets ownership
     of the client. If that discovery gets halted, we exit the discovery thread,
     what is going to resume it? There's no control over the discovery other than
     running in once.
     Maybe App should get it, or there should be a manager object for that...

     Yes - architecture rewrite is needed
    */

#if __APPLE__
    // Create a discovery object, that periodically performs DNS-SD functionality.
    std::shared_ptr<const BVService_Bonjour> service_p =
        std::make_shared<const BVService_Bonjour>(service);
    BVDiscovery_Bonjour discovery{service_p,
                                  discoveryQueueMutex,
                                  ioContext,
                                  discoveryQueue_p,
                                  discoveryQueueCV,
                                  isDiscoveryQueueReady}; // TODO: Pass messageQueue
#endif
#if __linux__
    auto data = service.TransferClient();
    BVDiscovery_Avahi discovery{std::move(data),
                                discoveryQueueMutex,
                                ioContext,
                                discoveryQueue_p,
                                discoveryQueueCV,
                                isDiscoveryQueueReady,
                                service.GetHostData(),
                                simple_poll_p}; // TODO: Pass messageQueue
#endif
    BVApp_ConsoleClient_Bonjour consoleClient{discoveryQueue_p,
                                              discoveryQueueMutex,
                                              discoveryQueueCV,
                                              isDiscoveryQueueReady}; // TODO: Pass messageQueue

    std::thread td([&discovery](){
        discovery();
    });

    consoleClient.Run();
    td.join();

    return 0;
}
