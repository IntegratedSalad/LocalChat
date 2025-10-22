#pragma once
#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include "BVApp.hpp"

/*
    BVApp_ConsoleClient_Bonjour functions as a console application of LocalChat.
    Define as a function object, because it should be run in a different thread?

    Maybe it should be just a simple application with a ">> " prompt.
    User types: "List" -> prints browsed clients
    User types: "Message XXX MMMMMMM" -> sends message
    etc...
    As simple as it gets.
    It does however, browse continuously for new services
*/
class BVApp_ConsoleClient_Bonjour : private BVApp
{
private:

public:
    // BVApp_ConsoleClient_Bonjour();

    void Init(void) override;
    void Run(void) override;
    void Quit(void) override;

    // BVStatus SendMessage(const asio::const_buffer);

    BVStatus SendMessage(const std::string&);
    BVStatus ReadMessages(void);
    BVStatus PrintServices(void);

    ~BVApp_ConsoleClient_Bonjour();
};
