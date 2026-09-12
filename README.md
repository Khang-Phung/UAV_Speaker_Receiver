# UAV Speaker Receiver

Chương trình nhận và xử lý dữ liệu âm thanh cho hệ thống **UAV Speaker** trên nền tảng Linux.

Receiver nhận các gói âm thanh được mã hóa qua UDP, giải mã AES-GCM, sắp xếp lại thứ tự gói bằng Jitter Buffer, giải mã Opus thành PCM và đưa dữ liệu âm thanh đến ngõ ra.

## Chức năng

* Nhận dữ liệu âm thanh qua UDP
* Giải mã AES-GCM
* Kiểm tra và quản lý sequence number
* Jitter Buffer để xử lý packet reorder và network jitter
* Giải mã âm thanh Opus
* Xử lý dữ liệu PCM
* Giao tiếp UART
* Xử lý đa luồng

## Kiến trúc hệ thống

                    Network
                       │
                       ▼
                ┌─────────────┐
                │     UDP     │
                │   Receive   │
                └──────┬──────┘
                       │
                       ▼
                ┌─────────────┐
                │     AES     │
                │   Decrypt   │
                └──────┬──────┘
                       │
                       ▼
                ┌─────────────┐
                │    Jitter   │
                │    Buffer   │
                └──────┬──────┘
                       │
                       ▼
                ┌─────────────┐
                │    Opus     │
                │    Decode   │
                └──────┬──────┘
                       │
                       ▼
                    PCM Audio
                       │
                       ▼
                      UART

## Cấu trúc project

UAV_Speaker_Receiver/
├── main.cpp
│
├── Receiver.cpp
├── Receiver.hpp
│
├── UDP.cpp
├── UDP.hpp
│
├── AES.cpp
├── AES.hpp
│
├── Opus.cpp
├── Opus.hpp
│
├── Jitter.cpp
├── Jitter.hpp
│
├── UART.cpp
├── UART.hpp
│
├── config.cpp
├── config.hpp
│
├── CMakeLists.txt
├── README.md
└── build/

## Các thành phần chính

### Receiver

`Receiver` là class điều khiển chính của chương trình.

Nhiệm vụ:

* Khởi tạo các module
* Tạo các thread xử lý
* Điều phối quá trình nhận và xử lý audio

Luồng chính:

Receiver
 ├── UDP
 ├── AES
 ├── JitterBuffer
 ├── Opus
 └── UART
```

Trong `Receiver::start()`:

                 Receiver::start()
                        │
             ┌──────────┴──────────┐
             │                     │
             ▼                     ▼
       UDP Thread             Audio Thread
     receiveTask()             audioTask()

### UDP

Chịu trách nhiệm nhận dữ liệu từ network.

Các chức năng chính:

* Tạo UDP socket
* Bind vào UDP port
* Nhận packet
* Chuyển dữ liệu nhận được cho các module xử lý tiếp theo

### AES

Chịu trách nhiệm giải mã dữ liệu được mã hóa bằng **AES-GCM**.

Khóa AES được khai báo trong:

config.hpp

và định nghĩa trong:

config.cpp

### JitterBuffer

Lưu trữ tạm thời các packet nhận được và sắp xếp chúng theo sequence number trước khi đưa đến Opus decoder.

Jitter Buffer giúp xử lý:

* Packet đến không đúng thứ tự
* Network jitter
* Packet bị mất
* Seq bị reset
* Seq lớn hơn Seq mong muốn nhiều

Jitter Buffer được truy cập bởi nhiều thread nên sử dụng `std::mutex` để bảo vệ dữ liệu.

### Opus

Chịu trách nhiệm giải mã dữ liệu âm thanh Opus thành PCM.

Cấu hình audio hiện tại:

Sample rate : 48000 Hz
Channels    : Mono
Frame size  : 960 samples
Frame time  : 20 ms

### UART

Chịu trách nhiệm giao tiếp UART với phần cứng bên ngoài.

### Config

Chứa các thông số cấu hình chung của project.

Ví dụ:

UDP_PORT
JB_SIZE
JB_THRESHOLD
SAMPLE_RATE
CHANNELS
FRAME_SIZE
SERIAL_BAUD
AES_KEY

## Luồng xử lý dữ liệu

Một packet audio đi qua các bước:

Sender
   │
   │ Opus Encode
   ▼
AES-GCM Encrypt
   │
   │ UDP
   ▼
Receiver
   │
   ▼
UDP Receive
   │
   ▼
AES-GCM Decrypt
   │
   ▼
Jitter Buffer
   │
   ▼
Opus Decode
   │
   ▼
PCM
   │
   ▼
UART

## Sequence Number

Mỗi packet có sequence number để receiver xác định thứ tự packet.

Ví dụ:

100
101
103
102

Jitter Buffer sẽ sắp xếp lại:

100
101
102
103

Nếu packet bị mất:

100
101
103

receiver có thể phát hiện packet `102` bị mất và sử dụng **Opus PLC (Packet Loss Concealment)** để tạo dữ liệu âm thanh thay thế.

Nếu packet mới cách xa so với packet mong muốn hiện tại:

packet hiện tại 100
packket mới 140

Reset buffer


## Đa luồng

Receiver sử dụng hai thread chính:

### UDP Thread

receiveTask()

Chịu trách nhiệm:

UDP Receive
    ↓
AES Decrypt
    ↓
Jitter Push

### Audio Thread

audioTask()

Chịu trách nhiệm:

Jitter Pop
    ↓
Opus Decode
    ↓
PCM Processing
    ↓
UART
Hai thread giao tiếp với nhau thông qua `JitterBuffer`.

## Yêu cầu

### Hệ điều hành

* Ubuntu / Linux

### Thư viện

* CMake
* Ninja
* OpenSSL
* libopus
* POSIX Threads

Cài đặt trên Ubuntu:

sudo apt update
sudo apt install cmake ninja-build clang libssl-dev libopus-dev

## Build

Di chuyển vào thư mục project:

cd ~/CT_UAV/Project_Speaker/UAV_Speaker_Receiver

Tạo build directory:

cmake -S . -B build -G Ninja

Build:

cmake --build build

File executable sau khi build:

build/receiver

## Chạy

./build/receiver

Khi khởi động, chương trình sẽ hiển thị các địa chỉ IP:

    UAV Speaker Receiver
---------------------------
IP Address (Sender chọn IP VPN):
- 100.x.x.x
---------------------------
WAITING FOR SENDER START

Sender sử dụng IP VPN tương ứng để gửi audio đến receiver.

## Cấu hình

Các thông số của hệ thống được đặt trong:

config.hpp
config.cpp

## Trạng thái

* [x] UDP Receiver
* [x] AES-GCM Decryption
* [x] Jitter Buffer
* [x] Opus Decoder
* [x] Multithreading
* [x] UART


## Mục đích

Project được phát triển cho hệ thống **UAV Speaker**, phục vụ việc truyền và phát âm thanh từ máy gửi đến thiết bị loa trên UAV thông qua mạng IP.
