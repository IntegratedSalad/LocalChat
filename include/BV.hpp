#pragma once
#include <string>

typedef enum class BVThrMessage
{
    BVTHRMESSAGE_PAUSE,
    BVTHRMESSAGE_CONTINUE,
    BVTHRMESSAGE_EXIT
} BVThrMessage;

// BVActorMessage?
// messages sent between actors - someone disconnected, someone sent a message

typedef enum class BVStatus
{
    // General flags
    BVSTATUS_OK,
    BVSTATUS_NOK,
    BVSTATUS_IN_PROGRESS,
    BVSTATUS_FATAL_ERROR
} BVStatus;

struct BVServiceBrowseInstance
{
    std::string serviceName;
    std::string regType;
    std::string replyDomain;
};
