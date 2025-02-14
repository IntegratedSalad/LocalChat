# Design
Because what I will write here will be used for BonVoyage, a graphical client for sharing files, we should
design an interface that takes out some of the core functionality of DNS-SD.

BV - BonVoyage, suite name
LocalChat is an application providing simple messaging utility over mDNS.
FileSharing is an application providing simple file exchaning utility over mDNS.
## Classes
## Class 'BVService'
Description:
This class embeds the DNS-SD Service Registration functionality:
    1. Service Registration (and handling the response from daemon),
    2. ?Discovery of Browsing and Registration Domains (Domain Enumeration),
       Although we will for now, use only .local (and should only use local?)
    3. Record handling (optionally)?
    4. Service deregistration and deallocation of resources (DNSServiceRef).
       Question: does DNSServiceRef for DNSServiceRegister has to be deallocated
       after reading the reply from the daemon?

Other:
Service has a fixed, constant type: _localchathost._tcp. Name is the host name providing this service.
## Class 'BVDiscovery'
Description:
This class embeds the DNS-SD Service Discovery functionality:
    1. Discovery of instances (hosts) of localchathost service.
    2. Resolving a host name of a service (to IPv4).

How to interface with asio? Discovery is an asynchronous task.
This is a good idea, BVDiscovery can be a function object.

## Class 'BVActor'
Description:
An actor, an instance that acts like a user would.
It has service, it can discover other services.
It should be independent of the implementation (Avahi/Bonjour)
BVActor can run all threads and define mutexes and resources for BVXX functionality.

!Wydaje mi sie, ze nalezy zrobic pewna abstrakcje. dns_sd.h definiuje pare operacji, po ktorych nalezy czekac na odpowiedz od daemona.
Czy nie da sie zrobic jakiegos systemu ktory by byl abstrakcją tego? Czy to jest potrzebne?
Poniewaz teraz BVActor musi wyszukac, czy istnieje juz nazwa hosta o podanym serwisie. Jezeli istnieje, to nie rejestruj jeszcze raz.
W zasadzie taka abstrakcja wymagana bylaby, w momencie gdybym chcial rozszerzyc funkcjonalnosc mDNSResponder albo funkcji, ktore 
wymagaja kontaktu z demonem byloby bardzo duzo.

Byc moze najpierw zrobmy te funkcjonalnosc w BVDiscovery, gdzie BVActor zarzadza wywolywaniem tych dwoch funkcjonalnosci.
To ma swoj koszt - musimy rozroznic znow pomiedzy Bonjour i Avahi.
Chyba, ze najlepiej zrobic BVActor ktory dziala niezaleznie od implementacji. Jednakze, najlatwiej bedzie zrobic prototyp, gdzie bedzie klasa
BVActor_Bonjour i BVActor_Avahi

Record handling?

## GUI library:
FTLK or raygui

Logging functionality?

## How BV operates
## LocalChat
# Application Overview
LocalChat serves as a messaging utility over local network (LAN).
It utilizes mDNS and DNS-SD networking protocols in order to connect OS agnostic hosts in the same network, 
without setting up a DNS server, or utilizing any other external server, that would exchange messages,
manage sessions and provide other utilities, required from this type of entity.
LocalChat uses FLTK to provide a GUI for a user; a panel with available hosts within the network,
text field, and other widgets.

# LocalChat flow
1. Initialization of BVService_Bonjour (for now class that supports only Bonjour)
2. Registration of service (TODO: provide complete service name) (_localchat._tcp.local?),
   where .local is mandatory, because mDNS exclusively resolves hostnames ending with the .local top-level domain[^1].
   [^1]: [Wikipedia on mDNS](https://en.wikipedia.org/wiki/Multicast_DNS#Protocol_overview)
   1. If service was already registered, do not register it twice. (TODO: Should this application work in background?)
      meaning, if someone writes a message to a user, where their application was closed, (but not the service)
      should they receive the messages?
3. UI initialization. This can mean a CLI or GUI aplication. (TODO: Build for different targets?)
4. Main Program Loop:
   1. Accept user input AND
   2. Discovery for localchat services periodically (timer) AND
   3. Notify user when someone is available to chat AND (Thread handling this will block until specific ?message? comes)
   4. Output messages sent to user (callback upon receiving data on socket)
   Main problem is, that discovery for localchat services etc. needs to call callbacks/completion handlers upon
   finished task.

# Threading
## Boost threadpool
boost::threadpool allows for the automatic thread management.
## Threads needed
First should take care of registration and announce if it was succesful or not.
It has to register the service with hostname, and wait for the daemon to reply.

Second thread handles GUI.

Third thread handles communication over TCP.
