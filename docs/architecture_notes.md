#### For now, keep this in a separate file
# Architecture
Every Component in Application needs a coordinated and well defined matter in which it operates. Without it, we will not be able to manage jobs, start up, shut down and ensure a safe way of sharing data between threads.
## Components interaction
The three components - Discovery, Service and App have to know how to interact with each other (perform tasks) in a way that doesn't introduce data races on data they share.
Discovery can start and stop - it has to listen to App demand.
Discovery puts discovery instances onto the queue - it has to inform the App somehow.
Service is not put on a new thread.
### Discovery queue handling
**What Discovery does (who produces the items to be put on queue)**
Discovery polls the DNS-SD results and appends it to a queue.
Discovery should not directly (not in a non thread-safe manner) push to the queue.

**Problem?**
Isn't bonjour discovery component blocked when there's no browsed instances?
How will it react to the shutdown? It has to shutdown gracefully ie do the whole procedure
of deallocating the dnsRef
How the components simultaneously wait on queue and do things?
I think it can be solved with two threads per each Component.
Also App Component has this problem.
Bonjour could manage two threads.
One is the discovery thread - it performs DNS-SD Discovery functionality and
it has a handle to the Broker object in order to message App.

**Who consumes the queue**
Application itself should not consume the items.
It should query the queue.

## Component communication
There's an idea of separation into independent threads. (C++ Concurrency In Action section 4.4.2)
That requires to be ensured that no data is shared between the threads other than through message queues.

"Using synchronization of operations to simplify code":
*"Rather than sharing data directly between threads, each task can be produced with the data it needs, and the result can be disseminated to any other threads that need it through the use of futures."*
I don't want to use async programming here.
We can orchestrate everything with the FSM Finite State Machine. This is because we don't want to Stop (free) the Discovery component twice, we don't want to Register a service twice etc.. Maybe each component has a 'brain' - a SFM?
There cannot be just one global queue: they all possess a shared_ptr to each of their queues and share it with the Broker. Or maybe there be MsgIn queue, to which every Component can put items, and only Broker can pop.

### ?ACK/NACK/STATUS INFORMATION?
?Some actions need confirmation from another component?

## Component Execution Flow Brief
They all have member functions corresponding to the action
that is taken upon receiving a message.
These functions, if applicable, *try* to change the State of the component.
They are completely unaware of the state diagram.
If a component cannot change state (or change from state A->A is forbidden),
it is a FSM responsibility to prevent that.
They also provide a way to communicate with a broker (send message)

## Component Interface
*Rationale:*
Discovery and Service have to have some basic **interface** which provides way to request changes in how they operate.
Discovery will Start/Stop. It will also try to access a shared ThreadSafeQueue.
Service will also Start/Stop.
App will also Start/Stop.

Component interface defines member variables and functions
that facilitate interactions:

**member vars**:
shared_ptr to ThreadSafeQueue outMailBox_p
shared_ptr to ThreadSafeQueue inMailBox_p

**member functions**:
1. OnStart = abstract
2. OnShutdown = abstract
3. OnRestart? = abstract
4. Send(Message) = concrete, public => Send Message through mailbox to Broker
6. DeAttach(Broker)

???
1. RequestStart    -> concrete, virtual
2. RequestShutdown -> concrete, virtual
3. RequestRestart  -> concrete, virtual
4. RequestStatus?  -> concrete, virtual

Besides that, the concrete implementation of the Component also defines functions
that "react" to the other messages.

## Finite State Machine class

## Broker class
Maintains a list/hashtable of subscribers and their mailboxes (ThreadSafe queues).
Another idea: It keeps their mailboxes, but also inMailbox, where it does not put any message,
instead: every component puts their message there.
Key-> eventType
Allows to subscribe/unsubscribe to Event (MessageType).
Passess only messages to subscribers that are interested in them.
Each mailbox is a shared_ptr between a Component and Broker

**Problem?**
Broker now has to wait on N queues.
It will spawn multiple threads.

**member vars**:
std::vector of std::pair of Subscriber, Mailbox (ThreadSafeQueue)
uint8_t numOfSubscribersRegistered

**member functions**:
SendMessage(Message)
Subscribe(subscriber, const std::array of Events)
Attach(Broker) = concrete => Create and share a MailBox and register a subscriber ID.
Unsubscribe(subscriber, const Event)

## Subscriber struct
ID (uint8_t or std::string)
std::vector of EventTypes

## Message struct
EventType
unique_ptr to std::any data

### How broker works
### Subscription
Subscription is nothing but pushing an enum symbolising
an event type, which subscriber wants to be notified on,
to a vector of EventTypes.
### Receiving a message (Component -> Broker)
Message comes from a Component to Broker through their dedicated channel (shared_ptr to mailbox).
Another idea: it can come to a dedicated msgIn channel shared between any attached components.
Broker does not publish any message to this channel.

### Sending a message
Upon receiving a message, broker must now gather everyone that is interested in this.
One idea: it iterates over the subscribers and their subscribed topics.
When it finds a subscriber, it puts this message to its mailbox.
It's probably slow.
**Every message is const by default**

Broker passess (copies) a message ?from the entry queue (its mailbox) to the
specific component queue based on the message type subscribed?

## Component Execution Flow Specific

### Application Starts

**Init Sequence (App OnStart())**
Service put on separate thread.
Discovery put on separate thread.
App on main thread.

Service subscribes to Events: StartDiscovery/StopDiscovery/Shutdown
Discovery subscribes to Events:
App subscribes to Events:
App Starts.
App requests Service Registration:
1. App sends message (Request Service Registration; Start Service)
2. Broker forwards the message to the Service 
3. Service waits on the queue and receives the message
4. Service OnStart() happens (service.Register()) (does Service send an ACK?)
5. Service sends status message
6. If status message: BVStatus:OK -> App sends message (Request Discovery Start). If status message: BVStatus:ERROR/NOK -> Error is displayed and back to step 1
7. OK: Discovery waits on queue and receives the message
8. Discovery OnStart() happens (?Discovery spawns a discovery thread?).
9. Discovery sends an ACK to App
10. If everything is ok, Init is complete. If not, Error is displayed and user has the option to start discovery once again.

**End Init Sequence**

**Discovery publishes a discovery event Sequence**
