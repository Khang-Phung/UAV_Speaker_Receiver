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
#include "TCP.hpp"


class Receiver
{
public:
    void receiveTask();
    void audioTask();
    void tcpTask();
    void start();

private:
    std::thread udpThread;
    std::thread audioThread;
    std::thread tcpThread;
    UDP udp;
    AES aes;
    JitterBuffer jitterBuffer;
    Opus opus;
    UART uart;
    TCP tcp;
};