0.
1. Host registering a service ._localchathost_tcp service. [V]
2. Discovering services on the local net. [V]
   a. Plans:
   1.1a -> Write Discovery regardless of the architecture [V]
   1.2a -> Test if it works on mac and linux [V]
   1.3a -> Write Discovery interface, an abstraction that provides functionality
           regardless of implementation [V]
   1.4a -> Test if it works on mac and linux [V]
   1.4b -> If possible and quick - write quick tests that would test:
           A. Registration - mocking the response from the daemon (somehow)
           B. Discovery - mocking the response from the daemon [V]
   1. Stopping and restarting the Registration functionality [V]
      2.2 Needs an architectural rewrite. [V]
   2. Stopping and restarting the discovery functionality [V]
   3. Deregistration [V]
3. Choosing one service and sending it a string of bytes. [V]
   1. Resolving the host with DNSServiceResolve [V]
4. Multiple sessions opened [V]
   1. Better observability []
   2. Console interface for viewing sessions - messages and message history []
   3. Notification printed on screen that there are new messages []
5. Sending files []
6. Messages are 

7. 
Release:
1. Console Chat application that handles multiple sessions and allows for file exchange []