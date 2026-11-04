cmake_minimum_required(VERSION 3.16)

project(UAV_Speaker_Receiver LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Dependencies
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(OPUS REQUIRED IMPORTED_TARGET opus)

# Source files
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

# Link libraries
target_link_libraries(receiver PRIVATE
    OpenSSL::Crypto
    PkgConfig::OPUS
    Threads::Threads
)

# Compiler warnings
target_compile_options(receiver PRIVATE
    -Wall
    -Wextra
)