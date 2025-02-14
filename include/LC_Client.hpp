#pragma once
#include <iostream>
#include <string>

/*
    LC_Client functions as a console application of LocalChat.
    Define as a function object, because it should be run in a different thread?
*/
class LC_Client
{
private:

public:
    LC_Client();

    void Init();

    ~LC_Client();

};
