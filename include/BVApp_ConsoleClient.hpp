#pragma once
#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <termios.h>
#include <unistd.h>
#include <optional>
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

enum class BVConsoleActionType
{
    BVCONSOLEACTION_SENDMSG,
    BVCONSOLEACTION_REPRINT,
    BVCONSOLEACTION_QUIT,
    BVCONSOLEACTION_PAUSE_DISCOVERY,
    BVCONSOLEACTION_RESUME_DISCOVERY,
    BVCONSOLEACTION_BLOCKHOST
};

struct ConsoleActionS
{
    BVConsoleActionType type;
};

/* BVTerminal
   BVTerminal allows to set certain terminal options
   for a nicer output.
*/
class BVTerminal
{
private:
    termios originalTerminal;
    termios currentTerminal;
    bool isInitialized{false};

    void EnsureInitialized(void) const
    {
        if (!isInitialized)
        {
            throw std::runtime_error("Terminal not initialized");
        }
    }
public:
    BVTerminal()
    {
        if (!::isatty(STDIN_FILENO))
        {
            throw std::runtime_error("BVTerminal: stdin is not a terminal");
        }

        if (::tcgetattr(STDIN_FILENO, &originalTerminal) != 0)
        {
            throw std::runtime_error("BVTerminal: tcgetattr failed");
        }
        currentTerminal = originalTerminal;
        isInitialized = true;
    }

    ~BVTerminal()
    {
        Restore();
    }

    enum class InputMode
    {
        Canonical,
        NonCanonical
    };

    struct Config
    {
        InputMode mode{InputMode::Canonical};
        bool echo{true};
        cc_t vmin{1};
        cc_t vtime{0};
    };

    BVTerminal(const BVTerminal&) = delete;
    BVTerminal& operator=(const BVTerminal&) = delete;

    void SetCanonicalMode(bool echo = true)
    {
        Config cfg;
        cfg.mode = InputMode::Canonical;
        cfg.echo = echo;
        Apply(cfg);
    }

    void SetNonCanonicalMode(bool echo = false, cc_t vmin = 1, cc_t vtime = 0)
    {
        Config cfg;
        cfg.mode = InputMode::NonCanonical;
        cfg.echo = echo;
        cfg.vmin = vmin;
        cfg.vtime = vtime;
        Apply(cfg);
    }

    void FlushInput() const
    {
        EnsureInitialized();
        if (::tcflush(STDIN_FILENO, TCIFLUSH) != 0)
        {
            throw std::runtime_error("BVTerminal: tcflush failed");
        }
    }

    void Apply(const Config& cfg)
    {
        EnsureInitialized();
        termios t = originalTerminal;
        if (cfg.mode == InputMode::Canonical)
        {
            t.c_lflag |= ICANON;
        }
        else
        {
            t.c_lflag &= ~ICANON;
            t.c_cc[VMIN] = cfg.vmin;
            t.c_cc[VTIME] = cfg.vtime;
        }
        if (cfg.echo)
        {
            t.c_lflag |= ECHO;
        }
        else
        {
            t.c_lflag &= ~ECHO;
        }
        if (::tcsetattr(STDIN_FILENO, TCSANOW, &t) != 0)
        {
            throw std::runtime_error("BVTerminal: tcsetattr failed");
        }
        currentTerminal = t;
    }

    void Restore()
    {
        if (isInitialized)
        {
            ::tcsetattr(STDIN_FILENO, TCSANOW, &originalTerminal);
        }
    }

    char ReadChar() const
    {
        EnsureInitialized();

        char c = '\0';
        const ssize_t n = ::read(STDIN_FILENO, &c, 1);
        // if (n < 0)
        // {
        //     throw std::runtime_error("BVTerminal: read failed");
        // }
        if (n == 0)
        {
            throw std::runtime_error("BVTerminal: EOF on stdin");
        }
        return c;
    }

    std::string ReadLine() const
    {
        EnsureInitialized();

        std::string line;
        if (!std::getline(std::cin, line))
        {
            throw std::runtime_error("BVTerminal: getline failed");
        }
        return line;
    }
};

class BVApp_ConsoleClient : public BVApp,
                            public BVComponent,
                            public BVLoggable
{
private:
    std::mutex stdoutMutex; // mutex for internal worker threads, in this case printing.
    std::thread stdinThread; // worker thread? I don't think this is needed
    BVTerminal terminal{};

public:
    BVApp_ConsoleClient(std::shared_ptr<threadsafe_queue<BVMessage>> _outMbx,
                        std::shared_ptr<threadsafe_queue<BVMessage>> _inMbx,
                        boost::asio::io_context& _ioContext);

    void Run(void) override;

    BVStatus HandlePublishedServices(std::unique_ptr<std::any> dp) override;

    BVStatus ReadMessages(void);
    BVStatus PrintMessages(void);
    BVStatus PrintServices(void);
    void PrintNewServicesNotification(void);
    void PrintAll(void);
    std::optional<BVConsoleActionType> ParseConsoleActionFromKey(char key);
    // void PrintAll(bool);

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
