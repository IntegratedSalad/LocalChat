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
class BVTCPSession : public BVLoggable, std::enable_shared_from_this<BVTCPSession>
{
private:
    boost::asio::io_context& ioContext;
    std::shared_ptr<BVTCPNodeConnectionSessionData> sessionData_p;
    // std::thread worker_thread;

    void Read(void);
    void Write(void);
    void WriteCallback(const boost::system::error_code& ec,
                       std::size_t bytes_transferred,
                       std::shared_ptr<BVTCPNodeConnectionSessionData> sessionData_p)
    {
        if (ec)
        {
            std::cerr << "Error while writing to a socket! " << ec.value() << std::endl;
            std::cerr << "Message: " << sessionData_p->buf << std::endl;
            return; // TODO: 
        }
        sessionData_p->totalBytesWritten += bytes_transferred;

        if (sessionData_p->totalBytesWritten == sessionData_p->buf.length())
        {
            return;
        }

        sessionData_p->sock->async_write_some(
            boost::asio::buffer(sessionData_p->buf),
            std::bind(&BVTCPSession::WriteCallback, this, std::placeholders::_1, std::placeholders::_2, this->sessionData_p)
        );
    }

public:
    BVTCPSession(std::shared_ptr<BVTCPNodeConnectionSessionData> _sessionData_p,
                 boost::asio::io_context& _ioContext);

    void Start(void);
    void Shutdown(void);

    const BVTCPNodeConnectionSessionData* GetSessionData(void) const
    {
        return this->sessionData_p.get();
    }

    // for now only text
    void RequestWrite(const std::string& data);

    ~BVTCPSession() {};
};
