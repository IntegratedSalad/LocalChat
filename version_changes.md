0.1:
1. Host registering a service ._localchathost_tcp service. [V]
2. Discovering services on the local net. [V] (2.a also linux avahi [])
   a. Plans:
   1.1a -> Write Discovery with Avahi regardless of the architecture []
   1.2a -> Test if it works on mac and linux
   1.3a -> Write Discovery interface, an abstraction that provides functionality
           for writing to a queue regardless of implementation []
   1.4a -> Test if it works on mac and linux
   2. Stopping and restarting the Registration functionality
      2.2 Needs an architectural rewrite.
   3. Stopping and restarting the discovery functionality []
3. Choosing one service and sending it a string of bytes. []
   1. Resolving the host with DNSServiceResolve
0.2:
1. x
2. Local Chat console application - sending and receiving text messages with active hosts.
3. Announcing that hosts has disconnected - closed the application.
