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

#define WAIT_FOR_SENDING_MS 400

// File transfer is always outgoing?
// What will the context be for receiving a message?
// Just handling it in app
// and concatenating it to a one file?

// TODO:
// Rename this file to BVFileTransferContext.hpp
// classes: BVFileTransferContext (outgoing) and BVFileIncoming
// 

enum class FileTransferState
{
    FILETRANSFERSTATE_FIRST_CHUNK,
    FILETRANSFERSTATE_ONGOING,
    FILETRANSFERSTATE_LAST_CHUNK
};

class BVFileTransferContext : public BVLoggable
{
private:
    std::fstream fhandle;
    const std::uint32_t  fsize;
    const uint16_t       ftcid; // id of the BVFileTransferContext
    const std::string    fname;
    
    std::shared_ptr<BVTCPSession> session_p;
    MailboxGetter mailbox_F; // this will directly send messages to app. But for what?

    FileTransferState state = FileTransferState::FILETRANSFERSTATE_FIRST_CHUNK;
    std::uint32_t csize; // chunk size
    std::uint32_t bytesSent = 0;
    std::atomic_bool isRunning{true};

    std::thread worker_thread;

    // I think we will need to wait before transmitting another chunk...
    // Because we are still writing to the readBuf of the receiving session, and this is not a buffer
    // that is resized to fit chunks.
    // The best architecture is to wait for confirmation from the other host that it received
    // BVSESSIONREGULARMESSAGETYPE_FILE_TRANSFER_BEGIN
    // We can also wait for fixed time amount

    // I think that issue with ordering/timing still persists.
    void TransferNextChunk(void)
    {
        uint8_t msgType;
        uint64_t metadata = 0;
        if (state == FileTransferState::FILETRANSFERSTATE_FIRST_CHUNK)
        {
            std::string payloadStr = 
                this->session_p->GetSessionData()->thisMachineServiceName + "|" + fname;
            std::vector<char> ftBeginPayload{payloadStr.begin(), payloadStr.end()};
            ftBeginPayload.push_back('\0'); // information for session where to stop processing
            msgType = BVTCPMessageType::BVSESSIONREGULARMESSAGETYPE_FILE_TRANSFER_BEGIN;
            // We put fsize on 32 high bits and csize on 32 low bits.
            metadata = ((uint64_t)csize << 32) | ((uint64_t)fsize);
            state = FileTransferState::FILETRANSFERSTATE_ONGOING;
            BVTCPFileHeader fChunkHeader = ConstructFileHeader(msgType, csize, metadata); 
            BVTCPFileChunk  fChunk       = ConstructFileChunk(fChunkHeader, ftBeginPayload);
            // Payload: Servicename|Filename\0 (with extension)
            /*
                BUG: Writing/reading mangled data SOLUTION.
                Okay - we were writing less than 138 bytes to the socket,
                but receiving end (readBuf on another host) read 138 bytes (expected message frame size).
                This was ok when receiving BVSESSIONREGULARMESSAGETYPE_FILE_TRANSFER_BEGIN,
                but when receiving another message, the stream was shifted 138-payload bytes, past
                the next data, which was actually past the whole header 
                (receiving end didn't receive enough and waited for remaining X bytes up to 138 to complete async_read).
                So when receiving FILE_TRANSFER_BEGIN, the other host read header of the next packet (thus shifting the stream).
                This is why we couldn't parse the chunk header.
                Below difference HAS to be greater or equal than ftBeginPayload.size().
            */
            session_p->WriteFileChunk(fChunk, MESSAGE_FRAME_SIZE_BYTES - FILE_HEADER_SIZE_BYTES);
            LogTrace("[BVFileTransferContext]: Sent FILE_TRANSFER_BEGIN of size: {}", csize);
            LogTrace("[BVFileTransferContext]: File name: {}", fname);
            LogDebug("[BVFileTransferContext]: Metadata raw: {}", metadata);
            LogTrace("[BVFileTransferContext]: Waiting for buffer change...");
            std::cout << std::endl;
            std::cout << "Starting file transfer of " << fname << "..." << std::endl;
            return;
        }
        std::vector<char> dataToTransferBuffer(csize);
        fhandle.read(dataToTransferBuffer.data(), 
            static_cast<std::streamsize>(dataToTransferBuffer.size()));
        const std::streamsize bytesRead = fhandle.gcount();
        if (bytesRead > 0)
        {
            LogTrace("[BVFileTransferContext]: Read {} bytes from file.", bytesRead);
            if (state == FileTransferState::FILETRANSFERSTATE_ONGOING)
            {
                if (bytesSent + csize >= fsize)
                {
                    state = FileTransferState::FILETRANSFERSTATE_LAST_CHUNK;
                } else
                {
                    msgType = BVTCPMessageType::BVSESSIONREGULARMESSAGETYPE_FILE_TRANSFER_CHUNK_SENT;
                    state = FileTransferState::FILETRANSFERSTATE_ONGOING;
                    LogTrace("[BVFileTransferContext]: Sent FILE_TRANSFER_CHUNK_SENT of size: {}", csize);
                    std::cout << "Sending..." << std::endl;
                }
            } 
            if (state == FileTransferState::FILETRANSFERSTATE_LAST_CHUNK)
            {
                msgType = BVTCPMessageType::BVSESSIONREGULARMESSAGETYPE_FILE_TRANSFER_END;
                state = FileTransferState::FILETRANSFERSTATE_ONGOING;
                LogTrace("[BVFileTransferContext]: Sent FILETRANSFERSTATE_FILE_TRANSFER_END of size: {}", csize);
                std::cout << "Sent!..." << std::endl;
                isRunning = false;
            }
            BVTCPFileHeader fChunkHeader = ConstructFileHeader(msgType, csize, metadata); 
            BVTCPFileChunk  fChunk       = ConstructFileChunk(fChunkHeader, dataToTransferBuffer);
            // This shouldn't be the solution but for now it will suffice.
            std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_FOR_SENDING_MS));
            session_p->WriteFileChunk(fChunk, csize);
            bytesSent += bytesRead;  
        } else
        {
            isRunning = false;
        }
    }

    void DetermineChunkSize(void)
    {
        constexpr std::size_t chunkSizeAbsoluteMinimum = 64;
        if (fsize < MIN_FILE_CHUNK_SIZE_BYTES_256B)
        {
            csize = chunkSizeAbsoluteMinimum; // if the file is really small then send max 64*4 chunks
        } else if (fsize < FILE_SIZE_BYTES_1KB)
        {
            csize = MIN_FILE_CHUNK_SIZE_BYTES_256B;
        } else if (fsize > FILE_CHUNK_SIZE_BYTES_1KB && fsize < FILE_SIZE_BYTES_8KB)
        {
            csize = FILE_CHUNK_SIZE_BYTES_512B;
        } else if (fsize > FILE_SIZE_BYTES_8KB && fsize < FILE_SIZE_BYTES_64KB)
        {
            csize = FILE_CHUNK_SIZE_BYTES_4KB;
        } else if (fsize > FILE_SIZE_BYTES_64KB && fsize < FILE_SIZE_BYTES_256KB)
        {
            csize = FILE_CHUNK_SIZE_BYTES_32KB;
        } else if (fsize > FILE_SIZE_BYTES_256KB && fsize < FILE_SIZE_BYTES_1MB)
        {
            csize = FILE_CHUNK_SIZE_BYTES_64KB;
        } else if (fsize > FILE_SIZE_BYTES_1MB && fsize < FILE_SIZE_BYTES_5MB)
        {
            csize = FILE_CHUNK_SIZE_BYTES_512KB;
        } else
        {
            csize = MAX_FILE_CHUNK_SIZE_BYTES_1MB;
        }
        // } else if (fsize > ) TODO: OTHER CASES
        LogTrace("[BVFileTransferContext]: Chosen chunk size: {}", csize);
    }
    
