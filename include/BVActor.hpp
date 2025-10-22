#pragma once
#include "BV.hpp"
#include <iostream>

class BVActor
{
private:

public:
    BVActor();
    ~BVActor();

    BVStatus SendMessage(const std::string&);
    BVStatus Disconnect(void);

    // GetServices()?

};