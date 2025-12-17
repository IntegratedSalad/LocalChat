# General
## VM
Developed on 12GB space 8GB RAM Fedora Workstation 43 on aarch64
Network must be bridged. Without it, VM will be isolated from the LAN.
## Setup
For development and VM, install packages from sys/fedora-linux-software
Preferred dev environment is vscode with: 
CMake, CMake Tools, C/C++, C/C++ Extension Pack, C++ TestMate and PlantUML
installed.

# Avahi
## Example
To run a basic example:
[Example](https://avahi.org/doxygen/html/client-browse-services_8c-example.html#a30)
Provide _localchathost._tcp as type when calling avahi_service_browser_new
Compile this with:
gcc file -o browse_ex -lavahi-common -lavahi-client
Run and register a service of said type to see it being browsed - this should confirm that avahi can find services being registered.
