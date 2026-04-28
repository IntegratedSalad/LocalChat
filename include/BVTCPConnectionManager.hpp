#pragma once
#include <string>
#include <iostream>
#include <boost/asio.hpp>
#include "BV.hpp"
#include "BVLoggable.hpp"
#include "BVService.hpp"
#include "threadsafequeue.hpp"
#include "BVMessage.hpp"
#include "BVTCPCommon.hpp"
#include "BVTCPSession.hpp"
#include <arpa/inet.h>
#include <map>
#include <mutex>


// // These are messages sent between hosts
// typedef enum class BVTCPMessageType
// {
//     BVTCPMESSAGETYPE_HANDSHAKE,
//     BVTCPMESSAGETYPE_DEREGISTRATION, // ? 
//     BVTCPMESSAGETYPE_TEXT_MESSAGE,
//     BVTCPMESSAGETYPE_FILE,

// } BVTCPMessageType;

/*
 * BVTCPConnectionManager
 * As part of the BVTCP Suite.
 * This class manages all connections - these coming in (we accept them)
 * and those that we initiate (we connect to other).
 * 
 * 
*/

// Remember to open a different file to log the connections logs. <- needed?
class BVTCPConnectionManager : public BVLoggable // BVComponent?
{
private:

    SessionID currentSessionID = 0;

    // This machine is this Node in the network.
    const BVServiceData thisMachineServiceData;
    BVNode              thisMachineHostData;
    boost::asio::io_context& ioContext;

    boost::asio::ip::tcp::acceptor acceptorSocket;

    // Map of active connections to other Nodes.
    // Session can be created in one thread,
    // But removed in the other.
    std::map<SessionID, std::shared_ptr<BVTCPSession>> sessions_m;
    std::mutex session_m_mutex;

    // Incoming messages from sessions are put right into App's inMailbox_p
    std::shared_ptr<threadsafe_queue<BVMessage>> appInMailBox_p;

    // Outwards communication queue with App.
    // App just pushes to one of these, doesn't listen to them.
    // BVMessage is also used here as a payload.
    // SessionID, not nodeID as a key
    std::map<NodeID, std::shared_ptr<threadsafe_queue<BVMessage>>> outMailboxes_p;

    std::map<std::string, NodeID> service_nodeid_m;

    // We have to also instantiate some object that will tie service with a nodeID.
    // or at least - provide an interface to App, so that it can just push message to
    // a certain service/node and be done with it.

    // Should BVTCPConnectionManager have another thread to monitor/manage sessions?

    NodeID GetNodeIDByServiceName(const std::string& _serviceName, BVStatus& status_out)
    {
        NodeID id = 0;
        BVStatus status = BVStatus::BVSTATUS_NOK;
        for (auto const& [k, v] : sessions_m)
        {
            if (_serviceName == v->GetSessionData()->nodeData.serviceName)
            {
                id = k;
                status = BVStatus::BVSTATUS_OK;
                break;
            }
        }
        status_out = status;
        return id;
    }
    
public:
    BVTCPConnectionManager(boost::asio::io_context& _ioContext,
                           const BVServiceData _thisMachineServiceData);

    // Upon construction of objects other than for test purposes, instantiate acceptor socket and initialize connection.
    BVStatus StartAcceptingConnections(void);

    /*
        Establish a Node's communication channel with app and the manager
    */
    BVStatus InitiateSessionWithNode(const BVNode nodeData);

    void SetAppInMailBoxP(std::shared_ptr<threadsafe_queue<BVMessage>> p)
    {
        this->appInMailBox_p = p;
    }

    // Set communication channel towards the Node. (their inMailbox_p)
    // TODO: Rename it to set mailbox p or something - this does not start a communication session!
    BVStatus StartCommunicationSessionWithNode(NodeID& nid, std::shared_ptr<threadsafe_queue<BVMessage>>& nodeInMailbox_p)
    {
        static NodeID current_id = 0;
        bool foundEmpty = false;
        BVStatus status = BVStatus::BVSTATUS_OK;
        for (NodeID _id = 0; _id < N_SERVICES_MAX; _id++)
        {
            if (outMailboxes_p.at(_id) == nullptr) // find any empty spot
            {
                nodeInMailbox_p = std::make_shared<threadsafe_queue<BVMessage>>();
                foundEmpty = true;
                break;
            }
        }
        if (!foundEmpty)
        {
            // there are more communication channels trying to be registered, and there is no place.
            status = BVStatus::BVSTATUS_NOK;
        }
        nid = current_id;
        current_id = (current_id % N_SERVICES_MAX) + 1;
        return status;
    }

    bool IsSessionAlreadyPresent(const BVNode& nodeData)
    {
        bool found = false;
        {
            std::lock_guard<std::mutex> l(session_m_mutex);
            for (const auto& [k,v] : this->sessions_m)
            {
                if (v->GetSessionData()->nodeData.serviceName ==
                    nodeData.serviceName)
                {
                    found = true;
                    break;
                }
            }
        }
        return found;
    }

