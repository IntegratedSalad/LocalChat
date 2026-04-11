#pragma once
#include <string>
#include <iostream>
#include <boost/asio.hpp>
#include "BV.hpp"
#include "BVLoggable.hpp"

struct BVUser
{
    std::string name;
    BVHost hostData;
};


struct BVTCPConnection
{
    BVHost host;
    bool alive;
};

// Remember to open a different file to log the connections logs.
class BVTCPConnectionManager : public BVLoggable
{

};
