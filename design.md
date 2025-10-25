### Design of BonVoyage FileSharing
Because what I will write here will be used for BonVoyage, a graphical client for sharing files, we should
design an interface that takes out some of the core functionality of DNS-SD.

BVXXX_Bonjour is defined as an implementation of BV suite utilizing the mDNSResponder - Bonjour software
implemented by Apple

**Avahi implementation is not planned**

BV - BonVoyage, suite name
LocalChat is an application providing simple messaging utility over mDNS.
FileSharing is an application providing simple file exchanging utility over mDNS.

There should be also a CLI tool offering the same functionality.
But maybe just focus on the GUI application.
First step is to allow sharing messages. ?And maybe encryption of these messages?

MAKE A CLEAR DISTINCTION BETWEEN DNS-SD AND MDNS!
Make sure that we are using hostname to IP resolution WITH mDNS, not any local DNS server.

## Classes (Components)
## Component 'BVService'
Description:
This class embeds the DNS-SD Service Registration functionality and
holds instance's service basic information:
    1. Service Registration (and handling the response from daemon),
    2. ?Discovery of Browsing and Registration Domains (Domain Enumeration),
       Although we will for now, use only .local (and should only use local?)
    3. Record handling (optionally)? <- I think we have to manage this!
    4. Service deregistration and deallocation of resources (DNSServiceRef).
       Question: does DNSServiceRef for DNSServiceRegister has to be deallocated
       after reading the reply from the daemon?

Parameters:
1. RegType: "The service type followed by the protocol, separated by a dot (e.g. "_ftp._tcp")."
2. Service Name: "specifies the service name to be registered."
3. Reply Domain: Always .local
Service has a fixed, constant type: **_localchathost._tcp**. Name is the host name providing this service.
Should this class be overall used for services, not just for registration?
We create an instance of it for the service that is registered by the host.
Next we will be resolving the discovered services.

Question: How to make sure that the service with the specific hostname is not registered twice?
Is it done on the mDNS level?
Maybe we can detect that instance of LocalChat is already running with some OS mechanism?

## Component 'BVDiscovery'
Description:
This class embeds the DNS-SD Service Discovery functionality:
    1. Discovery of instances (hosts) of localchathost service.
    2. Resolving a host name of a service (to IPv4).

