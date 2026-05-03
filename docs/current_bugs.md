From logs:
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
