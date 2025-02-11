#include <iostream>
#include <boost/asio.hpp>
#include <thread>
#include <future>
#include "dns_sd.h"
#include "BVService_Bonjour.hpp"

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
        std::cout << "Currently running daemon (system service) is version " 
                  << v / 10000 << "." << v / 100 % 100 << "." << v % 100 << std::endl;
    }
    /* */

    std::string hostname = boost::asio::ip::host_name();  // Use an appropriate host name or retrieve it
    std::string domain = "local";

    /* 
        Let's keep registration procedure synchronous, at least for now.
        It really is a crucial step in order for the application to function.
    */
    BVService_Bonjour BV_Bonjour{hostname, domain, PORT};
    BVStatus status = BV_Bonjour.Register(); // blocks
    if (status == BVStatus::BVSTATUS_OK)
    {
        std::cout << "Setup successful" << std::endl;
    } else
    {
        std::cerr << "Setup failed" << std::endl;
        std::exit(-1);
    }

    boost::asio::thread_pool tp{3};

    return 0;
}

// BVStatus registerService(std::string& hostname, 
//                          std::string& domain,
//                          const int port)
// {
//     BVStatus status;
//     BVService_Bonjour BV_Bonjour{hostname, domain, port};
//     status = BV_Bonjour.Register(ioContext);
//     return status;
// }

/*
    TODO:
    Setup tests and playground

*/