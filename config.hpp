#pragma once
#include <cstdint>


// =====================
// Serial / UART
// =====================
constexpr int SERIAL_BAUD = 2000000;
// Raspberry Pi UART device
constexpr const char* UART_DEVICE = "/dev/ttyACM0";
#define FRAME_SYNC_1   0xAA
#define FRAME_SYNC_2   0x55
constexpr uint8_t FRAME_TYPE_AUDIO     = 0x01;
//#define FRAME_HEADER_SIZE 7
#define FRAME_HEADER_SIZE 9

#define FRAME_CRC_SIZE    2
// =====================
// UDP
// =====================
constexpr int UDP_PORT = 5000;

constexpr int TCP_PORT = 5001;

constexpr int UDP_BUFFER_SIZE = 1500;

// =====================
// Jitter Buffer
// =====================
constexpr int JB_SIZE = 35;

// 40 ms
constexpr int JB_TIMEOUT_US = 40000;

constexpr int JB_MAX_PACKET_SIZE = 1024;

// 20 × 20 ms = 400 ms
constexpr int JB_THRESHOLD = 20;

// =====================
// AES-GCM
// =====================
extern const uint8_t AES_KEY[32];

constexpr int AES_KEY_SIZE = 32;
constexpr int AES_NONCE_SIZE = 12;
constexpr int AES_TAG_SIZE = 16;

// =====================
// Opus
// =====================
constexpr int SAMPLE_RATE = 48000;
constexpr int CHANNELS = 1;
constexpr int FRAME_SIZE = 960;


