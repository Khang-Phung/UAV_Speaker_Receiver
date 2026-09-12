#pragma once
#include <cstddef>
#include <cstdint>
#include "config.hpp"
#include <cstring>

class UART 
{
public: 
bool init();

int uartSend(
    uint8_t type,
    uint32_t seq,
    const uint8_t* data,
    uint16_t len
);
private:

};