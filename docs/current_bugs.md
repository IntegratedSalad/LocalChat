[BUG0]
The sessionData->sessionID (SessionID) is assigned (it seems) non-atomically.
[LOGS]
[15:43:08 +02:00] [logger main_logger] [trace] [thread 5811141] Session [1]: Received _HELLOBACK_ with payload='fedora-thinkpad'. Calling Manager handler.
[15:43:08 +02:00] [logger main_logger] [trace] [thread 5811141] BVTCPConnectionManager: Established connection with node: fedora-thinkpad Address: ::ffff:192.168.0.74
[15:43:08 +02:00] [logger main_logger] [trace] [thread 5811141] BVTCPConnectionManager: Current sessions:
[15:43:08 +02:00] [logger main_logger] [trace] [thread 5811141] Session 0 : ID: __0__, service: fedora
[15:43:08 +02:00] [logger main_logger] [trace] [thread 5811141] Session 1 : ID: __0__, service: BupsioBup.local
[15:43:08 +02:00] [logger main_logger] [trace] [thread 5811141] Session 2 : ID: __1__, service: fedora-thinkpad

The sessionData->sessionID (SessionID) is assigned (it seems) non-atomically.

Services not found in vector:
15:43:49 +02:00] [logger main_logger] [trace] [thread 5811141] Discovery: DNSServiceProcessResult returned. Sending BVEVENTTYPE_APP_DEREGISTERED_SERVICE to App...
[15:43:49 +02:00] [logger main_logger] [trace] [thread 5811141] Discovery: Awaiting read readiness on the socket...
[15:43:49 +02:00] [logger main_logger] [trace] [thread 5811140] BVApp_ConsoleClient: HandleServiceDeregistration called
[15:43:49 +02:00] [logger main_logger] [warning] [thread 5811140] App, HandleServiceDeregistration: BupsioBup.local not found in serviceV!
[15:43:51 +02:00] [logger main_logger] [error] [thread 5811141] Session [0]: Error in ReadMessageFrame callback: 2, End of file
[15:43:51 +02:00] [logger main_logger] [debug] [thread 5811141] Read buffer:  Read buffer is a nullpointer: false Bytes read: 0 Bytes transferred: 0 Address in nodeData: ::ffff:192.168.0.132 Endpoint address: ::ffff:192.168.0.132 State: 1
[15:43:53 +02:00] [logger main_logger] [trace] [thread 5811141] Discovery: DNSServiceProcessResult returned. Sending BVEVENTTYPE_APP_DEREGISTERED_SERVICE to App...
[15:43:53 +02:00] [logger main_logger] [trace] [thread 5811141] Discovery: Awaiting read readiness on the socket...
[15:43:53 +02:00] [logger main_logger] [trace] [thread 5811140] BVApp_ConsoleClient: HandleServiceDeregistration called
[15:43:53 +02:00] [logger main_logger] [warning] [thread 5811140] App, HandleServiceDeregistration: fedora not found in serviceV!
[15:43:56 +02:00] [logger main_logger] [error] [thread 5811141] Session [1]: Error in ReadMessageFrame callback: 2, End of file
[15:43:56 +02:00] [logger main_logger] [debug] [thread 5811141] Read buffer:  Read buffer is a nullpointer: false Bytes read: 0 Bytes transferred: 0 Address in nodeData: ::ffff:192.168.0.74 Endpoint address: ::ffff:192.168.0.74 State: 1
[15:43:57 +02:00] [logger main_logger] [trace] [thread 5811141] Discovery: DNSServiceProcessResult returned. Sending BVEVENTTYPE_APP_DEREGISTERED_SERVICE to App...
[15:43:57 +02:00] [logger main_logger] [trace] [thread 5811141] Discovery: Awaiting read readiness on the socket...
[15:43:57 +02:00] [logger main_logger] [trace] [thread 5811140] BVApp_ConsoleClient: HandleServiceDeregistration called
[15:43:57 +02:00] [logger main_logger] [trace] [thread 5811140] App, HandleServiceDeregistration: removed fedora-thinkpad.
[15:43:57 +02:00] [logger main_logger] [trace] [thread 5811140] BVTCPConnectionManager: Removed session 1 for fedora-thinkpad
[15:43:59 +02:00] [logger main_logger] [trace] [thread 5811108] App: quitting. Sent TERMINATE_ALL message and BVEVENTTYPE_APP_SERVICE_DEREGISTERED to everyone
[SOLVED]
4peers.staplelog shows correct log after fixing removing services from serviceV logic and incrementing sessionID after creating the sessionData object.

