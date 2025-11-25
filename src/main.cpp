#include <iostream>
#include <boost/asio.hpp>
#include <thread>
#include <future>
#include <condition_variable>
#include "dns_sd.h"
#include "BVService_Bonjour.hpp"
#include "BVDiscovery_Bonjour.hpp"
#include "BVApp_ConsoleClient_Bonjour.hpp"

std::mutex discoveryQueueMutex;
std::mutex messageQueueMutex;
std::condition_variable discoveryQueueCV;
bool isDiscoveryQueueReady = false;
int main(int argc, char** argv)
{
    boost::asio::io_context ioContext;

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

    std::string hostname = boost::asio::ip::host_name();  // Use an appropriate host name or retrieve it
    std::string domain = "local";

    /*
        Let's keep registration procedure synchronous, at least for now.
        It really is a crucial step in order for the application to function.
    */
    BVService_Bonjour service{hostname, domain, PORT};
    BVStatus status = service.Register(); // blocks
    if (status == BVStatus::BVSTATUS_OK)
    {
        std::cout << "Setup successful" << std::endl;
    } else
    {
        std::cerr << "Setup failed" << std::endl;
        std::exit(-1);
    }

    std::shared_ptr<std::queue<BVServiceBrowseInstance>> discoveryQueue_p =
        std::make_shared<std::queue<BVServiceBrowseInstance>>();
    std::shared_ptr<std::queue<BVThrMessage>> messageQueue_p =
        std::make_shared<std::queue<BVThrMessage>>();

    // TODO: change this to the structure holding data to current service (host)
    // Not necessarily. This object can be used to distinguish host service from others.
    // But BVDiscovery_Bonjour doesn't need the entire class
    std::shared_ptr<const BVService_Bonjour> service_p =
        std::make_shared<const BVService_Bonjour>(service);

    // Create a discovery object, that periodically performs DNS-SD functionality.
    BVDiscovery_Bonjour discovery{service_p,
                                  discoveryQueueMutex,
                                  ioContext,
                                  discoveryQueue_p,
                                  discoveryQueueCV,
                                  isDiscoveryQueueReady}; // TODO: Pass messageQueue

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