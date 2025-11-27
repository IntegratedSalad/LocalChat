#pragma once
#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include "BVApp.hpp"

// Does it really have to be a Bonjour implementation (and later Avahi?)

/*
    BVApp_ConsoleClient_Bonjour functions as a console application of LocalChat.
    Define as a function object, because it should be run in a different thread?

    Maybe it should be just a simple application with a ">> " prompt.
    User types: "List" -> prints browsed clients
    User types: "Message XXX MMMMMMM" -> sends message
    etc...
    As simple as it gets.
    It does however, browse continuously for new services.
    It can use (LISTENFORMESSAGE) which basically tells us that the client blocks on read from
    stdin OR it can create a separate thread on which it listens for message and updates "screen" =>
    prints ~70 new lines and messages and prompt etc.
    Different screens - main screen for available hosts and then separate screen for each host
    Maybe separate object that does I/O operation.
    In form of a dispatcher that is a separate thread, but THE ONLY thread that operates on stdout
    It has its queue and waits for events.
*/

typedef enum class BVConsoleAction
{
    BVCONSOLEACTION_LISTHOSTS, // this would be good if it happened concurrently.
    BVCONSOLEACTION_SENDMSG,
    BVCONSOLEACTION_REPRINT,
    BVCONSOLEACTION_EXIT,
    BVCONSOLEACTION_BLOCKHOST
} BVConsoleAction;

class BVApp_ConsoleClient_Bonjour : private BVApp
{
private:
    std::mutex stdoutMutex; // mutex for internal worker threads, in this case printing.
    std::thread stdinThread;

public:
    BVApp_ConsoleClient_Bonjour(std::shared_ptr<std::queue<BVServiceBrowseInstance>> _discoveryQueue,
                                std::mutex& _discoveryQueueMutex,
                                std::condition_variable& _discoveryQueueCV,
                                bool& _isDiscoveryQueueReady) :
    BVApp(_discoveryQueue, _discoveryQueueMutex, _discoveryQueueCV, _isDiscoveryQueueReady)
    {
        this->Init();
    }

    void Init(void) override;
    void Run(void) override;
    void Quit(void) override;

    BVStatus ParseAction(const std::string&);
    BVStatus SendMessage(const std::string&);
    BVStatus ReadMessages(void);
    BVStatus PrintMessages(void);
    BVStatus PrintServices(void);
    void PrintAll(void);

    void HandleServicesDiscoveredUpdateEvent(void) override;
    void HandleUserKeyboardInput(void) override;

    // -------------------------------------------------------

    ~BVApp_ConsoleClient_Bonjour() {}
};
