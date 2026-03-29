#pragma once
#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include "BVApp.hpp"
#include "BVComponent.hpp"
#include "BVLoggable.hpp"

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

class BVApp_ConsoleClient : public BVApp,
                            public BVComponent,
                            public BVLoggable
{
private:
    std::mutex stdoutMutex; // mutex for internal worker threads, in this case printing.
    std::thread stdinThread; // worker thread? I don't think this is needed

public:
    BVApp_ConsoleClient(std::shared_ptr<threadsafe_queue<BVMessage>> _outMbx,
                        std::shared_ptr<threadsafe_queue<BVMessage>> _inMbx);

    void Run(void) override;

    BVStatus HandlePublishedServices(std::unique_ptr<std::any> dp) override;

    BVStatus ParseAction(const std::string&);
    BVStatus ReadMessages(void);
    BVStatus PrintMessages(void);
    BVStatus PrintServices(void);
    void PrintAll(void);

    // void HandleServicesDiscoveredUpdateEvent(void) override;
    // void HandleUserKeyboardInput(void) override;

    BVStatus OnStart(std::unique_ptr<std::any>) override;
    BVStatus OnResume(std::unique_ptr<std::any>) override;
    BVStatus OnShutdown(std::unique_ptr<std::any>) override;
    BVStatus OnRestart(std::unique_ptr<std::any>) override;
    BVStatus OnPause(std::unique_ptr<std::any>) override;

    // -------------------------------------------------------

    ~BVApp_ConsoleClient() {}
};
