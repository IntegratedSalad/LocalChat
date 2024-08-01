#include <iostream>
#include <boost/asio.hpp>
#include <thread>
#include "dns_sd.h"

DNSServiceRef dnsRef;

// Callback function for DNSServiceRegister
void registerReply(
    DNSServiceRef sdRef,
    DNSServiceFlags flags,
    DNSServiceErrorType errorCode,
    const char *name,
    const char *regtype,
    const char *domain,
    void *context
) {
    if (errorCode == kDNSServiceErr_NoError) {
        std::cout << "--- Registered " << name << ", as " << regtype << " in " << domain << "!" << std::endl;
    } else {
        std::cout << "An error occurred while trying to register " << name << std::endl;
    }
}

// Function to register the service
void _register() {
    auto hostname = boost::asio::ip::host_name();
    const char* hostnameC = hostname.c_str();
    const char* regtype = "_localchathost._tcp";
    const char* domain = "local";
    const uint16_t port = htons(50001);
    
    DNSServiceErrorType error = DNSServiceRegister(&dnsRef,
                        0,
                        0,
                        hostnameC,
                        regtype,
                        NULL,
                        NULL,
                        port,
                        0,
                        NULL,
                        registerReply,
                        NULL);

    if (error != kDNSServiceErr_NoError) {
        std::cerr << "Encountered an error: " << error << std::endl;
    }
}

// Function to process DNS-SD results and then exit
void processDNSServiceResults() {
    DNSServiceErrorType error = DNSServiceProcessResult(dnsRef);
    
    if (error != kDNSServiceErr_NoError) {
        std::cerr << "Encountered an error in DNSServiceProcessResult: " << error << std::endl;
    }
}

int main(int argc, char** argv) {
    boost::asio::io_context ioContext;

    uint32_t v;
    uint32_t size = sizeof(v);
    DNSServiceErrorType err = DNSServiceGetProperty(kDNSServiceProperty_DaemonVersion, &v, &size);
    if (err) {
        std::cerr << "DNSServiceGetProperty failed " << (long int)err << std::endl;
    } else {
        std::cout << "Currently running daemon (system service) is version " 
                  << v / 10000 << "." << v / 100 % 100 << "." << v % 100 << std::endl;
    }

    // Register the service
    _register(); // TODO: change this to normal function name as this doesn't need to be a callback.

    // Process DNS-SD results
    // Here we don't need to call run on main IO Loop.
    // As we are processing one event (waiting for response from the daemon on the DNSServiceRegister
    // call - we call the callback and try to read response from the daemon) we can synchronously wait for it.
    // It blocks until the daemon replies.
    // If we were to process two different events, we could use the timer to periodically check for the
    // response from the daemon.
    // Although daemon "may terminate its connection with a client that does not
    // process the daemon's responses""
    //
    processDNSServiceResults();

    // Clean up DNSServiceRef
    DNSServiceRefDeallocate(dnsRef);

    return 0;
}

/*
    TODO:
    Setup tests and playground

*/