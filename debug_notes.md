# Problem with mdnsd daemon
## Useful commands
sudo launchctl load /System/Library/LaunchDaemons/com.apple.mDNSResponder.plist -> launch daemon
sudo launchctl list -> list PIDs of daemons, agents and XPC services 
launchctl dumpstate -> view information about running daemons, agents and XPC services
launchctl dumpstate | grep -iE ".*mdns" -> view information about mdns daemon

580 -> PID