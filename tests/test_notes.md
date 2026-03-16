# Broker and Components Architecture Tests
We need to test how Broker and different Components interact with each other.
From creating/destroying objects themselves, to Attaching/Detaching to/from the Broker and Subscribing/Unsubscribing to different event types to scenarios which will be happening in the real application.
These will be probably integration tests.
Functional tests will test member functions of the classes, especially those who operate on containers,
to check if they're behaving in an expected way.

## Model
Mocks strip down the Components of their I/O tasks (querying the mdns daemon, polling the avahi loop or UI reading from stdin), and simulate their behavior to be tested for concrete execution flow.

## Mocks
MockDiscovery
MockApp

## Helper Components
TestHeartbeatComponent
TestHeartbeatListenerComponent
TCComponent

## Setup tests
Setup tests are carried out in component tests

## Component Tests

### Simulating BVService
TODO: Needed? as Service is just essentialy a flag.

### Simulating BVDiscovery
Done. MockDiscovery

### Simulating BVApp
Needed, will be tested in Communication test

### Broker
maybe broker_test_Basic in which we test:
1. Attaching to the broker
2. Detaching from the broker
3. Subscribing to an event type
4. Unsubscribing from an event type
5. Subscribing to a collection of event types
6. Unsubscribing from a collection of event types

## Communication tests (One Component / Multiple Components)
### Scenarios
We need to test scenarios that are possible and their suspected outcome.
This is utilized with a Broker, which needs to be tested first.

#### Discovery and App
This is a very important set of tests.
This integrates two crucial Components and represents them as Mocks,
and tests it with the Broker working.

For example App requests Shutdown/Restart when Discovery is continuously working.
App updates its list of the services

### Discovery and App and Service
TODO: Needed? as Service is just essentialy a flag.
TODO: Check if Avahi model requires Service to be put on another thread.

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

## Routing performance tests?
Test Components do something with a strict timing. Heartbeat must be acked within a short window or something


## Live
# VM
Fedora Linux -> compile mDNS and LocalChat.
Ensure that VM has a separate IP Address.