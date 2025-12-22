# Design of BonVoyage FileSharing (LocalChat)
Because what I will write here will be used for BonVoyage, a graphical client for sharing files, we should
design an interface that takes out some of the core functionality of DNS-SD.

## Concepts
**mDNS** - Multicast DNS. It resolves hostnames to IP addresses **without** any DNS server configured.
**DNS-SD** - DNS Service Discovery. in both *Bonjour* and *Avahi* implementations, mDNS and DNS-SD functionality are paired. [DNS-SD](https://en.wikipedia.org/wiki/Zero-configuration_networking#DNS-based_service_discovery)
**BV** - BonVoyage, name of the suite
**LocalChat** - Name of the application utilizing the BonVoyage suite and providing messaging and file sharing over LAN with mDNS.
**Bonjour** - Apple mDNSResponder zero-conf implementation that serves as a publisher of mDNS information
**Avahi** - Native Linux API implementing mDNS zero-conf implmentation and DNS-SD.

## dns-sd daemons
The Bonjour and Avahi provide ways to communicate with
mDNSResponder daemon and avahi-daemon background processess
respectively, listening on port 5353 for multicast DNS.
These multicast DNS messages contain information of specific
DNS queries **TODO: describe the messages, queries, records, PTRs etc...**

To verify:
Run
sudo tcpdump -ni any port 5353 | grep _localchathost
And
sudo tcpdump -ni any port 53 | grep _localchathost
and confirm NO traffic concerning the regtype *_localchathost* is present
when running the application

**Avahi implementation is in progress**

There should be also a CLI tool offering the same functionality.
But maybe just focus on the GUI application.
First step is to allow sharing messages. ?And maybe encryption of these messages?



MAKE A CLEAR DISTINCTION BETWEEN DNS-SD AND MDNS!
Make sure that we are using hostname to IP resolution WITH mDNS, not any local DNS server.

### Sources
[Wikipedia on mDNS](https://en.wikipedia.org/wiki/Multicast_DNS)
[multicastdns.org](https://www.multicastdns.org/)
[RFC6762](https://datatracker.ietf.org/doc/html/rfc6762)
[RFC6763](https://www.ietf.org/rfc/rfc6763.txt)
[mDNSResponder repository](https://github.com/apple-oss-distributions/mDNSResponder)
[Avahi main site](https://avahi.org/)

## Concepts
Component - a class providing a functionality eg DNS-SD Registration or DNS-SD Browsing
Functionality

## Classes (Components)
## Component 'BVService'
Description:
This class embeds the DNS-SD Service Registration functionality and
holds instance's service basic information:
    1. Service Registration (and handling the response from daemon),
    2. ?Discovery of Browsing and Registration Domains (Domain Enumeration),
       Although we will for now, use only .local (and should only use local?)
    3. Record handling (optionally)? <- I think we have to manage this!
    4. Service deregistration and deallocation of resources, ultimately:
       stopping the service from being browsed.

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
      a. Starting a discovery
      b. Stopping the discovery
      c. Putting the results onto a Queue
    ? 2. Resolving a host name of a service (to IPv4).?

### Data exchange
How will discovery results from BVDiscovery be used?
Should they be stored in queue for consumption?
Who will consume them?
### Discovery Queue
BVDiscovery main goal is to populate a discoveryQueue
(non-priority queue, with thread-safe operations)
with structures that contain information about services discovered via
dns-sd functionality over multicast.
Discovery component (class) should provide the functions for
starting/stopping the component functionality being executed.
This is done by making sure that the connection context of a particular implementation
is allocated and alive for the duration of discovery functionality.
It would be probably better that discovery queue
is not managed *per se* by Discovery component, but it can 'query' a thread-safe queue object that would put the new discovery object on the queue on discovery object' behalf.
### Discovery Thread
Discovery object ?should? run in a separate thread as it will need to block waiting for the response from the daemon about a new service being discovered.
### Shutdown procedure
We have to make sure that the shutdown of the discovery object,
either permanent (user closing the application) or momentary
(user toggling the discovery off) is announced, synchronized and
executed with the consumer in

How many bytes were transmitted?
How much hosts were discovered etc.

## Component 'BVActor'
Description:
An actor, an instance that acts like a user would.
Actor will be sending messages, keeping message history etc.

## Component 'BVApp' (A manager class)
BVApp is a class with virtual methods that helps define a GUI framework
while providing basic functionality for managing sub-components.
Managing subcomponents:
1. It can Register/Deregister a service.
2. It can start/stop browsing
3. It can start/stop watching the browser queue.
It manages these components by sending messages to them via a shared queue.
These objects have defined states, as to behave in a defined manner.



!Wydaje mi sie, ze nalezy zrobic pewna abstrakcje. dns_sd.h definiuje pare operacji, po ktorych nalezy czekac na odpowiedz od daemona.
Czy nie da sie zrobic jakiegos systemu ktory by byl abstrakcją tego? Czy to jest potrzebne?
Poniewaz teraz BVActor musi wyszukac, czy istnieje juz nazwa hosta o podanym serwisie. Jezeli istnieje, to nie rejestruj jeszcze raz.
W zasadzie taka abstrakcja wymagana bylaby, w momencie gdybym chcial rozszerzyc funkcjonalnosc mDNSResponder albo funkcji, ktore
wymagaja kontaktu z demonem byloby bardzo duzo.

## Component BVApp_ConsoleClient
BVApp_ConsoleClient implements BVApp.
It is a CLI application that is meant to be a preview and/or playground environment
that provides simple functionality of exchanging messages between hosts.
1. User has a list of 10 max available hosts he can message.
   1. They're appearing regardless of action of the user - the list is updated concurrently.
2. User has a prompt '>> ' and he can execute several instructions.
   1. They can (S)witch to the host and it's messages.
   2. They can send the (M)essage
   3. They can (E)xit the application.
   4. They can (P)rint the screen again

Execution loop:
1. User executes the application.
2. Main screen is printed. It has list of connected (registered) services.
   Next to the service, there's a string: '(X) messages unread' if the host sent the user a message.
3. User is idle. The discovery queue has been updated.
   1. The app is notified about the event
   2. Screen is refreshed with the new positions.
4. User chooses (S)witch to the host.
   1. The screen is redrawn. It shows message history.
   2. The host doesn't disconnect.
      1. User writes a message
      2. Host (receiver) receives the message
      3. Host (sender) sends message to the user
      4. Message is printed.
      5. User (S)witches screen to main OR host disconnects
   3. Host disconnects
      1. Information is printed that user closed the app.
      2. (M)essaging does nothing
5. User (E)xits.

discoveryQueue:
Queue is updated (items are pushed into this queue) by BVDiscovery component.
Items are consumed by BVApp_ConsoleClient.
Another thread that waits and prints the updated services?

/ discoveryQueueMutex is locked by discovery component
=> services are put to discoveryQueue
/ discoveryQueueMutex is unlocked by discovery component
/ discoveryQueueMutex is locked by app
/ vectorMutex is locked by thread handling discoveryQueue
=> vector is updated
/ discoveryQueueMutex is unlocked by app
/ vectorMutex is unlocked by thread handling discoveryQueue
=> app event 'updated vector' is sent
 => ?services? ?app event handler? thread executes function responsible for handling this -> printing (performing std I/O op.)
 => function is called ->
  / vectorMutex doesn't have to be locked here? What if the vector will be updated again? Maybe lock it.
  => vector is printed
  / vectorMutex is unlocked
What if the user writes to stdout and the function that is a handler of 'updated vector' is called
at the same time? what happens with the stdout?
If the user is typing something and then the screen gets redrawn by the handler, what happens?
Maybe these operations are directly executed upon receiving a message.
So -> (P)rint sends message, handler sends a message (to update the screen)
So these are put on the queue and executed sequentially and there's only one class managing the I/O queue,
that performs printing to the screen.
It listens for messages and it only performs I/O.
So - any action in APP component involving discoveryQueue,vector and I/O is a request in form of a message to the queue.
There's a class that handles discoveryQueue and I/O - discoveryQueue has items - there's a message sent.
Update vector - message.
User wants to print something - message.
There has to be an update on screen - message.
User chooses some host (accesses vector) - message.
App has exclusive access to the I/O

Maybe for now, let's just see which problems we are facing.

## Component 'BVTCPConnection'
Establishes a TCP connection between two hosts.
Main question: when is it established?
I send someone a message - TCP socket is created.
When the TCP connection is to be made - DNS resolution is made.
Remember to disallow to interact with service registered on the same machine.
How to announce that client disconnects? This should be a multicast/broadcast message
so that BVApps can update their UI.
Separate ioContext for each BVTCPConnection?
Maybe utilize thread pool. For each connection, just take unused thread.

# Resolution
What exactly does 'Resolve' in mDNS mean:
**Resolve a service name discovered via DNSServiceBrowse() to a target host name, port number, and txt record.**

Am I doing this correct passing host name as service name?
Yes, if it is not passed, the name of the host is chosen.

Okay, little confusion:
DNSSD Resolution:
 * Resolve a service name discovered via DNSServiceBrowse() to a target host name, port number, and
txt record.

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
2. Registration of service
   where .local is mandatory, because mDNS exclusively resolves hostnames ending with the .local top-level domain[^1].
   [^1]: [Wikipedia on mDNS](https://en.wikipedia.org/wiki/Multicast_DNS#Protocol_overview)
   1. If service was already registered, do not register it twice.
      meaning, if someone writes a message to a user, where their application was closed, (but not the service)
      should they receive the messages? -> Closing the application means closing the service.
3. UI initialization. This can mean a CLI or GUI aplication.
4. Main Program Loop:
   1. Accept user input AND
   2. Discovery for localchat services periodically (timer) AND
   3. Notify user when someone is available to chat AND
   4. Output messages sent to user (callback upon receiving data on socket)

(TODO: Should this application work in background?)
How to ensure that an application works in the background?

# Constants
How many hosts can one user discover max?
128?

# Threading
## Best practices [^2]
**SRP - Single Responsibility Principle**
Method/class/component should have a single reason to change.
[^2]: Taken from the Clean Code by Robert C. Martin

**Limit the scope of data**
Limit the number of critical sections where two or more threads modify the data within the section.
Severely limit the access of any data that may be shared.

**Use copies of data**
In some situations it is possible to use copies of data and treat them as read-only.

**Threads should be as independent as possible**

## Boost threadpool
boost::threadpool allows for the automatic thread management.
## Threads needed

## Producers and consumers
## Console client
### Producers
### Consumers
## Misc
Instead of writing BV(...) use a namespace