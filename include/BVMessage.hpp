#pragma once
#include "BVEvent.hpp"
#include <any>

// template<typename T>
struct BVMessage
{
    BVEventType event_t;
    std::unique_ptr<std::any> data_p;
};