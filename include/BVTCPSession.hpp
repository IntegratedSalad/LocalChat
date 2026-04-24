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
class BVTCPSession : public BVLoggable, public std::enable_shared_from_this<BVTCPSession>
{
private:
    boost::asio::io_context& ioContext;
    std::shared_ptr<BVTCPNodeConnectionSessionData> sessionData_p;
    // std::thread worker_thread;

    void Read(void);
    void Write(void); // data? or is it written in session Data
    void WriteCallback(const boost::system::error_code& ec,
                       std::size_t bytes_transferred)
    {
        if (ec)
        {
            LogError("Session [{}]: Error while writing to a socket! {}, {}. Message: {}",  
                this->GetSessionData()->sessionID, ec.value(), ec.message(), sessionData_p->writeBuf);
                return;
            return;
        }
        this->sessionData_p->totalBytesWritten += bytes_transferred;

        if (this->sessionData_p->totalBytesWritten == this->sessionData_p->writeBuf.length())
        {
            return;
        }

        this->sessionData_p->sock->async_write_some(
            boost::asio::buffer(sessionData_p->writeBuf),
            std::bind(&BVTCPSession::WriteCallback, this, std::placeholders::_1, std::placeholders::_2)
        );
    }

    void ReadCallback(const boost::system::error_code& ec,
                      std::size_t bytes_transferred)
    {
        if (ec)
        {
            LogError("Session [{}]: Error in read callback: {}, {}",  
                this->GetSessionData()->sessionID, ec.value(), ec.message());
                return;
        }
        this->sessionData_p->totalBytesRead += bytes_transferred;
        if (this->sessionData_p->totalBytesRead == MAX_MESSAGE_SIZE_BYTES)
        {
            return;
        }
        this->sessionData_p->sock->async_read_some(
            boost::asio::buffer(this->sessionData_p->readBuf.get() + this->sessionData_p->totalBytesRead,
                MAX_MESSAGE_SIZE_BYTES - this->sessionData_p->totalBytesRead),
            std::bind(&BVTCPSession::ReadCallback, this, std::placeholders::_1, std::placeholders::_2)
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
