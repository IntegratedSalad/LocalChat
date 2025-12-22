0.
1. Host registering a service ._localchathost_tcp service. [V]
2. Discovering services on the local net. [V]
   a. Plans:
   1.1a -> Write Discovery with Avahi regardless of the architecture [V]
   1.2a -> Test if it works on mac and linux [V]
   1.3a -> Write Discovery interface, an abstraction that provides functionality
           for writing to a queue regardless of implementation []
   1.4a -> Test if it works on mac and linux []
   1.4b -> If possible and quick - write quick tests that would test:
           A. Registration - mocking the response from the daemon (somehow)
           B. Discovery - mocking the response from the daemon
   1. Stopping and restarting the Registration functionality []
      2.2 Needs an architectural rewrite. []
   2. Stopping and restarting the discovery functionality []
3. Choosing one service and sending it a string of bytes. []
   1. Resolving the host with DNSServiceResolve []
4. Multiple sessions opened []
5. Sending files []

1. 
Release:
1. Console Chat application that handles multiple sessions and allows for file exchange []