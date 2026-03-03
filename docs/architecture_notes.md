#### For now, keep this in a separate file
# Architecture
Every Component in Application needs a coordinated and well defined matter in which it operates. Without it, we will not be able to manage jobs, start up, shut down and ensure a safe way of sharing data between threads.

**TODO:** Make diagram of every flow in the program.
How App reacts? What is being passed? How it is packed etc...

## Components interaction
The three components - Discovery, Service and App have to know how to interact with each other (perform tasks) in a way that doesn't introduce data races on data they share.
Discovery can start and stop - it has to listen to App demand.
Discovery puts discovery instances onto the queue - it has to inform the App somehow.
Service is not put on a new thread.
Maybe it is put on the same thread as Discovery.

## Key rules
Each piece of mutable state, so a component and its data, has exactly one owning thread.

**TODO:** : No longer a queue, update this
### Discovery queue handling
**What Discovery does (who produces the items to be put on queue)**
Discovery polls the DNS-SD results and appends it to a queue.
Discovery should not directly (not in a non thread-safe manner) push to the queue.
**Important**
When there is message passing, we can directly include the list in the
message! Should we really need a queue right now?
Queue was needed, when there wans't any message passing and just notifying that the queue has been updated.

**Problem?**
Isn't bonjour discovery component blocked when there's no browsed instances?
How will it react to the shutdown? It has to shutdown gracefully ie do the whole procedure
of deallocating the dnsRef
How the components simultaneously wait on queue and do things?

**I think it can be solved with two threads per each Component.**

*We do not want to create two threads per component, because we will need to synchronize*
*them with each other.*
*We have to somehow make use of ASIO framework, to manage async tasks that Discovery is making.*
^ This is not that straightforward: test first.

Discovery is very simple. It performs DNS-SD, and when it finds Services, it appends them to the discovery queue.
It can then inform the Components which subscribe to the specific message.

### Asio Use
We do not make very good use of ASIO - simply using it for managing communications is not ideal.
Seems like I'm creating problems and workarounds around a solution that is maybe complex, but really solid.
We need to somehow listen for messages and perform 'business logic' (DNS-SD here) concurrently.
Keeping it how it is now, will likely cause huge problems.

I think that we can put any work that is done periodically to the ASIO loop, on the io_context.
We can then define cancellation logic for it.
Still, it doesn't solve the component waiting for messages on a separate thread.
Still, Avahi has a separate logic for handling operations, a poll loop.

Maybe there's a need for the execution context to post tasks.
What it means, is that there is no two threads now, but one:
Discovery and App now, because they operate on blocking I/O, don't need separate threads
to monitor the queue and do their own things.
Reacting upon receiving message now won't involve 2 threads and syncing them together.
There's even a cancellation mechanism written, so for cancelling we have a library support.

**Who consumes the Discovery queue**
Application itself should not consume the items.
It should query the queue.

## Component communication
There's an idea of separation into independent threads. (C++ Concurrency In Action section 4.4.2)
That requires to be ensured that no data is shared between the threads other than through message queues.

"Using synchronization of operations to simplify code":
*"Rather than sharing data directly between threads, each task can be produced with the data it needs, and the result can be disseminated to any other threads that need it through the use of futures."*
We should rely on async programming (but not coroutines).
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
std::thread mailbox_thread;
std::unordered_map of events and callbacks

bool (atomic?) to see if we're processing the queue (and stop it).

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
Think of maybe adding a functionHandler/Handler/FuncPointer to the type.
The main thread of the component will publish messages to the outMailbox_p, as result
of the logic it implements.
In conclusion, Component class manages the state of the BVX object.

## Finite State Machine class

## Broker class
Maintains a list/hashtable of subscribers and their mailboxes (ThreadSafe queues).
Another idea: It keeps their mailboxes, but also inMailbox, where it does not put any message,
instead: every component puts their message there.
Key-> eventType
Allows to subscribe/unsubscribe to Event (MessageType).
Passess only messages to subscribers that are interested in them.
Each mailbox is a shared_ptr between a Component and Broker.
It works in another thread.
?It terminates when it reads EVENT_TERMINATE_ALL - it finishes everything (empties the queue), sends EVENT_TERMINATE to everyone and terminates itself.?
This object should be set to never Restart.
It must live until the end of the application

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

**Another idea: it can come to a dedicated msgIn channel shared between any attached components.**
**^ This also can be a bad idea**
Somehow there's a callback posting architecture that can be better.
It utilizes io_context.

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
Service put on App main thread.
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

## Thread Model (Message Passing model)
Which threads are lauching/joining which threads, what data are they holding, what data are they changing, if they mutate the data of other threads, and what work they do:

**Thread name**
Main thread

**Thread created by**

**Thread joined when**

**Thread joined by**

**Thread terminated when**

**Thread name**
Discovery worker thread

**Thread created by**

**Thread joined when**

**Thread joined by**

**Thread terminated when**

**Thread name**
Discovery mailbox thread

**Thread created by**

**Thread joined when**

**Thread joined by**

**Thread terminated when**

## Problems and Important Things To Address
1. Two threads per blocking Component, and Broker sending messages
Blocking component is a component which requires some I/O operations.
For Discovery, it is performing ProcessDNSServiceBrowseResult (waiting for daemon to answer),
for App, it is taking input from the user and updating UI, text-based or GUI.
This is not an ideal approach, because then two threads operate on one state.
This was a solution for the Broker passing messages ('routing') - how to react upon a new message?
*Have another one checking the messages and changing the behaviour, while the other performs other logic*
**Solution: Probably find a way to post tasks tied to receiving a specific message to the execution loop with async model implemented in ASIO**
Another thread can post tasks.

1. Bonjour Discovery with a separate thread performing DNS-SD.
**Solution: use ASIO for dispatching tasks and retrieving results with completion handlers**

## Good ideas
Broker routing messages from component to components that subscribe to the messages.
Broker should be only to route message->component and post the task onto loop.
*It should not be put on a separate thread.*
This is called event-driven model.

## Architecture Verification and Notes about choosing the right architecture
This whole dilemma above about architecture being bad is not entirely right.
There's nothing wrong **inherently** about the two threads per component and synchronization of these threads.
Implementation matters, but what's the most important is to write tests and verify if this architecture supports what it was created for. If the tests are done right, and by that we mean that every execution flow is tested, every function is covered and outputs what we want, then this should follow when implementing main logic itself with UI.

We have to stop obsessing over architecture so much, because bad design and problems will come out with testing.

Write functional tests to test functionality (message passing, message constructing, subscribing/unsubscribing, attaching/detaching) and execution flow tests, to ensure that every action is scheduled and executed as planned (starting/stopping discovery, discovery notifying App and sending discovery results, stopping everything).