#include "UART.hpp"
#include <cstdio>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
static int uartFd = -1;

uint16_t crc16(
    const uint8_t* data,
    uint16_t len
)
{
    uint16_t crc = 0xFFFF;

    for (uint16_t i = 0; i < len; i++)
    {
        crc ^= data[i];

        for (uint8_t j = 0; j < 8; j++)
        {
            if (crc & 0x0001)
            {
                crc >>= 1;
                crc ^= 0xA001;
            }
            else
            {
                crc >>= 1;
            }
        }
    }

    return crc;
}

bool UART::init()
{
    uartFd = open(
        UART_DEVICE,
        O_RDWR | O_NOCTTY
    );

    if (uartFd < 0)
    {
        perror("UART open failed");
        return false;
    }

    struct termios tty{};

    if (tcgetattr(
            uartFd,
            &tty
        ) != 0)
    {
        perror("tcgetattr failed");

        close(uartFd);

        uartFd = -1;

        return false;
    }


    cfsetispeed(
        &tty,
        B2000000
    );

    cfsetospeed(
        &tty,
        B2000000
    );


    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;

    tty.c_cflag &= ~PARENB;

    tty.c_cflag &= ~CSTOPB;

    tty.c_cflag &= ~CRTSCTS;

    tty.c_cflag |= CREAD;

    tty.c_cflag |= CLOCAL;


    tty.c_lflag &= ~ICANON;
    tty.c_lflag &= ~ECHO;
    tty.c_lflag &= ~ECHOE;
    tty.c_lflag &= ~ISIG;


    tty.c_iflag &= ~(
        IXON |
        IXOFF |
        IXANY
    );


    tty.c_oflag &= ~OPOST;


    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 1;


    if (tcsetattr(
            uartFd,
            TCSANOW,
            &tty
        ) != 0)
    {
        perror("tcsetattr failed");

        close(uartFd);

        uartFd = -1;

        return false;
    }


    printf(
        "UART initialized: %s\n",
        "Connected!!!"
    );

    return true;
}


int UART::uartSend(
    uint8_t type,
    uint32_t seq,
    const uint8_t* data,
    uint16_t len
)
{
    if (uartFd < 0)
    {
        return -1;
    }

    if (data == nullptr && len > 0)
    {
        return -1;
    }

    // Full frame:
    
    // [SYNC1][SYNC2][TYPE][SEQ_L][SEQ_H]
    // [LEN_L][LEN_H][PAYLOAD][CRC_L][CRC_H]

    // const uint16_t frameLen =
    //     FRAME_HEADER_SIZE +
    //     len +
    //     FRAME_CRC_SIZE;

    // uint8_t frame[FRAME_HEADER_SIZE + 8192 + FRAME_CRC_SIZE];

    // // --------------------------------------------------
    // // 1. SYNC
    // // --------------------------------------------------

    // frame[0] = FRAME_SYNC_1;
    // frame[1] = FRAME_SYNC_2;

    // // --------------------------------------------------
    // // 2. TYPE
    // // --------------------------------------------------

    // frame[2] = type;

    // // --------------------------------------------------
    // // 3. SEQ
    // // Little Endian
    // // --------------------------------------------------

    // frame[3] = seq & 0xFF;
    // frame[4] = (seq >> 8) & 0xFF;

    // // --------------------------------------------------
    // // 4. LEN
    // // Little Endian
    // // --------------------------------------------------

    // frame[5] = len & 0xFF;
    // frame[6] = (len >> 8) & 0xFF;

    // // --------------------------------------------------
    // // 5. PAYLOAD
    // // --------------------------------------------------

    // if (len > 0)
    // {
    //     memcpy(
    //         &frame[7],
    //         data,
    //         len
    //     );
    // }

    // // --------------------------------------------------
    // // 6. CRC16
    // // CRC tính từ TYPE đến hết PAYLOAD
    // // Không tính SYNC
    // // --------------------------------------------------

    // uint16_t crc = crc16(
    //     &frame[2],
    //     5 + len
    // );

    // // --------------------------------------------------
    // // 7. Append CRC
    // // --------------------------------------------------

    // frame[7 + len] =
    //     crc & 0xFF;

    // frame[8 + len] =
    //     (crc >> 8) & 0xFF;

    // // --------------------------------------------------
    // // 8. Send frame
    // // --------------------------------------------------

    // return write(
    //     uartFd,
    //     frame,
    //     frameLen
    // );


// Full frame:
// [SYNC1][SYNC2][TYPE][SEQ_L][SEQ_ML][SEQ_MH][SEQ_H]
// [LEN_L][LEN_H][PAYLOAD][CRC_L][CRC_H]

const uint16_t frameLen =
    FRAME_HEADER_SIZE +
    len +
    FRAME_CRC_SIZE;

uint8_t frame[FRAME_HEADER_SIZE + 8192 + FRAME_CRC_SIZE];

// --------------------------------------------------
// 1. SYNC
// --------------------------------------------------

frame[0] = FRAME_SYNC_1;
frame[1] = FRAME_SYNC_2;

// --------------------------------------------------
// 2. TYPE
// --------------------------------------------------

frame[2] = type;

// --------------------------------------------------
// 3. SEQ - 32 bit
// Little Endian
// --------------------------------------------------

frame[3] = seq & 0xFF;
frame[4] = (seq >> 8) & 0xFF;
frame[5] = (seq >> 16) & 0xFF;
frame[6] = (seq >> 24) & 0xFF;

// --------------------------------------------------
// 4. LEN - 16 bit
// Little Endian
// --------------------------------------------------

frame[7] = len & 0xFF;
frame[8] = (len >> 8) & 0xFF;

// --------------------------------------------------
// 5. PAYLOAD
// --------------------------------------------------

if (len > 0)
{
    memcpy(
        &frame[9],
        data,
        len
    );
}

// --------------------------------------------------
// 6. CRC16
// CRC tính từ TYPE đến hết PAYLOAD
// Không tính SYNC
// --------------------------------------------------

uint16_t crc = crc16(
    &frame[2],
    7 + len
);

// --------------------------------------------------
// 7. Append CRC
// --------------------------------------------------

frame[9 + len] =
    crc & 0xFF;

frame[10 + len] =
    (crc >> 8) & 0xFF;

// --------------------------------------------------
// 8. Send frame
// --------------------------------------------------

return write(
    uartFd,
    frame,
    frameLen
);


}