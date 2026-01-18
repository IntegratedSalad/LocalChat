# Broker and Components Architecture Tests
We need to test how Broker and different Components interact with each other.
From creating/destroying objects themselves, to Attaching/Detaching to/from the Broker and Subscribing/Unsubscribing to different event types to scenarios which will be happening in the real application.
These will be probably integration tests.
Functional tests will test member functions of the classes, especially those who operate on containers,
to check if they're behaving in an expected way.

## Setup tests

## Simulating Component Behaviour
### Simulating BVService

### Simulating BVDiscovery

### Simulating BVApp

## Communication tests
### Scenarios
We need to test scenarios that are possible and their suspected outcome.

## Integration tests/Behavioral? tests

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

## Capacity tests

## Live
# VM
Fedora Linux -> compile mDNS and LocalChat.
Ensure that VM has a separate IP Address.