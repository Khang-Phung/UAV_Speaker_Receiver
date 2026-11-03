#pragma once
#include <cstddef>
#include <cstdint>
#include "config.hpp"
#include <cstring>
#include <chrono>

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

bool isConnected() const;

bool checkConnection();

private:
bool _connect = false;

    std::chrono::steady_clock::time_point last_uart_retry =
        std::chrono::steady_clock::now();
};