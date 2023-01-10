cmake_minimum_required(VERSION 3.12)

project(UDPPerf)

set (CMAKE_CXX_STANDARD 11)
include_directories(.)

set(udptx_SRC udptx.cpp common/Socket.cpp common/Timer.cpp)
set(udptx_INC common/Socket.h common/Timer.h common/TSCTimer.h)
add_executable(udptx ${udptx_SRC} ${udptx_INC})

set(udprx_SRC udprx.cpp common/Socket.cpp common/Timer.cpp)
set(udprx_INC common/Socket.h common/Timer.h common/TSCTimer.h)
add_executable(udprx ${udprx_SRC} ${udprx_INC})
