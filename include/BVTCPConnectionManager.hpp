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

    // Send data to chosen node. This is an interface for App
    BVStatus SendDataToService(const ChatMessage& msg)
    {
        BVStatus idStatus;
        const NodeID nodeId = msg.metadata.recipient; //this->GetNodeIDByServiceName(serviceName, idStatus);
        if (idStatus == BVStatus::BVSTATUS_OK)
        {
            {
                std::lock_guard<std::mutex> l(this->session_m_mutex);
                this->sessions_m.at(nodeId)->RequestWrite(msg.textData);
                // this->sessions_m.at(nodeId)->sock.async_write_some(); // to implement
            }
        } else
        {
            return BVStatus::BVSTATUS_NOK;
        }
    }
    
    ~BVTCPConnectionManager();
};