[BUG1]
It seems that linux discovery and tcp have some issues between themselves.
[LOGS]
[23:34:47 +02:00] [logger main_logger] [trace] [thread 8418] Logger set up.
[23:34:47 +02:00] [logger main_logger] [trace] [thread 8418] BVTCPConnectionManager: Accepting connections on :::50001... for service: fedora
[23:34:47 +02:00] [logger main_logger] [trace] [thread 8422] App: HandlePublishedServices is called.
[23:34:47 +02:00] [logger main_logger] [trace] [thread 8422] App: Sending request to Discovery to resolve BupsioBup.local
[23:34:47 +02:00] [logger main_logger] [trace] [thread 8422] App: HandlePublishedServices is called.
[23:34:47 +02:00] [logger main_logger] [trace] [thread 8422] App: Sending request to Discovery to resolve ProBoopens.local
[23:34:47 +02:00] [logger main_logger] [trace] [thread 8422] App: HandlePublishedServices is called.
[23:34:47 +02:00] [logger main_logger] [trace] [thread 8422] App: HandlePublishedServices is called.
[23:34:47 +02:00] [logger main_logger] [trace] [thread 8422] App: HandleResolvedServices ENTER
[23:34:47 +02:00] [logger main_logger] [trace] [thread 8422] App: Resolved BupsioBup.local to hosttarget: BupsioBup.local
[23:34:47 +02:00] [logger main_logger] [trace] [thread 8422] App: on port 50001
[23:34:47 +02:00] [logger main_logger] [trace] [thread 8422] BVApp_ConsoleClient::ResolveServiceToEndpoint: Resolving host BupsioBup.local on port: 50001
[23:34:47 +02:00] [logger main_logger] [trace] [thread 8422] Successfuly resolved BupsioBup.local to 192.168.0.14
[23:34:47 +02:00] [logger main_logger] [trace] [thread 8422] App: Trying to connect asynchronously with service: BupsioBup.local. Trying to create Session ID: 1
[23:34:47 +02:00] [logger main_logger] [trace] [thread 8422] App: HandleResolvedServices ENTER
[23:34:47 +02:00] [logger main_logger] [trace] [thread 8422] App: Resolved ProBoopens.local to hosttarget: ProBoopens.local
[23:34:47 +02:00] [logger main_logger] [trace] [thread 8422] App: on port 50001
[23:34:47 +02:00] [logger main_logger] [trace] [thread 8422] BVApp_ConsoleClient::ResolveServiceToEndpoint: Resolving host ProBoopens.local on port: 50001
[23:34:47 +02:00] [logger main_logger] [trace] [thread 8422] Successfuly resolved ProBoopens.local to 192.168.0.16
[23:34:47 +02:00] [logger main_logger] [trace] [thread 8422] App: Trying to connect asynchronously with service: ProBoopens.local. Trying to create Session ID: 2
[23:34:47 +02:00] [logger main_logger] [trace] [thread 8422] App: HandleResolvedServices ENTER
[23:34:47 +02:00] [logger main_logger] [trace] [thread 8422] App: Resolved BupsioBup.local to hosttarget: BupsioBup.local
[23:34:47 +02:00] [logger main_logger] [trace] [thread 8422] App: on port 50001
[23:34:47 +02:00] [logger main_logger] [trace] [thread 8422] BVApp_ConsoleClient::ResolveServiceToEndpoint: Resolving host BupsioBup.local on port: 50001
[23:34:47 +02:00] [logger main_logger] [trace] [thread 8423] ConnectHandler: Successfuly connected to ProBoopens.local: 192.168.0.16:50001 SessionID: 3
[23:34:47 +02:00] [logger main_logger] [trace] [thread 8423] ConnectHandler: Current Sessions:
[23:34:47 +02:00] [logger main_logger] [trace] [thread 8423] ConnectHandler: ServiceName: ProBoopens.local Session ID: 3
[23:34:47 +02:00] [logger main_logger] [trace] [thread 8422] Successfuly resolved BupsioBup.local to 192.168.0.14
[23:34:47 +02:00] [logger main_logger] [trace] [thread 8422] App: Trying to connect asynchronously with service: BupsioBup.local. Trying to create Session ID: 3
[23:34:47 +02:00] [logger main_logger] [trace] [thread 8422] App: HandleResolvedServices ENTER
[23:34:47 +02:00] [logger main_logger] [trace] [thread 8422] App: Resolved ProBoopens.local to hosttarget: ProBoopens.local
[23:34:47 +02:00] [logger main_logger] [trace] [thread 8422] App: on port 50001
[23:34:47 +02:00] [logger main_logger] [trace] [thread 8422] BVApp_ConsoleClient::ResolveServiceToEndpoint: Resolving host ProBoopens.local on port: 50001
[23:34:47 +02:00] [logger main_logger] [trace] [thread 8423] Session [3]: Read 138 bytes
[23:34:47 +02:00] [logger main_logger] [trace] [thread 8423] Session [3]: Received BVSESSIONCONTROLMESSAGETYPE_HELLO. Sending _HELLOBACK
[23:34:47 +02:00] [logger main_logger] [debug] [thread 8423] WriteMessageFrame: data size: 128
[23:34:47 +02:00] [logger main_logger] [debug] [thread 8423] WriteMessageFrame: dataLen: 6
[23:34:47 +02:00] [logger main_logger] [trace] [thread 8422] Successfuly resolved ProBoopens.local to 192.168.0.16
[23:34:47 +02:00] [logger main_logger] [trace] [thread 8422] Session for ProBoopens.local already present (probably we accepted it).
[23:34:47 +02:00] [logger main_logger] [trace] [thread 8423] Session [3]: Read all bytes 138
[23:34:47 +02:00] [logger main_logger] [debug] [thread 8423] Session [3]: Writebuffer: 
[23:34:47 +02:00] [logger main_logger] [trace] [thread 8423] Session [3]: Written 138 bytes
[23:34:47 +02:00] [logger main_logger] [trace] [thread 8423] Session [3]: Written all bytes
[23:34:47 +02:00] [logger main_logger] [trace] [thread 8423] ConnectHandler: Successfuly connected to BupsioBup.local: 192.168.0.14:50001 SessionID: 4
[23:34:47 +02:00] [logger main_logger] [trace] [thread 8423] ConnectHandler: Current Sessions:
[23:34:47 +02:00] [logger main_logger] [trace] [thread 8423] ConnectHandler: ServiceName: ProBoopens.local Session ID: 3
[23:34:47 +02:00] [logger main_logger] [trace] [thread 8423] ConnectHandler: ServiceName: BupsioBup.local Session ID: 4
[23:34:47 +02:00] [logger main_logger] [info] [thread 8423] ConnectHandler: Session associated with service BupsioBup.local already present.
[23:34:47 +02:00] [logger main_logger] [trace] [thread 8423] Session [4]: Read 138 bytes
[23:34:47 +02:00] [logger main_logger] [trace] [thread 8423] Session [4]: Received BVSESSIONCONTROLMESSAGETYPE_HELLO. Sending _HELLOBACK
[23:34:47 +02:00] [logger main_logger] [debug] [thread 8423] WriteMessageFrame: data size: 128
[23:34:47 +02:00] [logger main_logger] [debug] [thread 8423] WriteMessageFrame: dataLen: 6
[23:34:47 +02:00] [logger main_logger] [trace] [thread 8423] Session [4]: Read all bytes 138
[23:34:47 +02:00] [logger main_logger] [debug] [thread 8423] Session [4]: Writebuffer: 
[23:34:47 +02:00] [logger main_logger] [trace] [thread 8423] Session [4]: Written 138 bytes
[23:34:47 +02:00] [logger main_logger] [trace] [thread 8423] Session [4]: Written all bytes
[23:34:47 +02:00] [logger main_logger] [trace] [thread 8422] App: HandlePublishedServices is called.
[23:34:47 +02:00] [logger main_logger] [trace] [thread 8422] App: HandlePublishedServices is called.
[23:34:47 +02:00] [logger main_logger] [trace] [thread 8422] App: HandlePublishedServices is called.
[23:34:53 +02:00] [logger main_logger] [trace] [thread 8422] App: HandlePublishedServices is called.
[23:34:53 +02:00] [logger main_logger] [trace] [thread 8422] App: Sending request to Discovery to resolve fedora-thinkpad
[23:34:53 +02:00] [logger main_logger] [trace] [thread 8422] App: HandlePublishedServices is called.
[23:34:54 +02:00] [logger main_logger] [trace] [thread 8422] App: HandleResolvedServices ENTER
[23:34:54 +02:00] [logger main_logger] [trace] [thread 8422] App: Resolved fedora-thinkpad to hosttarget: fedora-thinkpad.local
[23:34:54 +02:00] [logger main_logger] [trace] [thread 8422] App: on port 50001
[23:34:54 +02:00] [logger main_logger] [trace] [thread 8422] BVApp_ConsoleClient::ResolveServiceToEndpoint: Resolving host fedora-thinkpad.local on port: 50001
[23:34:54 +02:00] [logger main_logger] [trace] [thread 8422] Successfuly resolved fedora-thinkpad to 192.168.0.74
[23:34:54 +02:00] [logger main_logger] [trace] [thread 8422] App: Trying to connect asynchronously with service: fedora-thinkpad. Trying to create Session ID: 4
[23:34:54 +02:00] [logger main_logger] [trace] [thread 8422] App: HandleResolvedServices ENTER
[23:34:54 +02:00] [logger main_logger] [trace] [thread 8422] App: Resolved fedora-thinkpad to hosttarget: fedora-thinkpad.local
[23:34:54 +02:00] [logger main_logger] [trace] [thread 8422] App: on port 50001
[23:34:54 +02:00] [logger main_logger] [trace] [thread 8422] BVApp_ConsoleClient::ResolveServiceToEndpoint: Resolving host fedora-thinkpad.local on port: 50001
[23:34:54 +02:00] [logger main_logger] [trace] [thread 8422] Successfuly resolved fedora-thinkpad to 192.168.0.74
[23:34:54 +02:00] [logger main_logger] [trace] [thread 8422] App: Trying to connect asynchronously with service: fedora-thinkpad. Trying to create Session ID: 5
[23:34:54 +02:00] [logger main_logger] [error] [thread 8423] ConnectHandler Error: 113 No route to host system
[23:34:54 +02:00] [logger main_logger] [error] [thread 8423] ConnectHandler Error: 113 No route to host system
[23:35:01 +02:00] [logger main_logger] [trace] [thread 8422] BVApp_ConsoleClient: HandleServiceDeregistration called
[23:35:01 +02:00] [logger main_logger] [trace] [thread 8422] App, HandleServiceDeregistration: removed fedora-thinkpad.
[23:35:01 +02:00] [logger main_logger] [error] [thread 8422] BVTCPConnectionManager: No session mapping for fedora-thinkpad
[23:35:01 +02:00] [logger main_logger] [trace] [thread 8422] BVApp_ConsoleClient: HandleServiceDeregistration called
[23:35:01 +02:00] [logger main_logger] [warning] [thread 8422] App, HandleServiceDeregistration: fedora-thinkpad not found in serviceV!
[23:35:01 +02:00] [logger main_logger] [info] [thread 8422] App, HandleServiceDeregistration: Currently: 2 Services in serviceV:
[23:35:01 +02:00] [logger main_logger] [info] [thread 8422] 1: BupsioBup.local
[23:35:01 +02:00] [logger main_logger] [info] [thread 8422] 2: ProBoopens.local
[23:35:02 +02:00] [logger main_logger] [error] [thread 8423] Session [4]: Error in ReadMessageFrame callback: 2, End of file
[23:35:02 +02:00] [logger main_logger] [debug] [thread 8423] Read buffer:  Read buffer is a nullpointer: false Bytes read: 0 Bytes transferred: 0 Address in nodeData: 192.168.0.14 Endpoint address: 192.168.0.14 State: 0
[23:35:04 +02:00] [logger main_logger] [trace] [thread 8422] BVApp_ConsoleClient: HandleServiceDeregistration called
[23:35:04 +02:00] [logger main_logger] [trace] [thread 8422] App, HandleServiceDeregistration: removed BupsioBup.local.
[23:35:04 +02:00] [logger main_logger] [trace] [thread 8422] BVTCPConnectionManager: Removed session 4 for BupsioBup.local
[23:35:04 +02:00] [logger main_logger] [trace] [thread 8422] BVApp_ConsoleClient: HandleServiceDeregistration called
[23:35:04 +02:00] [logger main_logger] [warning] [thread 8422] App, HandleServiceDeregistration: BupsioBup.local not found in serviceV!
[23:35:04 +02:00] [logger main_logger] [info] [thread 8422] App, HandleServiceDeregistration: Currently: 1 Services in serviceV:
[23:35:04 +02:00] [logger main_logger] [info] [thread 8422] 1: ProBoopens.local
[23:35:06 +02:00] [logger main_logger] [trace] [thread 8418] App: quitting. Sent TERMINATE_ALL message and BVEVENTTYPE_APP_SERVICE_DEREGISTERED to everyone
[23:35:06 +02:00] [logger main_logger] [trace] [thread 8422] App: Shutting down...
[UNSOLVED]

