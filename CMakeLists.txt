cmake_minimum_required(VERSION 3.16)

project(UAV_Speaker_Receiver)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(receiver
    main.cpp
    Receiver.cpp
    UDP.cpp
    AES.cpp
    Opus.cpp
    Jitter.cpp
    UART.cpp
    config.cpp
    TCP.cpp
)

target_link_libraries(receiver
    crypto
    opus
    pthread
)