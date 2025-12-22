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
nm -g .dylib -> list symbols within shared lib
nm -D /path/to/.so -> list symbols in file.

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
When linked to libdns_sd.dylib, it somehow conflicted communication with the daemon.
Now I know, why including dns_sd.h works - symbols are found in libSystem.B.dylib, which is automatically linked to my binary.

# One DNS Service being found multiple times when Browsing for it.
Try to register another service after the initial has been found, and see if it appears repeated.
No, it appears once.
But - after I terminate the app on the other machine, it still calls DNSServiceProcessResult.
One thing is that we can see if something was already pushed.
Either we deal with this in BVDiscovery component or in App.
For example - BVApp consumes queue and makes Service List that graphical/TCP components utilize.
When consuming queue and updating list there should be a check if next queue element is in list already.

# How to announce that a service has disconnected on Bonjour implementation?
I think logic that handles communication and application itself can be written so that it can communicate
with both implementations.

## Corruption debugging
When something gets corrupted by multithreaded access and process dumps a core:

*build with this (at least on Linux):*
```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS_DEBUG="-g -Og -fno-omit-frame-pointer"*
*cmake --build build*
./Localchat in the build/ subdirectory
coredumpctl gdb LocalChat
```
*then backtrace:*
```
bt
frame 0 // find the frame in this example 0:
where
```