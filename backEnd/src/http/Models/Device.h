#pragma once
#include <string>

class Device
{    
public:
    Device(/* args */);
    ~Device();

private:
    int id;
    std::string name;
    std::string location;
    
};
