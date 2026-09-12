#include <iostream>
#include <thread>
#include <cstdint>
#include "UDP.hpp"
#include "AES.hpp"
#include "Jitter.hpp"
#include "Opus.hpp"
#include "UART.hpp"
#include "config.hpp"
#include <cstring>
#include <cstdio>


class Receiver
{
public:
    void receiveTask();
    void audioTask();

    void start();

private:
    std::thread udpThread;
    std::thread audioThread;
    UDP udp;
    AES aes;
    JitterBuffer jitterBuffer;
    Opus opus;
    UART uart;
};