    bool IsSessionAlreadyPresent(const std::string& _serviceName)
    {
        bool found = false;
        {
            std::lock_guard<std::mutex> l(session_m_mutex);
            for (const auto& [k,v] : this->sessions_m)
            {
                if (v->GetSessionData()->nodeData.serviceName == _serviceName)
                {
                    found = true;
                    break;
                }
            }
        }
        return found;
    }

    // Send data to chosen node. This is an interface for App
    template<typename PayloadType>
    BVStatus SendDataToService(const BVTCPMessage<PayloadType> msg)
    {
        BVStatus idStatus;
        const NodeID nodeId = msg.metadata.recipient; //this->GetNodeIDByServiceName(serviceName, idStatus);
        if (idStatus == BVStatus::BVSTATUS_OK)
        {
            {
                std::lock_guard<std::mutex> l(this->session_m_mutex);
                this->sessions_m.at(nodeId)->RequestSomeWrite(msg.textData);
                // this->sessions_m.at(nodeId)->sock.async_write_some(); // to implement
            }
        } else
        {
            return BVStatus::BVSTATUS_NOK;
        }
    }

    void ConnectHandler(const boost::system::error_code& error, 
                        const boost::asio::ip::tcp::endpoint ep,
                        std::shared_ptr<BVTCPNodeConnectionSessionData> sessionData_p)
    {
        if (error)
        {
            LogError("ConnectHandler Error: {} {} {}", error.value(), error.message(), error.category().name());
            return;
        }
        if (this->IsSessionAlreadyPresent(sessionData_p->nodeData))
        {
            LogInfo("ConnectHandler: Session associated with service {} already present.", sessionData_p->nodeData.serviceName);
            return;
        }
        {
            // We probably provide not the host machine, but the service name of the session that we are connecting to. 
            // On the other machine it will be their name
            std::lock_guard<std::mutex> l(session_m_mutex);
            sessionData_p->nodeData.ep = ep;
            std::shared_ptr<BVTCPSession> session_p = std::make_shared<BVTCPSession>(sessionData_p, ioContext);
            session_p->SetLogger(GetLogger());
            StartCommunicationSessionWithNode(session_p->GetSessionData()->nodeData.id, session_p->GetSessionData()->inMailbox_p);
            session_p->SetState(BVSessionState::BVSESSIONSTATE_UNPREPARED);
            session_p->RequestReadingFrames();
            session_p->SetOrigin(BVSessionOrigin::BVSESSIONORIGIN_OUTGOING);
            sessions_m[session_p->GetSessionData()->nodeData.id] = session_p;
            currentSessionID+=1;
        }
        LogTrace("ConnectHandler: Successfuly connected to {}: {}:{}", 
            sessionData_p->nodeData.serviceName, sessionData_p->nodeData.ep.address().to_string(), sessionData_p->nodeData.ep.port());
    }

    void HandleSessionIdentification(const std::string& serviceName, std::shared_ptr<BVTCPSession> caller)
    {
        if (!IsSessionAlreadyPresent(serviceName))
        {
            // This session is not a duplicate - we accepted it, it wasn't added (we didn't know who it was)
            // Now we know - we can add it, didn't connect to it before.
            caller->SetState(BVSessionState::BVSESSIONSTATE_ESTABLISHED);
            {
                std::lock_guard<std::mutex> l(session_m_mutex);

                // Set nodeData - this is not set when accepting!
                StartCommunicationSessionWithNode(caller->GetSessionData()->nodeData.id, caller->GetSessionData()->inMailbox_p);
                caller->GetSessionData()->nodeData.serviceName = serviceName;
                this->sessions_m[caller->GetSessionData()->nodeData.id] = caller;
                this->currentSessionID+=1;
            }
            LogTrace("BVTCPConnectionManager: Established connection with node: {} Address: {}", 
                caller->GetSessionData()->nodeData.serviceName, caller->GetSessionData()->nodeData.address.to_string());

            // When we now have serviceName, we have to get the ip address with that service name
            // Although that's weird, because we should already have their IP.
            // We have their IP when we connect, but when we accept, we do not.
            // We should get that IP From app, or the node should send it themselves.
            // The session with which we talk, is the session to US.
            LogTrace("BVTCPConnectionManager: Current sessions:");
            {
                std::lock_guard<std::mutex> l(session_m_mutex);
                int sidx = 0;
                for (const auto& [k,v] : this->sessions_m)
                {
                    LogTrace("Session {} : ID: {}, service: {}", sidx, v->GetSessionData()->sessionID, v->GetSessionData()->nodeData.serviceName);
                    sidx++;
                }
            }
            caller->RequestReadingFrames();
        } else
        {
            // This session is a duplicate - we might've accepted and connected at the same time.
            // Now we know who it is; we have this peer as a session, so we can close this one.
            LogTrace("BVTCPConnectionManager: Found duplicate session for {}. Closing.", caller->GetSessionData()->nodeData.serviceName);
            caller->Close(); // close the duplicate session
        }
    }

    ~BVTCPConnectionManager();
};
