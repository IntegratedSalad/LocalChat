#pragma once
#include "BV.hpp"
#include <iostream>

// Does BVActor is for every connection?
// Or it is just a connection handler between
// different hosts
// BVActorManager?
// Could be named BVSession

// Redundant?

class BVActor
{
private:
// hash map? with message history
// vector of blocked services?

public:
    BVActor();
    ~BVActor();

    // BVStatus StartConnection(void); // endpoint?
    // BVStatus TerminateConnection(void); // endpoint?
    BVStatus SendMessage(const std::string&);
    BVStatus TerminateSelf(void);

};