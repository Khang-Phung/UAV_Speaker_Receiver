#include "TCP.hpp"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <cstdio>

bool TCP::init(uint16_t port)
{       

    serverSock = socket(
        AF_INET,
        SOCK_STREAM,
        0
    );
    
    if (serverSock < 0)
    {
        perror("TCP socket");
        return false;
    }

    int opt = 1;

    setsockopt(
        serverSock,
        SOL_SOCKET,
        SO_REUSEADDR,
        &opt,
        sizeof(opt)
    );

    sockaddr_in addr{};

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(
        serverSock,
        reinterpret_cast<sockaddr*>(&addr),
        sizeof(addr)
    ) < 0)
    {
        perror("TCP bind");
        close();
        return false;
    }

    if (listen(serverSock, 1) < 0)
    {
        perror("TCP listen");
        close();
        return false;
    }
    return true;
}


bool TCP::acceptClient()
{   
    if (serverSock < 0)
        return false;

    sockaddr_in clientAddr{};
    socklen_t addrLen = sizeof(clientAddr);

    int newSock = accept(
        serverSock,
        reinterpret_cast<sockaddr*>(&clientAddr),
        &addrLen
    );

    if (newSock < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return false;

        perror("TCP accept");
            printf("TCP accept");

        return false;
    }

    // Nếu đang có client cũ thì đóng nó
    if (clientSock >= 0)
    {
        ::close(clientSock);
    }

    clientSock = newSock;
    
    return true;
}


int TCP::receive(int maxLen)
{
    if (clientSock < 0)
        return -1;

    if (maxLen > static_cast<int>(sizeof(rxBuffer)))
        maxLen = sizeof(rxBuffer);

    int received = recv(
        clientSock,
        rxBuffer,
        maxLen,
        0
    );

    // Có data
    if (received > 0)
    {
        return received;
    }

    // Client thực sự đóng connection
    if (received == 0)
    {
        ::close(clientSock);
        clientSock = -1;

        return -1;
    }

    // Non-blocking: hiện tại chưa có data
    if (errno == EAGAIN ||
        errno == EWOULDBLOCK)
    {
        return 0;
    }

    perror("TCP recv");

    ::close(clientSock);
    clientSock = -1;

    return -1;
}


bool TCP::send(
    const uint8_t* data,
    int len
)
{
    if (clientSock < 0)
        return false;

    int sent = ::send(
        clientSock,
        data,
        len,
        MSG_NOSIGNAL
    );

    if (sent < 0)
    {
        perror("TCP send");

        ::close(clientSock);
        clientSock = -1;

        return false;
    }

    return sent == len;
}


bool TCP::isConnected() const
{
    return clientSock >= 0;
}


void TCP::close()
{
    if (clientSock >= 0)
    {
        ::close(clientSock);
        clientSock = -1;
    }

    if (serverSock >= 0)
    {
        ::close(serverSock);
        serverSock = -1;
    }
}

const uint8_t* TCP::getBuffer() const
{
    return rxBuffer;
}