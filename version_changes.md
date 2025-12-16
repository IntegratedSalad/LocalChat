0.1:
1. Host registering a service ._localchathost_tcp service. [V]
2. Discovering services on the local net. [V] (2.a also linux avahi [])
   1. Stopping the discovery service []
       2.1 Needs an architectural rewrite.
3. Choosing one service and sending it a string of bytes. []
   1. Resolving the host with DNSServiceResolve

0.2:
2.
3. Local Chat console application - sending and receiving text messages with active hosts.
4. Announcing that hosts has disconnected - closed the application.
