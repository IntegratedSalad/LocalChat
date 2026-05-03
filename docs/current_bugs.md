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
It seems that linux discovery and tcp have some issues.
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