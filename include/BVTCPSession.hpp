#include "BV.hpp"
#include <boost/asio.hpp>
#include "threadsafequeue.hpp"
#include "BVMessage.hpp"
#include "BVLoggable.hpp"
#include "BVTCPCommon.hpp"

/*
 * Each and every connection we want to handle separately.
 * This class should implement and define functionality
 * that handles the connection with one service, started by accepting that connection.
 * What class should be responsible when we are trying to initiate (connect) to another service?
 * This should handle both incoming and outgoing traffic
 * 
 * This class communicates with App to update its data regarding communication with other nodes
*/

// Sessions can call manager's callbacks
class BVTCPSession : public BVLoggable
{
private:

    boost::asio::io_context ioContext;
    std::unique_ptr<BVTCPNodeConnectionSessionData> sessionData_p;
    // std::thread worker_thread;

    void Read(void);
    void Write(const BVMessage& message);

public:
    BVTCPSession(std::unique_ptr<BVTCPNodeConnectionSessionData> _sessionData_p,
                 boost::asio::io_context& _ioContext);

    void Start(void);
    void Shutdown(void);

    ~BVTCPNodeConnectionSessionData();
};
