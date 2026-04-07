# Architecture
Every Component in Application needs a coordinated and well defined matter in which it operates. Without it, we will not be able to manage jobs, start up, shut down and ensure a safe way of sharing data between threads.

## Components interaction
The three components - Discovery, Service and App have to know how to interact with each other (perform tasks) in a way that doesn't introduce data races on data they share.
A Component is a class, which facilitates a way to communicate using two mailboxes:
1. outMailbox - shared queue that a component pushed Messages to,
2. inMailbox - shared queue that a component consumes Messages from.

## Message
Message contains a BVEventType and data, which is a unique pointer to std::any.
The callback which utilizes data pointer (dp) needs to cast it to the expected type.

## Component Interface
*Rationale:*
Discovery and Service have to have some basic **interface** which provides way to request changes in how they operate.
Discovery will Start/Stop, it can be paused and resumed.
App will also Start/Stop.

Component interface defines member variables and functions
that facilitate interactions:

**member vars**:
shared_ptr to ThreadSafeQueue outMailBox_p
shared_ptr to ThreadSafeQueue inMailBox_p
std::thread mailbox_thread;
std::unordered_map of events and callbacks

**member functions**:
1. OnStart = abstract
2. OnShutdown = abstract
3. OnRestart? = abstract
4. Send(Message) = concrete, public => Send Message through mailbox to Broker
5. Detach(Broker)
6. RequestStart    -> concrete, virtual
7. RequestShutdown -> concrete, virtual
8. RequestRestart  -> concrete, virtual
9. RequestStatus?  -> concrete, virtual

Besides that, the concrete implementation of the Component also defines functions
that react to other messages.
Every callback is registered to an event (set to be called upon receiving message of the said event type)
with RegisterCallback() function.
The main thread of the component will publish messages to the outMailbox_p, as result
of the logic it implements.
In conclusion, Component class manages the state of the BVX object.

## Broker class
Maintains a list/hashtable of subscribers and their mailboxes (ThreadSafe queues). It works in a separate thread and lives throughout the application.
Key -> eventType
Allows to subscribe/unsubscribe to Event (MessageType).
Passess only messages to subscribers that are interested in them.
Each mailbox is a shared_ptr between a Component and Broker.
It terminates when it reads BVEVENTTYPE_TERMINATE_ALL - it finishes everything (empties the queue), sends BVEVENTTYPE_TERMINATE_ALL to everyone and terminates itself.
This object should be set to never Restart.

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

## How broker works
### Attaching
Attaching means connecting mailboxes of the Component and setting up the mailbox_m at the Subscriber ID of the component.
### Subscription
Subscription is nothing but pushing an enum symbolising
an event type, which subscriber wants to be notified on,
to a vector of EventTypes.
### Receiving a message (Component -> Broker)
Message comes from a Component to Broker through their dedicated channel (shared_ptr to mailbox).
It is then routed to Components that subscribed to the message type (event type).

## Thread Model (Message Passing model)
Every thread is created and joined by the main thread.
Each component has a worker thread and a mailbox thread, where mailbox thread controls the worker thread,
as it receives messages coming from other components. It is facilitated by the Component class.
In order to pass data from Component to Component, each object that inherits BVComponent calls
SendMessage() function, passing the event type and data pointer, if applicable.
