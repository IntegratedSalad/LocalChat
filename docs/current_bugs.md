From logs:
[15:43:08 +02:00] [logger main_logger] [trace] [thread 5811141] Session [1]: Received _HELLOBACK_ with payload='fedora-thinkpad'. Calling Manager handler.
[15:43:08 +02:00] [logger main_logger] [trace] [thread 5811141] BVTCPConnectionManager: Established connection with node: fedora-thinkpad Address: ::ffff:192.168.0.74
[15:43:08 +02:00] [logger main_logger] [trace] [thread 5811141] BVTCPConnectionManager: Current sessions:
[15:43:08 +02:00] [logger main_logger] [trace] [thread 5811141] Session 0 : ID: __0__, service: fedora
[15:43:08 +02:00] [logger main_logger] [trace] [thread 5811141] Session 1 : ID: __0__, service: BupsioBup.local
[15:43:08 +02:00] [logger main_logger] [trace] [thread 5811141] Session 2 : ID: __1__, service: fedora-thinkpad

The sessionData->sessionID (SessionID) is assigned (it seems) non-atomically.