[BUG2]
VM + one mac + second mac = sessions are not established correctly.
E.g.
First: VM + MacBook Pro. They show sessions established between them.
Next MacBook Air connects. on Pro: two sessions. On VM: two sessions. On Air - only between fedora.
Okay: on Air faulty behavior noticed - connected to ProBoopens.local as session ID: 3 and then connected to fedora for session ID: 3 - the same.
[LOGS]
Okay, I've found one bug:
When connecting, sometimes the same ID is assigned to a session:
[01:49:56 +02:00] [logger main_logger] [trace] [thread 2785705] Logger set up.
[01:49:56 +02:00] [logger main_logger] [trace] [thread 2785705] BVTCPConnectionManager: Accepting connections on :::50001... for service: BupsioBup.local
[01:49:56 +02:00] [logger main_logger] [trace] [thread 2785729] Discovery: Awaiting read readiness on the socket...
[01:49:56 +02:00] [logger main_logger] [trace] [thread 2785729] Discovery: Browsing active...
[01:49:56 +02:00] [logger main_logger] [trace] [thread 2785731] Discovery: DNSServiceProcessResult returned. Sending BVEVENTTYPE_APP_PUBLISHED_SERVICE to App...
[01:49:56 +02:00] [logger main_logger] [trace] [thread 2785731] Discovery: Awaiting read readiness on the socket...
[01:49:56 +02:00] [logger main_logger] [trace] [thread 2785730] App: HandlePublishedServices is called.
[01:49:56 +02:00] [logger main_logger] [trace] [thread 2785730] App, HandlePublishedServices: Added fedora to serviceV
[01:49:56 +02:00] [logger main_logger] [trace] [thread 2785730] App, HandlePublishedServices: Added ProBoopens.local to serviceV
[01:49:56 +02:00] [logger main_logger] [trace] [thread 2785730] App: Sending request to Discovery to resolve fedora
[01:49:56 +02:00] [logger main_logger] [trace] [thread 2785730] App: Sending request to Discovery to resolve ProBoopens.local
[01:49:56 +02:00] [logger main_logger] [trace] [thread 2785728] Discovery: Received request to resolve a service...
[01:49:56 +02:00] [logger main_logger] [trace] [thread 2785728] Discovery: Resolve context created.
[01:49:56 +02:00] [logger main_logger] [trace] [thread 2785728] Discovery: Resolve job scheduled...
[01:49:56 +02:00] [logger main_logger] [trace] [thread 2785728] Discovery: Received request to resolve a service...
[01:49:56 +02:00] [logger main_logger] [trace] [thread 2785731] Discovery: Resolve result has been processed.
[01:49:56 +02:00] [logger main_logger] [trace] [thread 2785731] Discovery: Resolve ref has been deallocated.
[01:49:56 +02:00] [logger main_logger] [trace] [thread 2785728] Discovery: Resolve context created.
[01:49:56 +02:00] [logger main_logger] [trace] [thread 2785728] Discovery: Resolve job scheduled...
[01:49:56 +02:00] [logger main_logger] [trace] [thread 2785730] App: HandleResolvedServices ENTER
[01:49:56 +02:00] [logger main_logger] [trace] [thread 2785730] App: Resolved fedora to hosttarget: fedora.local.
[01:49:56 +02:00] [logger main_logger] [trace] [thread 2785730] App: on port 50001
[01:49:56 +02:00] [logger main_logger] [trace] [thread 2785730] BVApp_ConsoleClient::ResolveServiceToEndpoint: Resolving host fedora.local. on port: 50001
[01:49:56 +02:00] [logger main_logger] [trace] [thread 2785731] Discovery: Resolve result has been processed.
[01:49:56 +02:00] [logger main_logger] [trace] [thread 2785731] Discovery: Resolve ref has been deallocated.
[01:49:56 +02:00] [logger main_logger] [trace] [thread 2785730] Successfuly resolved fedora to fe80::abad:9f3e:6666:ae1c%en0
[01:49:56 +02:00] [logger main_logger] [trace] [thread 2785730] App: Trying to connect asynchronously with service: fedora. Trying to create Session ID: 1
[01:49:56 +02:00] [logger main_logger] [trace] [thread 2785730] App: HandleResolvedServices ENTER
[01:49:56 +02:00] [logger main_logger] [trace] [thread 2785730] App: Resolved ProBoopens.local to hosttarget: ProBoopens.local.
[01:49:56 +02:00] [logger main_logger] [trace] [thread 2785730] App: on port 50001
[01:49:56 +02:00] [logger main_logger] [trace] [thread 2785730] BVApp_ConsoleClient::ResolveServiceToEndpoint: Resolving host ProBoopens.local. on port: 50001
[01:49:56 +02:00] [logger main_logger] [trace] [thread 2785730] Successfuly resolved ProBoopens.local to fe80::8ca:3732:f751:18e8%en0
[01:49:56 +02:00] [logger main_logger] [trace] [thread 2785730] App: Trying to connect asynchronously with service: ProBoopens.local. Trying to create Session ID: 2
[01:49:56 +02:00] [logger main_logger] [trace] [thread 2785731] ConnectHandler: Successfuly connected to ProBoopens.local: fe80::8ca:3732:f751:18e8%en0:50001 SessionID: 3
[01:49:56 +02:00] [logger main_logger] [trace] [thread 2785731] ConnectHandler: Current Sessions:
[01:49:56 +02:00] [logger main_logger] [trace] [thread 2785731] ConnectHandler: ServiceName: ProBoopens.local Session ID: 3
[01:49:56 +02:00] [logger main_logger] [trace] [thread 2785731] ConnectHandler: Successfuly connected to fedora: fe80::abad:9f3e:6666:ae1c%en0:50001 SessionID: 3
[01:49:56 +02:00] [logger main_logger] [trace] [thread 2785731] ConnectHandler: Current Sessions:
[01:49:56 +02:00] [logger main_logger] [trace] [thread 2785731] ConnectHandler: ServiceName: fedora Session ID: 3
[01:49:56 +02:00] [logger main_logger] [trace] [thread 2785731] Session [3]: Read 138 bytes
[01:49:56 +02:00] [logger main_logger] [trace] [thread 2785731] Session [3]: Received BVSESSIONCONTROLMESSAGETYPE_HELLO. Sending _HELLOBACK
[01:49:56 +02:00] [logger main_logger] [debug] [thread 2785731] WriteMessageFrame: data size: 128
[01:49:56 +02:00] [logger main_logger] [debug] [thread 2785731] WriteMessageFrame: dataLen: 15
[01:49:56 +02:00] [logger main_logger] [trace] [thread 2785731] Session [3]: Read all bytes 138
[01:49:56 +02:00] [logger main_logger] [debug] [thread 2785731] Session [3]: Writebuffer: 
[01:49:56 +02:00] [logger main_logger] [trace] [thread 2785731] Session [3]: Written 138 bytes
[01:49:56 +02:00] [logger main_logger] [trace] [thread 2785731] Session [3]: Written all bytes
[01:49:56 +02:00] [logger main_logger] [trace] [thread 2785731] Session [3]: Read 138 bytes
[01:49:56 +02:00] [logger main_logger] [trace] [thread 2785731] Session [3]: Received BVSESSIONCONTROLMESSAGETYPE_HELLO. Sending _HELLOBACK
[01:49:56 +02:00] [logger main_logger] [debug] [thread 2785731] WriteMessageFrame: data size: 128
[01:49:56 +02:00] [logger main_logger] [debug] [thread 2785731] WriteMessageFrame: dataLen: 15
[01:49:56 +02:00] [logger main_logger] [trace] [thread 2785731] Session [3]: Read all bytes 138
[01:49:56 +02:00] [logger main_logger] [debug] [thread 2785731] Session [3]: Writebuffer: 
[01:49:56 +02:00] [logger main_logger] [trace] [thread 2785731] Session [3]: Written 138 bytes
[01:49:56 +02:00] [logger main_logger] [trace] [thread 2785731] Session [3]: Written all bytes
[01:49:56 +02:00] [logger main_logger] [trace] [thread 2785731] Discovery: DNSServiceProcessResult returned. Sending BVEVENTTYPE_APP_PUBLISHED_SERVICE to App...
[01:49:56 +02:00] [logger main_logger] [trace] [thread 2785731] Discovery: Awaiting read readiness on the socket...
[01:49:56 +02:00] [logger main_logger] [trace] [thread 2785730] App: HandlePublishedServices is called.
[01:50:07 +02:00] [logger main_logger] [trace] [thread 2785705] App: quitting. Sent TERMINATE_ALL message and BVEVENTTYPE_APP_SERVICE_DEREGISTERED to everyone
[01:50:07 +02:00] [logger main_logger] [trace] [thread 2785730] App: Shutting down...
[01:50:07 +02:00] [logger main_logger] [trace] [thread 2785728] Discovery: Shutting down...
[01:50:07 +02:00] [logger main_logger] [trace] [thread 2785705] Discovery dies.
Here fedora and proboopens have SessionID: 3

