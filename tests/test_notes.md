# Broker and Components Architecture Tests
We need to test how Broker and different Components interact with each other.
From creating/destroying objects themselves, to Attaching/Detaching to/from the Broker and Subscribing/Unsubscribing to different event types to scenarios which will be happening in the real application.
These will be probably integration tests.
Functional tests will test member functions of the classes, especially those who operate on containers,
to check if they're behaving in an expected way.

## Model
Mocks strip down the Components of their I/O tasks (querying the mdns daemon, polling the avahi loop or UI reading from stdin), and simulate their behavior to be tested for concrete execution flow.

## Setup tests
Setup tests are carried out in component tests

### Component
#### Discovery
TODO: Describe each tests

### Broker
maybe broker_test_Basic in which we test:
1. Attaching to the broker
2. Detaching from the broker
3. Subscribing to an event type
4. Unsubscribing from an event type
5. Subscribing to a collection of event types
6. Unsubscribing from a collection of event types

NEEDED?
## Simulating Component Behaviour (Behavioral tests)
Simulate receiving some event amidst some action.
Simulate without Broker.

### Simulating BVService

### Simulating BVDiscovery

### Simulating BVApp
NEEDED?

## Communication tests (One Component / Multiple Components)
### Scenarios
We need to test scenarios that are possible and their suspected outcome.
This is utilized with a Broker (TODO: needs to be tested first as a standalone object).

#### Discovery and App
For example App requests Shutdown/Restart when Discovery is continuously working.
App updates its list of the services

### Discovery and App and Service

## Validation tests?
Check if the Component has subscribed to one event, only this event is passed to it and handled by it!
So Subscribe() to event and evoke many many other events and this specific. Only this specific should be forwarded to the susbcriber.


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