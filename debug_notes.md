# Problem with mdnsd daemon
There might be a dynamic link mismatch between what dns-sd util is linked against and my binary.
otool -L LocalChat -> :
LocalChat:
	**build/prod/libdns_sd.dylib (compatibility version 0.0.0, current version 0.0.0)**
	/usr/lib/libc++.1.dylib (compatibility version 1.0.0, current version 1300.23.0)
	**/usr/lib/libSystem.B.dylib (compatibility version 1.0.0, current version 1311.100.3)**

## Useful commands
sudo launchctl load /System/Library/LaunchDaemons/com.apple.mDNSResponder.plist -> launch daemon
sudo launchctl list -> list PIDs of daemons, agents and XPC services 
launchctl dumpstate -> view information about running daemons, agents and XPC services
launchctl dumpstate | grep -iE ".*mdns" -> view information about mdns daemon
ls -lah /var/run -> list all daemons
otool -L LocalChat -> list object files

## Useful dirs
/usr/bin -> binaries
/var/run -> daemons

502 -> PID

# Solution:
**We don't need to compile *anything* from mDNSPosix.**
**mDNSResponder daemon, and mDNSResponderHelper are installed system-wide.**
**And, we don't have to link it with the libdns_sd.dylib, only *libSystem.B.dylib***
**libSystem.B.dylib has symbols found in dns_sd.h**
When we look at otool -L /usr/bin/dns-sd, we can see that it has only one shared library - *libSystem.B.dylib*.
