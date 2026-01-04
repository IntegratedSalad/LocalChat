#pragma once

// why uint8_t?
// ?does each TCPConnection can be a component?
typedef uint8_t SubscriberID;

struct Subscriber
{
    SubscriberID id;
};
