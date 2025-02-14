#pragma once

class BVDiscovery
{
public:
    virtual void run() = 0;
    void operator()(void) const
    {
        this->run();
    }
};