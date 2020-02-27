cmake_minimum_required(VERSION 3.12)

set (CMAKE_CXX_STANDARD 11)

include_directories(.)

set(udptx_SRC udptx.cpp common/Socket.cpp common/Timer.cpp)
set(udptx_INC common/Socket.h common/Timer.h common/TSCTimer.h)

add_executable(udptx ${udptx_SRC} ${udptx_INC})

#target_link_libraries(udptx efu_common)
set_target_properties(udptx PROPERTIES
  COMPILE_FLAGS "-fno-strict-aliasing")

set(udprx_SRC udprx.cpp common/Socket.cpp common/Timer.cpp)
set(udprx_INC common/Socket.h common/Timer.h common/TSCTimer.h)

add_executable(udprx ${udprx_SRC} ${udprx_INC})
#target_link_libraries(udprx efu_common)

install(TARGETS udptx udprx DESTINATION bin)
