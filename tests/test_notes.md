# Broker and Components Architecture Tests
We need to test how Broker and different Components interact with each other.
From creating/destroying objects themselves, to Attaching/Detaching to/from the Broker and Subscribing/Unsubscribing to different event types to scenarios which will be happening in the real application.
These will be probably integration tests.
Functional tests will test member functions of the classes, especially those who operate on containers,
to check if they're behaving in an expected way.

## Setup tests

### Component

### Broker

## Simulating Component Behaviour (Behavioral tests)
Simulate receiving some event amidst some action.
Simulate without Broker

### Simulating BVService

### Simulating BVDiscovery

### Simulating BVApp

## Communication tests (One Component / Multiple Components)
### Scenarios
We need to test scenarios that are possible and their suspected outcome.

## Integration tests?
Check if the Component has subscribed to one event, only this event is passed to it and handled by it!
So Subscribe() to event and evoke many many other events and this specific. Only this specific should be forwarded to the susbcriber.

## Model
Mocks strip down the Components of their I/O tasks (querying the mdns daemon, polling the avahi loop or UI reading from stdin), and simulate their behavior to be tested for concrete execution flow.

## System tests
They should check if flow of the application is working:
- Happy Path
- Something not working etc...

## Subsystem tests
They should check if parts of the system are working as intended:
- Communication between tests
- Different paths
- Pushing results to queue and consuming them
-

## Stability tests?

## Live
# VM
Fedora Linux -> compile mDNS and LocalChat.
Ensure that VM has a separate IP Address.