public:
    BVFileTransferContext(std::shared_ptr<BVTCPSession> _session_p,
                        std::filesystem::path& _fpath,
                        const uint16_t _ftcid,
                        MailboxGetter _mailbox_F) :
    session_p(_session_p),
    fsize(std::filesystem::file_size(_fpath)),
    fname(std::filesystem::path(_fpath).filename()),
    ftcid(_ftcid),
    mailbox_F(_mailbox_F)
    {
        // 1. Get file size
        // 2. Determine chunk size
        // 3. Create file handle
        DetermineChunkSize();
        fhandle = std::fstream{_fpath, fhandle.binary | fhandle.in};
        if (!fhandle.is_open())
        {
            LogError("[BVFileTransferContext]: Failed to open file for: {}", _fpath.string());
            // TODO: send BVEVENTTYPE_APP_FILE_TRANSFER_CANCELLED
        } else
        {
            LaunchFileTransfer();
        }
    }

    void LaunchFileTransfer(void)
    {
        worker_thread = std::thread([&] {
            while (isRunning)
            {
                if (fhandle)
                {
                    this->TransferNextChunk();
                }
            }
        });
        // if (worker_thread.joinable())
        // {
        //     worker_thread.join();
        // }
    }

    void CancelFileTransfer(void)
    {
        this->isRunning = false;
        fhandle.close();

        // send message that file transfer has been cancelled.
    }
    ~BVFileTransferContext()
    {
        LogTrace("[BVFileTransferContext]: FTContext id: {} dies.", ftcid);
        if (worker_thread.joinable())
        {
            worker_thread.join();
        }
    }
};
