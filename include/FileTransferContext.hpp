#pragma once
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <atomic>
#include <boost/asio.hpp>
#include "BVTCPCommon.hpp"
#include "BVTCPSession.hpp"
#include "BVLoggable.hpp"

// File transfer is always outgoing?
enum class FileTransferState
{
    FILETRANSFERSTATE_FIRST_CHUNK,
    FILETRANSFERSTATE_ONGOING,
    FILETRANSFERSTATE_LAST_CHUNK
};
class FileTransferContext : public BVLoggable
{
private:
    std::fstream fhandle;
    const std::size_t  fsize;
    const uint16_t     ftcid; // id of the FileTransferContext
    
    std::shared_ptr<BVTCPSession> session_p;
    MailboxGetter mailbox_F; // this will directly send messages to app.    

    FileTransferState state = FileTransferState::FILETRANSFERSTATE_FIRST_CHUNK;
    std::size_t  csize; // chunk size
    std::size_t  bytesSend = 0;
    std::atomic_bool isRunning{true};

    std::thread worker_thread;

    void TransferNextChunk(void)
    {
        if (state == FileTransferState::FILETRANSFERSTATE_FIRST_CHUNK)
        {
            // Construct BVSESSIONREGULARMESSAGETYPE_FILE_TRANSFER_BEGIN

            state = FileTransferState::FILETRANSFERSTATE_ONGOING;
            return;
        } else if (state == FileTransferState::FILETRANSFERSTATE_ONGOING)
        {
            return;
        } else if (state == FileTransferState::FILETRANSFERSTATE_LAST_CHUNK)
        {
            return;
        }
    }

    void DetermineChunkSize(void)
    {
        constexpr std::size_t chunkSizeAbsoluteMinimum = 64;
        if (fsize < MIN_FILE_CHUNK_SIZE_BYTES_256B)
        {
            csize = chunkSizeAbsoluteMinimum; // if the file is really small then send 64*4 chunks
        } else if (fsize < FILE_SIZE_BYTES_1KB)
        {
            csize = MIN_FILE_CHUNK_SIZE_BYTES_256B;
        } else if (fsize > FILE_CHUNK_SIZE_BYTES_1KB && fsize < FILE_SIZE_BYTES_8KB)
        {
            csize = FILE_CHUNK_SIZE_BYTES_512B;
        } else if (fsize > FILE_SIZE_BYTES_8KB && fsize < FILE_SIZE_BYTES_64KB)
        {
            csize = FILE_CHUNK_SIZE_BYTES_1KB;
        } else if (fsize > FILE_SIZE_BYTES_64KB && fsize < FILE_SIZE_BYTES_256KB)
        {
            csize = FILE_CHUNK_SIZE_BYTES_4KB;
        } else if (fsize > FILE_SIZE_BYTES_256KB && fsize < FILE_SIZE_BYTES_1MB)
        {
            csize = FILE_CHUNK_SIZE_BYTES_16KB;
        } else if (fsize > FILE_SIZE_BYTES_1MB && fsize < FILE_SIZE_BYTES_5MB)
        {
            csize = FILE_CHUNK_SIZE_BYTES_64KB;
        } else
        {
            csize = FILE_CHUNK_SIZE_BYTES_512KB;
        }
        // } else if (fsize > ) TODO: OTHER CASES
    }
    
public:
    FileTransferContext(std::shared_ptr<BVTCPSession> _session_p,
                        std::filesystem::path& _fpath,
                        const uint16_t _ftcid,
                        MailboxGetter _mailbox_F) :
    fsize(std::filesystem::file_size(_fpath)),
    ftcid(_ftcid),
    mailbox_F(_mailbox_F),
    session_p(_session_p)
    {
        // 1. Get file size
        // 2. Determine chunk size
        // 3. Create file handle
        DetermineChunkSize();
        fhandle = std::fstream{_fpath, fhandle.binary | fhandle.in};
        LaunchFileTransfer();
    }

    void LaunchFileTransfer(void)
    {
        worker_thread = std::thread([&] {
            while (isRunning)
            {
                this->TransferNextChunk();
            }
        });
        if (worker_thread.joinable())
        {
            worker_thread.join();
        }
    }

    void CancelFileTransfer(void)
    {
        this->isRunning = false;

        // send message that file transfer has been cancelled.
    }
    ~FileTransferContext(){fhandle.close();}
};
