# Implementations
Base class facilitates virtual functions that help setup, tear down, start discovery and process its results.
However, because there are two DNS-SD implementations, how discovery is paused, resumed and how it performs DNS Service Browsing varies.
Both implementations concern a FD (file descriptor) on which file the appropriate daemon puts browsing results.
Reading from it means processing the DNS Service Browse results, and counts as knowing about these services (they won't be rediscovered).
When the Discovery is paused, nothing should be read. Because we do not send a BVEVENTTYPE_APP_PUBLISHED_SERVICE message, and thus App is not updated, there would be a possibility of not Browsing the missed Services upon resuming the Discovery.

**General flow**
![diagram](discovery_diagram.wsd)
This diagram represents the abstraction of how Discovery operates.

## mDNSResponder
mDNSResponder does not provide an event loop, and io_context object from Boost.ASIO serves as such.
Browsing starts by getting a **socket FD** by calling DNSServiceRefSockFD.
Next, we await the socket read readiness, and we process the Browse Result when it is ready.
If App requests pause, we cancel awaiting socket readiness, do not AwaitFD no more, and instead we schedule
a pause timer.
Whenever App requests resumation, we then cancel the pauseTimer and resume Awaiting the FD.

## Avahi
Avahi provides a base SimplePollLoop which can be _iterated() on. This differs from calling avahi_simple_poll_loop() - that way we couldn't have any control over where to not process (poll the FD for readiness).
When browser is instantiated, we enter a loop to check if Discovery is paused. If it isn't we call
avahi_simple_poll_iterate with a timeout of 500 ms.
If App requests pausing the Discovery, we awake the timeout, so that the function returns, and block with a conditional variable (pause).
When App requests resumation, we set isPaused to false and notify the CV. This resumes the loop.

