#### For now, keep this in a separate file
# Architecture
Every Component in Application needs a coordinated and well defined matter in which it operates. Without it, we will not be able to manage jobs, starting up, shutting down and ensure a safe way of sharing data between threads.
## Components interaction
The three components - Discovery, Service and App have to know how to interact with each other (perform tasks) in a way that doesn't introduce data races on data they share.
Discovery can start and stop - it has to listen to App demand.
Discoevry puts discovery instances onto the queue - it has to inform the App somehow

## Component interfaces
Discovery and Service have to have some basic **interface** which provides way to request changes in how they operate.
Discovery will Start/Stop. It will also try to access a shared ThreadSafeQueue.
Service will also Start/Stop.
App will also Start/Stop.

## Component communication
Will they need some manager class outside App?
Or this Manager/Synchronizer will be an orchestrator of the whole application?
MessageQueues? Listeners? Subscribers?
There's separation into independent threads. (C++ Concurrency In Action section 4.4.2)
That requires to be ensured that no data is shared between the threads other than through message queues.

"Using synchronization of operations to simplify code":
*"Rather than sharing data directly between threads, each task can be produced with the data it needs, and the result can be disseminated to any other threads that need it through the use of futures."*
I don't want to use async programming here.
But maybe Subscriber/Observer architecture in which each Observer acts upon an event type passed to the queue. So each message is actually broadcasted, but only if the Observer has Subscribed to the specific message event type, the callback/action is called.
Plus we can orchestrate everything with the FSM Finite State Machine. This is because we don't want to Stop (free) the Discovery component twice, we don't want to Register a service twice etc.. Maybe each component has a 'brain' - a SFM?
If they all receive a shared pointer to the queue, they cannot consume anything from queue. If the message is broadcasted, how to know that we can clear/consume the element?