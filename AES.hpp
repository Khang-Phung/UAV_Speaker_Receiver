#pragma once
#include "config.hpp"
#include <cstdint>

class AES 
{
public: 
bool init();

bool decrypt(
    uint8_t* data,
    int len
);

uint8_t* getBuffer();
int getLen();



private:
    uint8_t _plainBuffer[JB_MAX_PACKET_SIZE];
    int plainLen;
};