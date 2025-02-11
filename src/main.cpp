#include <iostream>
#include <boost/asio.hpp>
#include <thread>
#include "dns_sd.h"
#include "BVService_Bonjour.hpp"

void countToTen(boost::asio::steady_timer* t, boost::asio::io_context* ioc, int* count, int ms);
void askForInput(void);
void init(void);
std::atomic<BVStatus> registerStatus = BVStatus::BVSTATUS_NOK; // this should be available to all threads
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

    boost::asio::thread_pool tp{3};

    BVService_Bonjour BV_Bonjour{hostname, domain, PORT};
    BVStatus status = BV_Bonjour.Register(ioContext); // we probably need a future

    /*
        Let's try to register the service, wait for the daemon to reply
        and schedule another, arbitrary task providing a function that counts to ten using
        a timer.
     */
    std::cout << std::endl;
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

void registerService(void)
{
    BVService_Bonjour BV_Bonjour{hostname, domain, PORT};
    BVStatus status = BV_Bonjour.Register(ioContext);
    

}

/*
    TODO:
    Setup tests and playground

*/