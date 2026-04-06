#pragma once

// why uint8_t?
// ?does each TCPConnection can be a component?
#if __linux__
#include <cstdint>
#endif
typedef uint8_t SubscriberID;

struct Subscriber
{
    SubscriberID id;
};