How to interface with asio? Discovery is an asynchronous task.
This is a good idea, BVDiscovery can be a function object.
Does BVDiscovery need its own io_context? Or does it need just a reference
to io_context from main? BVDiscovery utilizes external io_context.
How will discovery results from BVDiscovery be used?
Should they be stored in queue for consumption?
Who will consume them?
One reason is that what comes from BVDiscovery should be consumed and stored somewhere else.
Maybe one class that stores that information. Session?
Then UI, and component that will send messages/files can acquire const pointer to this BVSession.
Then, if new discovery results come, they will be put on queue and the consumer
can update its data with new results (check if they're in fact new).
Session always tries to read the queue.
BVSession can contain every information that is tied to a session.
How many bytes were transmitted?
How much hosts were discovered etc.

## Component 'BVActor'
Description:
An actor, an instance that acts like a user would.
Actor will be sending messages, keeping message history etc.

## Component 'BVApp'
BVApp is an abstract class for an application
that utilizes discovery results from discovery component.
It listens for new results by using condition variable and discoveryQueueMutex.
Upon queue not being empty, it consumes whatever there is and updates the serviceVector.

!Wydaje mi sie, ze nalezy zrobic pewna abstrakcje. dns_sd.h definiuje pare operacji, po ktorych nalezy czekac na odpowiedz od daemona.
Czy nie da sie zrobic jakiegos systemu ktory by byl abstrakcją tego? Czy to jest potrzebne?
Poniewaz teraz BVActor musi wyszukac, czy istnieje juz nazwa hosta o podanym serwisie. Jezeli istnieje, to nie rejestruj jeszcze raz.
W zasadzie taka abstrakcja wymagana bylaby, w momencie gdybym chcial rozszerzyc funkcjonalnosc mDNSResponder albo funkcji, ktore
wymagaja kontaktu z demonem byloby bardzo duzo.

Byc moze najpierw zrobmy te funkcjonalnosc w BVDiscovery, gdzie BVActor zarzadza wywolywaniem tych dwoch funkcjonalnosci.
To ma swoj koszt - musimy rozroznic znow pomiedzy Bonjour i Avahi.
Chyba, ze najlepiej zrobic BVActor ktory dziala niezaleznie od implementacji. Jednakze, najlatwiej bedzie zrobic prototyp, gdzie bedzie klasa
BVActor_Bonjour i BVActor_Avahi

## Component 'BVTCPConnection'
Establishes a TCP connection between two hosts.
Main question: when is it established?
I send someone a message - TCP socket is created.
When the TCP connection is to be made - DNS resolution is made.
Remember to disallow to interact with service registered on the same machine.
How to announce that client disconnects? This should be a multicast/broadcast message
so that BVApps can update their UI.
Separate ioContext for each BVTCPConnection?

# Resolution
What exactly does 'Resolve' in mDNS mean:
**Resolve a service name discovered via DNSServiceBrowse() to a target host name, port number, and txt record.**

Am I doing this correct passing host name as service name?
Yes, if it is not passed, the name of the host is chosen.

Okay, little confusion:
DNSSD Resolution:
 * Resolve a service name discovered via DNSServiceBrowse() to a target host name, port number, and
 * txt record.
If we already passed host name as the name of the service:
""_Service" part can be concatenation of name + host"
then why resolve the service name to target host name?
Port number is understandable.
It seems that we already have the information needed and we do not need to resolve.
And next: why is there
gethostbyname
function mentioned?
Okay - Resolution doesn't mean resolve the hostname to IP address. It means:
Resolve a service name to target host name, port number and txt record
DNS-SD part is doing it's thing. It is not strictly mDNS functionality, but
mDNS and DNS-SD are paired to provide this functionality of connecting hosts.
passing a hostname on .local domain should result in mDNS query.

So:
We register a service name characterized by hostname and registered type on the .local domain.
DNS-SD resolves a service name onto target host name port number and txt record.
This target host name on the .local domain can be resolved by mDNS onto an IP address.
Wikipedia:
"By default, mDNS exclusively resolves hostnames ending with the .local top-level domain"

# Connection Type
P2P? There isn't really any server to be expected. Maybe when it comes to

# Session Structure
What data a session has?

## Sockets
Maybe two sockets. One for accepting connection and other for receiving/sending data.
Does a client need a passive socket (acceptor)?

# Async
Does async apart from that it doesn't block means that the boost asio creates a thread
that handles the operation execution? Yes.
Target functionality is to have a bunch of people messaging/trying to send files simultaneously.

Record handling?

## GUI library:
FTLK or raygui
wxWidgets

## Logging
Build with maybe different logging levels.

## Testing
Functional tests.
"Live tests" -> after the application finished (code is delivered) build for
testing/debug and look at the logs if everything is ok.

## How BV operates
## LocalChat
# Application Overview
LocalChat serves as a messaging utility over local network (LAN).
It utilizes mDNS and DNS-SD networking protocols in order to connect OS agnostic hosts in the same network,
without setting up a DNS server, or utilizing any other external server, that would exchange messages,
manage sessions and provide other utilities, required from this type of entity.
LocalChat uses FLTK to provide a GUI for a user; a panel with available hosts within the network,
text field, and other widgets.

# LocalChat flow
1. Initialization of BVService_Bonjour (for now class that supports only Bonjour)
2. Registration of service (TODO: provide complete service name) (_localchat._tcp.local?),
   where .local is mandatory, because mDNS exclusively resolves hostnames ending with the .local top-level domain[^1].
   [^1]: [Wikipedia on mDNS](https://en.wikipedia.org/wiki/Multicast_DNS#Protocol_overview)
   1. If service was already registered, do not register it twice. (TODO: Should this application work in background?)
      meaning, if someone writes a message to a user, where their application was closed, (but not the service)
      should they receive the messages? -> Closing the application means closing the service.
3. UI initialization. This can mean a CLI or GUI aplication. (TODO: Build for different targets?)
4. Main Program Loop:
   1. Accept user input AND
   2. Discovery for localchat services periodically (timer) AND
   3. Notify user when someone is available to chat AND (Thread handling this will block until specific ?message? comes)
   4. Output messages sent to user (callback upon receiving data on socket)
   Main problem is, that discovery for localchat services etc. needs to call callbacks/completion handlers upon
   finished task.

# Constants
How many hosts can one user discover max?
128?

# Threading
## Boost threadpool
boost::threadpool allows for the automatic thread management.
## Threads needed
First should take care of registration and announce if it was succesful or not.
It has to register the service with hostname, and wait for the daemon to reply.

Second thread handles GUI.

Third thread handles communication over TCP.

## Misc
Instead of writing BV(...) use a namespace

### Implementation roadmap

We have to start simple.
One other client that registers its service and communication with them, while continuously browsing.
No avahi support is planned for now.
Test on macOS first.