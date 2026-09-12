#include "UDP.hpp"
#include <cstdio>
#include <cerrno>

#include <unistd.h>

#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>

static int sock = -1;


bool UDP::init(uint16_t port)
{
    // Create UDP socket
    sock = socket(
        AF_INET, // IPv4
        SOCK_DGRAM,//UDP
        0
    );

    if (sock < 0)
    {
        perror("socket");
        return false;
    }

    // Increase receive buffer
    int rcvbuf = 16 * 1024; // 32 KB

    if (setsockopt(
            sock,
            SOL_SOCKET,
            SO_RCVBUF,
            &rcvbuf,
            sizeof(rcvbuf)) < 0)
    {
        perror("setsockopt SO_RCVBUF");
    }

    // Check actual receive buffer
    socklen_t optlen = sizeof(rcvbuf);

    if (getsockopt(
            sock,
            SOL_SOCKET,
            SO_RCVBUF,
            &rcvbuf,
            &optlen) == 0)
    {
        // printf("SO_RCVBUF = %d\n", rcvbuf);
    }

    // Receive timeout: 10 ms
    struct timeval tv{};

    tv.tv_sec = 0;
    tv.tv_usec = 10000;

    if (setsockopt(
            sock,
            SOL_SOCKET,
            SO_RCVTIMEO,
            &tv,
            sizeof(tv)) < 0)
    {
        perror("setsockopt SO_RCVTIMEO");
    }

    // Bind address
    sockaddr_in addr{};

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(
            sock,
            reinterpret_cast<sockaddr*>(&addr),
            sizeof(addr)) < 0)
    {
        perror("bind");

        close(sock);
        sock = -1;

        return false;
    }

    printf("UDP port listen: %d\n", port);

    return true;
}


int UDP::receive(
    int maxLen)
{
    if (sock < 0)
        return -1;

    sockaddr_in source{};

    socklen_t len = sizeof(source);

    int received = recvfrom(
        sock,
        _rxBuffer,
        maxLen,
        0,
        reinterpret_cast<sockaddr*>(&source),
        &len
    );

    if (received < 0)
    {
        // Timeout
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            return 0;
        }

        perror("recvfrom");
        return -1;
    }

    return received;
}

uint8_t* UDP::getBuffer()
{
    return _rxBuffer;
}