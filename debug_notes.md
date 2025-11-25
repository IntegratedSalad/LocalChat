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

# How to announce that a service has disconnected?

# Installing linux dependencies
Sometimes even if a certain package is installed, there are no header files that come with that library.
It means that the .so files were compiled with the header files, but the package doesn't come with it.
It is helpful to find a dev package.
mDNSPosix tries to include mbedtls/certs.h which is not present in version 3.x.x of mbedtls
Might switch to 2.x.x but not system-wide.
It is because I'm using the old mDNSResponder version!
Latest doesn't include mbedtls/certs.h...
Okay - symbols that are referenced in dns-sd.h on MacOS are present in the libSystemB.dylib which is linked by default. In Linux, I have to somehow provide a libdns_sd.so.
nm -D /path/to/.so/ -> list symbols in file.
## Compatibility library problems
If using avahi compatibility layer is too hard/demanding - just compile the mDNSResponder daemon and use full Bonjour implementation. But this means that we have to disable the avahi daemon - it's not optimal.
We might just have to do two targets - BV_Bonjour and BV_Avahi.
Application will be the same for each of the implementations...
In case of having Avahi and Bonjour, we have to make abstract clasess more precise.
Discovery should have the same interface - only implementation changes.
Bonjour and Avahi should put BrowseInstances on queue.
First - compile on linux the example.
Maybe get around first in the hacky way - at least to see that two services in the in-progress-app can
discover each other.
This is only talking about the mDNS and DNS-SD functionality, so service registration, browsing and resolution.
I think logic that handles communication and application itself can be written so that it can communicate
with both implementations.