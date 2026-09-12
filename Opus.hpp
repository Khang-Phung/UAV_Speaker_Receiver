#pragma once
#include <cstdint>
#include "config.hpp"
#include <cstddef>
class Opus 
{
public: 
bool init();

int decode(const uint8_t* data,int len);

int16_t* getBuffer();

void low_volume();

size_t getBufferSize();

private:
    uint8_t data[UDP_BUFFER_SIZE];
    int16_t _pcmBuffer[FRAME_SIZE];

};