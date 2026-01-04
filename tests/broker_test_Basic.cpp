/*
    We can test broker and components initialization
    and communication with fake Discovery and App components "talking".
    We can simulate them doing their job (waiting) and scenarios.
    We can assert that their mailboxes are shared across threads.
    We can assert that no thread is starved.

    We also can derive from BVDiscovery and BVService and BVApp and
    override the functions that perform DNS-SD to simulate their behavior,
    not performing the actual DNS-SD, but simulating the outcome of 
    their functionality.

    1. Test attaching and detaching Components from Broker
*/