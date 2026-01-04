#pragma once
#include "BVEvent.hpp"
#include <any>

// template<typename T>

// TODO: maybe typedef this std::shared_ptr<threadsafe_queue<BVMessage >>

struct BVMessage
{
    BVEventType event_t;
    std::unique_ptr<std::any> data_p;

    // Make the type move-only, to prohibit copying the message
    BVMessage(BVMessage&&) = default;
    BVMessage& operator=(BVMessage&&) = default;

    BVMessage(BVMessage &) = delete;
    BVMessage& operator=(BVMessage &) = delete;
};