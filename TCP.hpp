#pragma once

#include "config.hpp"
#include <cstdint>

class TCP
{
public:
    bool init(uint16_t port);

    bool acceptClient();

    int receive(int maxLen);

    bool send(
        const uint8_t* data,
        int len
    );

    bool isConnected() const;

    void close();

    const uint8_t* getBuffer() const;

private:
    int serverSock = -1;
    int clientSock = -1;
    uint8_t rxBuffer[512]{};
};