# Avahi
## Example
[Example](https://avahi.org/doxygen/html/client-browse-services_8c-example.html#a30)
Provide _localchathost._tcp as type when calling avahi_service_browser_new
Compile this with:
gcc file -o browse_ex -lavahi-common -lavahi-client
Run and register a service of said type to see it being browsed - this should confirm that avahi can find services registered on Mac/Linux.
