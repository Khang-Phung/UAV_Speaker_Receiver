#pragma once
#include "config.hpp"
#include <mutex>
#include <cstring>

struct JitterPacket
{
    uint32_t seq;
    int len;
    uint8_t data[JB_MAX_PACKET_SIZE];
};

class JitterBuffer
{
public:
    void init(int startPackets);

    bool push(
        const uint8_t* data,
        int len
    );

    int pop(
        JitterPacket* pkt
    );

private:
    JitterPacket _packetBuffer[JB_SIZE];

    int _startThreshold = 5;
    int _count = 0;

    bool _started = false;

    uint32_t _expectedSeq = 0;

    std::mutex jbMutex;
};