#pragma once
#include "config.hpp"
#include <cstdint>

class UDP 
{
public: 
    bool init(uint16_t port);
    
    int receive(int maxLen);

    uint8_t* getBuffer();

    void setAlive();
    
    bool isAlive();


private:
    uint8_t _rxBuffer[UDP_BUFFER_SIZE];
    bool _alive;
};