I'm reassigning a value, which could be incremented in the next connect/accept attempt..
Not assigning the value again in ConnectHandler seemed to fixed the issue.
[SOLVED]

[BUG3]
Couldn't connect to peer after 5 retrials.
[LOGS]
[02:03:57 +02:00] [logger main_logger] [trace] [thread 5191] Logger set up.
[02:03:57 +02:00] [logger main_logger] [trace] [thread 5191] BVTCPConnectionManager: Accepting connections on :::50001... for service: fedora
[02:03:58 +02:00] [logger main_logger] [trace] [thread 5195] App: HandlePublishedServices is called.
[02:03:58 +02:00] [logger main_logger] [trace] [thread 5195] App: HandlePublishedServices is called.
[02:03:58 +02:00] [logger main_logger] [trace] [thread 5195] App: HandlePublishedServices is called.
[02:04:01 +02:00] [logger main_logger] [trace] [thread 5196] Accept successful. Requesting identification from the peer.
[02:04:01 +02:00] [logger main_logger] [debug] [thread 5196] WriteMessageFrame: data size: 128
[02:04:01 +02:00] [logger main_logger] [debug] [thread 5196] WriteMessageFrame: dataLen: 128
[02:04:01 +02:00] [logger main_logger] [debug] [thread 5196] Session [0]: Writebuffer: Lð\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00
[02:04:01 +02:00] [logger main_logger] [trace] [thread 5196] Session [0]: Written 138 bytes
[02:04:01 +02:00] [logger main_logger] [trace] [thread 5196] Session [0]: Written all bytes
[02:04:01 +02:00] [logger main_logger] [trace] [thread 5196] Session [0]: Read 138 bytes
[02:04:01 +02:00] [logger main_logger] [trace] [thread 5196] Session [0]: Received _HELLOBACK with payload='ProBoopens.local'. Calling Manager handler.
[02:04:01 +02:00] [logger main_logger] [trace] [thread 5196] BVTCPConnectionManager: Established connection with node: ProBoopens.local Address: fe80::8ca:3732:f751:18e8%enp0s1
[02:04:01 +02:00] [logger main_logger] [trace] [thread 5196] BVTCPConnectionManager: Current sessions:
[02:04:01 +02:00] [logger main_logger] [trace] [thread 5196] Session 0 : ID: 0, service: ProBoopens.local
[02:04:01 +02:00] [logger main_logger] [trace] [thread 5196] Session [0]: Read all bytes 138
[02:04:01 +02:00] [logger main_logger] [trace] [thread 5195] App: HandlePublishedServices is called.
[02:04:01 +02:00] [logger main_logger] [trace] [thread 5195] App, HandlePublishedServices: Added ProBoopens.local to serviceV
[02:04:01 +02:00] [logger main_logger] [trace] [thread 5195] App: Sending request to Discovery to resolve ProBoopens.local
[02:04:01 +02:00] [logger main_logger] [trace] [thread 5195] App: HandlePublishedServices is called.
[02:04:01 +02:00] [logger main_logger] [trace] [thread 5195] App: HandleResolvedServices ENTER
[02:04:01 +02:00] [logger main_logger] [trace] [thread 5195] App: Resolved ProBoopens.local to hosttarget: ProBoopens.local
[02:04:01 +02:00] [logger main_logger] [trace] [thread 5195] App: on port 50001
[02:04:01 +02:00] [logger main_logger] [trace] [thread 5195] BVApp_ConsoleClient::ResolveServiceToEndpoint: Resolving host ProBoopens.local on port: 50001
[02:04:01 +02:00] [logger main_logger] [trace] [thread 5195] Successfuly resolved ProBoopens.local to 192.168.0.16
[02:04:01 +02:00] [logger main_logger] [trace] [thread 5195] Session for ProBoopens.local already present (probably we accepted it).
[02:04:01 +02:00] [logger main_logger] [trace] [thread 5195] App: HandleResolvedServices ENTER
[02:04:01 +02:00] [logger main_logger] [trace] [thread 5195] App: Resolved ProBoopens.local to hosttarget: ProBoopens.local
[02:04:01 +02:00] [logger main_logger] [trace] [thread 5195] App: on port 50001
[02:04:01 +02:00] [logger main_logger] [trace] [thread 5195] BVApp_ConsoleClient::ResolveServiceToEndpoint: Resolving host ProBoopens.local on port: 50001
[02:04:01 +02:00] [logger main_logger] [trace] [thread 5195] Successfuly resolved ProBoopens.local to 192.168.0.16
[02:04:01 +02:00] [logger main_logger] [trace] [thread 5195] Session for ProBoopens.local already present (probably we accepted it).
[02:04:05 +02:00] [logger main_logger] [trace] [thread 5196] Accept successful. Requesting identification from the peer.
[02:04:05 +02:00] [logger main_logger] [debug] [thread 5196] WriteMessageFrame: data size: 128
[02:04:05 +02:00] [logger main_logger] [debug] [thread 5196] WriteMessageFrame: dataLen: 128
[02:04:05 +02:00] [logger main_logger] [debug] [thread 5196] Session [1]: Writebuffer: Lð\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00
[02:04:05 +02:00] [logger main_logger] [trace] [thread 5196] Session [1]: Written 138 bytes
[02:04:05 +02:00] [logger main_logger] [trace] [thread 5196] Session [1]: Written all bytes
[02:04:05 +02:00] [logger main_logger] [trace] [thread 5196] Session [1]: Read 138 bytes
[02:04:05 +02:00] [logger main_logger] [trace] [thread 5196] Session [1]: Received _HELLOBACK with payload='BupsioBup.local'. Calling Manager handler.
[02:04:05 +02:00] [logger main_logger] [trace] [thread 5196] BVTCPConnectionManager: Established connection with node: BupsioBup.local Address: fe80::1425:6133:ce74:1ea7%enp0s1
[02:04:05 +02:00] [logger main_logger] [trace] [thread 5196] BVTCPConnectionManager: Current sessions:
[02:04:05 +02:00] [logger main_logger] [trace] [thread 5196] Session 0 : ID: 0, service: ProBoopens.local
[02:04:05 +02:00] [logger main_logger] [trace] [thread 5196] Session 1 : ID: 1, service: BupsioBup.local
[02:04:05 +02:00] [logger main_logger] [trace] [thread 5196] Session [1]: Read all bytes 138
[02:04:05 +02:00] [logger main_logger] [trace] [thread 5195] App: HandlePublishedServices is called.
[02:04:05 +02:00] [logger main_logger] [trace] [thread 5195] App, HandlePublishedServices: Added BupsioBup.local to serviceV
[02:04:05 +02:00] [logger main_logger] [trace] [thread 5195] App: Sending request to Discovery to resolve BupsioBup.local
[02:04:05 +02:00] [logger main_logger] [trace] [thread 5195] App: HandlePublishedServices is called.
[02:04:05 +02:00] [logger main_logger] [trace] [thread 5195] App: HandleResolvedServices ENTER
[02:04:05 +02:00] [logger main_logger] [trace] [thread 5195] App: Resolved BupsioBup.local to hosttarget: BupsioBup.local
[02:04:05 +02:00] [logger main_logger] [trace] [thread 5195] App: on port 50001
[02:04:05 +02:00] [logger main_logger] [trace] [thread 5195] BVApp_ConsoleClient::ResolveServiceToEndpoint: Resolving host BupsioBup.local on port: 50001
[02:04:05 +02:00] [logger main_logger] [trace] [thread 5195] Successfuly resolved BupsioBup.local to 192.168.0.14
[02:04:05 +02:00] [logger main_logger] [trace] [thread 5195] Session for BupsioBup.local already present (probably we accepted it).
[02:04:05 +02:00] [logger main_logger] [trace] [thread 5195] App: HandleResolvedServices ENTER
[02:04:05 +02:00] [logger main_logger] [trace] [thread 5195] App: Resolved BupsioBup.local to hosttarget: BupsioBup.local
[02:04:05 +02:00] [logger main_logger] [trace] [thread 5195] App: on port 50001
[02:04:05 +02:00] [logger main_logger] [trace] [thread 5195] BVApp_ConsoleClient::ResolveServiceToEndpoint: Resolving host BupsioBup.local on port: 50001
[02:04:05 +02:00] [logger main_logger] [trace] [thread 5195] Successfuly resolved BupsioBup.local to 192.168.0.14
[02:04:05 +02:00] [logger main_logger] [trace] [thread 5195] Session for BupsioBup.local already present (probably we accepted it).
[02:04:25 +02:00] [logger main_logger] [error] [thread 5196] Session [1]: Error in ReadMessageFrame callback: 2, End of file
[02:04:25 +02:00] [logger main_logger] [debug] [thread 5196] Read buffer:  Read buffer is a nullpointer: false Bytes read: 0 Bytes transferred: 0 Address in nodeData: fe80::1425:6133:ce74:1ea7%enp0s1 Endpoint address: fe80::1425:6133:ce74:1ea7%enp0s1 State: 1
[02:04:26 +02:00] [logger main_logger] [trace] [thread 5195] BVApp_ConsoleClient: HandleServiceDeregistration called
[02:04:26 +02:00] [logger main_logger] [trace] [thread 5195] App, HandleServiceDeregistration: removed BupsioBup.local.
[02:04:26 +02:00] [logger main_logger] [trace] [thread 5195] BVTCPConnectionManager: Removed session 1 for BupsioBup.local
[02:04:26 +02:00] [logger main_logger] [info] [thread 5195] App, HandleServiceDeregistration: Currently: 1 Services in serviceV:
[02:04:26 +02:00] [logger main_logger] [info] [thread 5195] 1: ProBoopens.local
[02:04:28 +02:00] [logger main_logger] [trace] [thread 5195] BVApp_ConsoleClient: HandleServiceDeregistration called
[02:04:28 +02:00] [logger main_logger] [warning] [thread 5195] App, HandleServiceDeregistration: BupsioBup.local not found in serviceV!
[02:04:28 +02:00] [logger main_logger] [info] [thread 5195] App, HandleServiceDeregistration: Currently: 1 Services in serviceV:
[02:04:28 +02:00] [logger main_logger] [info] [thread 5195] 1: ProBoopens.local
[02:04:33 +02:00] [logger main_logger] [trace] [thread 5196] Accept successful. Requesting identification from the peer.
[02:04:33 +02:00] [logger main_logger] [debug] [thread 5196] WriteMessageFrame: data size: 128
[02:04:33 +02:00] [logger main_logger] [debug] [thread 5196] WriteMessageFrame: dataLen: 128
[02:04:33 +02:00] [logger main_logger] [debug] [thread 5196] Session [2]: Writebuffer: Mð\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00
[02:04:33 +02:00] [logger main_logger] [trace] [thread 5196] Session [2]: Written 138 bytes
[02:04:33 +02:00] [logger main_logger] [trace] [thread 5196] Session [2]: Written all bytes
[02:04:33 +02:00] [logger main_logger] [trace] [thread 5196] Session [2]: Read 138 bytes
[02:04:33 +02:00] [logger main_logger] [trace] [thread 5196] Session [2]: Received _HELLOBACK with payload='BupsioBup.local'. Calling Manager handler.
[02:04:33 +02:00] [logger main_logger] [trace] [thread 5196] BVTCPConnectionManager: Established connection with node: BupsioBup.local Address: fe80::1425:6133:ce74:1ea7%enp0s1
[02:04:33 +02:00] [logger main_logger] [trace] [thread 5196] BVTCPConnectionManager: Current sessions:
[02:04:33 +02:00] [logger main_logger] [trace] [thread 5196] Session 0 : ID: 0, service: ProBoopens.local
[02:04:33 +02:00] [logger main_logger] [trace] [thread 5196] Session 1 : ID: 2, service: BupsioBup.local
[02:04:33 +02:00] [logger main_logger] [trace] [thread 5196] Session [2]: Read all bytes 138
[02:04:33 +02:00] [logger main_logger] [trace] [thread 5195] App: HandlePublishedServices is called.
[02:04:33 +02:00] [logger main_logger] [trace] [thread 5195] App, HandlePublishedServices: Added BupsioBup.local to serviceV
[02:04:33 +02:00] [logger main_logger] [trace] [thread 5195] App: Sending request to Discovery to resolve BupsioBup.local
[02:04:33 +02:00] [logger main_logger] [trace] [thread 5195] App: HandlePublishedServices is called.
[02:04:33 +02:00] [logger main_logger] [trace] [thread 5195] App: HandleResolvedServices ENTER
[02:04:33 +02:00] [logger main_logger] [trace] [thread 5195] App: Resolved BupsioBup.local to hosttarget: BupsioBup.local
[02:04:33 +02:00] [logger main_logger] [trace] [thread 5195] App: on port 50001
[02:04:33 +02:00] [logger main_logger] [trace] [thread 5195] BVApp_ConsoleClient::ResolveServiceToEndpoint: Resolving host BupsioBup.local on port: 50001
[02:04:33 +02:00] [logger main_logger] [trace] [thread 5195] Successfuly resolved BupsioBup.local to 192.168.0.14
[02:04:33 +02:00] [logger main_logger] [trace] [thread 5195] Session for BupsioBup.local already present (probably we accepted it).
[02:04:33 +02:00] [logger main_logger] [trace] [thread 5195] App: HandleResolvedServices ENTER
[02:04:33 +02:00] [logger main_logger] [trace] [thread 5195] App: Resolved BupsioBup.local to hosttarget: BupsioBup.local
[02:04:33 +02:00] [logger main_logger] [trace] [thread 5195] App: on port 50001
[02:04:33 +02:00] [logger main_logger] [trace] [thread 5195] BVApp_ConsoleClient::ResolveServiceToEndpoint: Resolving host BupsioBup.local on port: 50001
[02:04:33 +02:00] [logger main_logger] [trace] [thread 5195] Successfuly resolved BupsioBup.local to 192.168.0.14
[02:04:33 +02:00] [logger main_logger] [trace] [thread 5195] Session for BupsioBup.local already present (probably we accepted it).
[02:04:35 +02:00] [logger main_logger] [error] [thread 5196] Session [2]: Error in ReadMessageFrame callback: 2, End of file
[02:04:35 +02:00] [logger main_logger] [debug] [thread 5196] Read buffer:  Read buffer is a nullpointer: false Bytes read: 0 Bytes transferred: 0 Address in nodeData: fe80::1425:6133:ce74:1ea7%enp0s1 Endpoint address: fe80::1425:6133:ce74:1ea7%enp0s1 State: 1
[02:04:36 +02:00] [logger main_logger] [trace] [thread 5195] BVApp_ConsoleClient: HandleServiceDeregistration called
[02:04:36 +02:00] [logger main_logger] [trace] [thread 5195] App, HandleServiceDeregistration: removed BupsioBup.local.
[02:04:36 +02:00] [logger main_logger] [trace] [thread 5195] BVTCPConnectionManager: Removed session 2 for BupsioBup.local
[02:04:36 +02:00] [logger main_logger] [info] [thread 5195] App, HandleServiceDeregistration: Currently: 1 Services in serviceV:
[02:04:36 +02:00] [logger main_logger] [info] [thread 5195] 1: ProBoopens.local
[02:04:36 +02:00] [logger main_logger] [trace] [thread 5195] BVApp_ConsoleClient: HandleServiceDeregistration called
[02:04:36 +02:00] [logger main_logger] [warning] [thread 5195] App, HandleServiceDeregistration: BupsioBup.local not found in serviceV!
[02:04:36 +02:00] [logger main_logger] [info] [thread 5195] App, HandleServiceDeregistration: Currently: 1 Services in serviceV:
[02:04:36 +02:00] [logger main_logger] [info] [thread 5195] 1: ProBoopens.local
[02:04:37 +02:00] [logger main_logger] [trace] [thread 5196] Accept successful. Requesting identification from the peer.
[02:04:37 +02:00] [logger main_logger] [debug] [thread 5196] WriteMessageFrame: data size: 128
[02:04:37 +02:00] [logger main_logger] [debug] [thread 5196] WriteMessageFrame: dataLen: 128
[02:04:37 +02:00] [logger main_logger] [debug] [thread 5196] Session [3]: Writebuffer: 4Mð\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00
[02:04:37 +02:00] [logger main_logger] [trace] [thread 5196] Session [3]: Written 138 bytes
[02:04:37 +02:00] [logger main_logger] [trace] [thread 5196] Session [3]: Written all bytes
[02:04:37 +02:00] [logger main_logger] [trace] [thread 5196] Session [3]: Read 138 bytes
[02:04:37 +02:00] [logger main_logger] [trace] [thread 5196] Session [3]: Received _HELLOBACK with payload='BupsioBup.local'. Calling Manager handler.
[02:04:37 +02:00] [logger main_logger] [trace] [thread 5196] BVTCPConnectionManager: Established connection with node: BupsioBup.local Address: fe80::1425:6133:ce74:1ea7%enp0s1
[02:04:37 +02:00] [logger main_logger] [trace] [thread 5196] BVTCPConnectionManager: Current sessions:
[02:04:37 +02:00] [logger main_logger] [trace] [thread 5196] Session 0 : ID: 0, service: ProBoopens.local
[02:04:37 +02:00] [logger main_logger] [trace] [thread 5196] Session 1 : ID: 3, service: BupsioBup.local
[02:04:37 +02:00] [logger main_logger] [trace] [thread 5196] Session [3]: Read all bytes 138
[02:04:38 +02:00] [logger main_logger] [trace] [thread 5195] App: HandlePublishedServices is called.
[02:04:38 +02:00] [logger main_logger] [trace] [thread 5195] App, HandlePublishedServices: Added BupsioBup.local to serviceV
[02:04:38 +02:00] [logger main_logger] [trace] [thread 5195] App: Sending request to Discovery to resolve BupsioBup.local
[02:04:38 +02:00] [logger main_logger] [trace] [thread 5195] App: HandlePublishedServices is called.
[02:04:38 +02:00] [logger main_logger] [trace] [thread 5195] App: HandleResolvedServices ENTER
[02:04:38 +02:00] [logger main_logger] [trace] [thread 5195] App: Resolved BupsioBup.local to hosttarget: BupsioBup.local
[02:04:38 +02:00] [logger main_logger] [trace] [thread 5195] App: on port 50001
[02:04:38 +02:00] [logger main_logger] [trace] [thread 5195] BVApp_ConsoleClient::ResolveServiceToEndpoint: Resolving host BupsioBup.local on port: 50001
[02:04:38 +02:00] [logger main_logger] [trace] [thread 5195] Successfuly resolved BupsioBup.local to 192.168.0.14
[02:04:38 +02:00] [logger main_logger] [trace] [thread 5195] Session for BupsioBup.local already present (probably we accepted it).
[02:04:38 +02:00] [logger main_logger] [trace] [thread 5195] App: HandleResolvedServices ENTER
[02:04:38 +02:00] [logger main_logger] [trace] [thread 5195] App: Resolved BupsioBup.local to hosttarget: BupsioBup.local
[02:04:38 +02:00] [logger main_logger] [trace] [thread 5195] App: on port 50001
[02:04:38 +02:00] [logger main_logger] [trace] [thread 5195] BVApp_ConsoleClient::ResolveServiceToEndpoint: Resolving host BupsioBup.local on port: 50001
[02:04:38 +02:00] [logger main_logger] [trace] [thread 5195] Successfuly resolved BupsioBup.local to 192.168.0.14
[02:04:38 +02:00] [logger main_logger] [trace] [thread 5195] Session for BupsioBup.local already present (probably we accepted it).
[02:04:41 +02:00] [logger main_logger] [error] [thread 5196] Session [3]: Error in ReadMessageFrame callback: 2, End of file
[02:04:41 +02:00] [logger main_logger] [debug] [thread 5196] Read buffer:  Read buffer is a nullpointer: false Bytes read: 0 Bytes transferred: 0 Address in nodeData: fe80::1425:6133:ce74:1ea7%enp0s1 Endpoint address: fe80::1425:6133:ce74:1ea7%enp0s1 State: 1
[02:04:42 +02:00] [logger main_logger] [trace] [thread 5195] BVApp_ConsoleClient: HandleServiceDeregistration called
[02:04:42 +02:00] [logger main_logger] [trace] [thread 5195] App, HandleServiceDeregistration: removed BupsioBup.local.
[02:04:42 +02:00] [logger main_logger] [trace] [thread 5195] BVTCPConnectionManager: Removed session 3 for BupsioBup.local
[02:04:42 +02:00] [logger main_logger] [info] [thread 5195] App, HandleServiceDeregistration: Currently: 1 Services in serviceV:
[02:04:42 +02:00] [logger main_logger] [info] [thread 5195] 1: ProBoopens.local
[02:04:42 +02:00] [logger main_logger] [trace] [thread 5195] BVApp_ConsoleClient: HandleServiceDeregistration called
[02:04:42 +02:00] [logger main_logger] [warning] [thread 5195] App, HandleServiceDeregistration: BupsioBup.local not found in serviceV!
[02:04:42 +02:00] [logger main_logger] [info] [thread 5195] App, HandleServiceDeregistration: Currently: 1 Services in serviceV:
[02:04:42 +02:00] [logger main_logger] [info] [thread 5195] 1: ProBoopens.local
[02:04:43 +02:00] [logger main_logger] [trace] [thread 5196] Accept successful. Requesting identification from the peer.
[02:04:43 +02:00] [logger main_logger] [debug] [thread 5196] WriteMessageFrame: data size: 128
[02:04:43 +02:00] [logger main_logger] [debug] [thread 5196] WriteMessageFrame: dataLen: 128
[02:04:43 +02:00] [logger main_logger] [debug] [thread 5196] Session [4]: Writebuffer: ÷0Mð\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00
[02:04:43 +02:00] [logger main_logger] [trace] [thread 5196] Session [4]: Written 138 bytes
[02:04:43 +02:00] [logger main_logger] [trace] [thread 5196] Session [4]: Written all bytes
[02:04:44 +02:00] [logger main_logger] [trace] [thread 5196] Session [4]: Read 138 bytes
[02:04:44 +02:00] [logger main_logger] [trace] [thread 5196] Session [4]: Received _HELLOBACK with payload='BupsioBup.local'. Calling Manager handler.
[02:04:44 +02:00] [logger main_logger] [trace] [thread 5196] BVTCPConnectionManager: Established connection with node: BupsioBup.local Address: fe80::1425:6133:ce74:1ea7%enp0s1
[02:04:44 +02:00] [logger main_logger] [trace] [thread 5196] BVTCPConnectionManager: Current sessions:
[02:04:44 +02:00] [logger main_logger] [trace] [thread 5196] Session 0 : ID: 0, service: ProBoopens.local
[02:04:44 +02:00] [logger main_logger] [trace] [thread 5196] Session 1 : ID: 4, service: BupsioBup.local
[02:04:44 +02:00] [logger main_logger] [trace] [thread 5196] Session [4]: Read all bytes 138
[02:04:44 +02:00] [logger main_logger] [trace] [thread 5195] App: HandlePublishedServices is called.
[02:04:44 +02:00] [logger main_logger] [trace] [thread 5195] App, HandlePublishedServices: Added BupsioBup.local to serviceV
[02:04:44 +02:00] [logger main_logger] [trace] [thread 5195] App: Sending request to Discovery to resolve BupsioBup.local
[02:04:44 +02:00] [logger main_logger] [trace] [thread 5195] App: HandlePublishedServices is called.
[02:04:44 +02:00] [logger main_logger] [trace] [thread 5195] App: HandleResolvedServices ENTER
[02:04:44 +02:00] [logger main_logger] [trace] [thread 5195] App: Resolved BupsioBup.local to hosttarget: BupsioBup.local
[02:04:44 +02:00] [logger main_logger] [trace] [thread 5195] App: on port 50001
[02:04:44 +02:00] [logger main_logger] [trace] [thread 5195] BVApp_ConsoleClient::ResolveServiceToEndpoint: Resolving host BupsioBup.local on port: 50001
[02:04:44 +02:00] [logger main_logger] [trace] [thread 5195] Successfuly resolved BupsioBup.local to 192.168.0.14
[02:04:44 +02:00] [logger main_logger] [trace] [thread 5195] Session for BupsioBup.local already present (probably we accepted it).
[02:04:44 +02:00] [logger main_logger] [trace] [thread 5195] App: HandleResolvedServices ENTER
[02:04:44 +02:00] [logger main_logger] [trace] [thread 5195] App: Resolved BupsioBup.local to hosttarget: BupsioBup.local
[02:04:44 +02:00] [logger main_logger] [trace] [thread 5195] App: on port 50001
[02:04:44 +02:00] [logger main_logger] [trace] [thread 5195] BVApp_ConsoleClient::ResolveServiceToEndpoint: Resolving host BupsioBup.local on port: 50001
[02:04:44 +02:00] [logger main_logger] [trace] [thread 5195] Successfuly resolved BupsioBup.local to 192.168.0.14
[02:04:44 +02:00] [logger main_logger] [trace] [thread 5195] Session for BupsioBup.local already present (probably we accepted it).
[02:04:45 +02:00] [logger main_logger] [error] [thread 5196] Session [0]: Error in ReadMessageFrame callback: 2, End of file
[02:04:45 +02:00] [logger main_logger] [debug] [thread 5196] Read buffer:  Read buffer is a nullpointer: false Bytes read: 0 Bytes transferred: 0 Address in nodeData: fe80::8ca:3732:f751:18e8%enp0s1 Endpoint address: fe80::8ca:3732:f751:18e8%enp0s1 State: 1
[02:04:47 +02:00] [logger main_logger] [trace] [thread 5195] BVApp_ConsoleClient: HandleServiceDeregistration called
[02:04:47 +02:00] [logger main_logger] [trace] [thread 5195] App, HandleServiceDeregistration: removed ProBoopens.local.
[02:04:47 +02:00] [logger main_logger] [trace] [thread 5195] BVTCPConnectionManager: Removed session 0 for ProBoopens.local
[02:04:47 +02:00] [logger main_logger] [info] [thread 5195] App, HandleServiceDeregistration: Currently: 1 Services in serviceV:
[02:04:47 +02:00] [logger main_logger] [info] [thread 5195] 1: BupsioBup.local
[02:04:47 +02:00] [logger main_logger] [trace] [thread 5195] BVApp_ConsoleClient: HandleServiceDeregistration called
[02:04:47 +02:00] [logger main_logger] [warning] [thread 5195] App, HandleServiceDeregistration: ProBoopens.local not found in serviceV!
[02:04:47 +02:00] [logger main_logger] [info] [thread 5195] App, HandleServiceDeregistration: Currently: 1 Services in serviceV:
[02:04:47 +02:00] [logger main_logger] [info] [thread 5195] 1: BupsioBup.local
[02:04:49 +02:00] [logger main_logger] [trace] [thread 5196] Accept successful. Requesting identification from the peer.
[02:04:49 +02:00] [logger main_logger] [debug] [thread 5196] WriteMessageFrame: data size: 128
[02:04:49 +02:00] [logger main_logger] [debug] [thread 5196] WriteMessageFrame: dataLen: 128
[02:04:49 +02:00] [logger main_logger] [debug] [thread 5196] Session [5]: Writebuffer: ðDMð\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00
[02:04:49 +02:00] [logger main_logger] [trace] [thread 5196] Session [5]: Written 138 bytes
[02:04:49 +02:00] [logger main_logger] [trace] [thread 5196] Session [5]: Written all bytes
[02:04:49 +02:00] [logger main_logger] [trace] [thread 5196] Session [5]: Read 138 bytes
[02:04:49 +02:00] [logger main_logger] [trace] [thread 5196] Session [5]: Received _HELLOBACK with payload='ProBoopens.local'. Calling Manager handler.
[02:04:49 +02:00] [logger main_logger] [trace] [thread 5196] BVTCPConnectionManager: Established connection with node: ProBoopens.local Address: fe80::8ca:3732:f751:18e8%enp0s1
[02:04:49 +02:00] [logger main_logger] [trace] [thread 5196] BVTCPConnectionManager: Current sessions:
[02:04:49 +02:00] [logger main_logger] [trace] [thread 5196] Session 0 : ID: 4, service: BupsioBup.local
[02:04:49 +02:00] [logger main_logger] [trace] [thread 5196] Session 1 : ID: 5, service: ProBoopens.local
[02:04:49 +02:00] [logger main_logger] [trace] [thread 5196] Session [5]: Read all bytes 138
[02:04:49 +02:00] [logger main_logger] [trace] [thread 5195] App: HandlePublishedServices is called.
[02:04:49 +02:00] [logger main_logger] [trace] [thread 5195] App, HandlePublishedServices: Added ProBoopens.local to serviceV
[02:04:49 +02:00] [logger main_logger] [trace] [thread 5195] App: Sending request to Discovery to resolve ProBoopens.local
[02:04:49 +02:00] [logger main_logger] [trace] [thread 5195] App: HandlePublishedServices is called.
[02:04:49 +02:00] [logger main_logger] [trace] [thread 5195] App: HandleResolvedServices ENTER
[02:04:49 +02:00] [logger main_logger] [trace] [thread 5195] App: Resolved ProBoopens.local to hosttarget: ProBoopens.localalchathost._tcp
[02:04:49 +02:00] [logger main_logger] [trace] [thread 5195] App: on port 50001
[02:04:49 +02:00] [logger main_logger] [trace] [thread 5195] BVApp_ConsoleClient::ResolveServiceToEndpoint: Resolving host ProBoopens.localalchathost._tcp on port: 50001
__[02:04:49 +02:00] [logger main_logger] [warning] [thread 5195] App: Resolve attempt failed... Retrying...__
[02:04:49 +02:00] [logger main_logger] [warning] [thread 5195] App: Resolve attempt failed... Retrying...
[02:04:49 +02:00] [logger main_logger] [warning] [thread 5195] App: Resolve attempt failed... Retrying...
[02:04:49 +02:00] [logger main_logger] [warning] [thread 5195] App: Resolve attempt failed... Retrying...
__[02:04:50 +02:00] [logger main_logger] [warning] [thread 5195] App: Resolve attempt failed... Retrying...__
__[02:04:50 +02:00] [logger main_logger] [error] [thread 5195] App: Error while resolving to... asio.netdb:1__
__[02:04:50 +02:00] [logger main_logger] [error] [thread 5195] App: Error while resolving info Host not found (authoritative) asio.netdb__
Maybe increase retrial attempts?
[UNSOLVED]