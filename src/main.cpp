#include <iostream>
#include <boost/asio.hpp>
#include <thread>
#include "dns_sd.h"
#include "BVService_Bonjour.hpp"

int main(int argc, char** argv) {
    boost::asio::io_context ioContext;

    /* Put this in a test */
    uint32_t v;
    uint32_t size = sizeof(v);
    DNSServiceErrorType err = DNSServiceGetProperty(kDNSServiceProperty_DaemonVersion, &v, &size);
    if (err)
    {
        std::cerr << "DNSServiceGetProperty failed " << (long int)err << std::endl;
    } else
    {
        std::cout << "Currently running daemon (system service) is version " 
                  << v / 10000 << "." << v / 100 % 100 << "." << v % 100 << std::endl;
    }

    std::string hostname = boost::asio::ip::host_name();  // Use an appropriate host name or retrieve it
    std::string domain = "local";
    int port = 50001;

    BVService_Bonjour BV_Bonjour{hostname, domain, port};
    BVStatus status = BV_Bonjour.Register(ioContext);

    /*
        Let's try to register the service, wait for the daemon to reply
        and schedule another, arbitrary task providing a function that counts to ten using
        a timer.
     */

    boost::asio::deadline_timer deadlinetimer{ioContext, };

    if (status == BVStatus::BVSTATUS_OK)
    {
        try 
        {
            std::cout << "Running io context..." << std::endl;
            ioContext.run();
        } catch (const std::exception& ex)
        {
            std::cout << ex.what() << std::endl;
        }
    } else
    {
        std::cerr << "Registration failed" << std::endl;
    }



    return 0;
}

/*
    TODO:
    Setup tests and playground

*/