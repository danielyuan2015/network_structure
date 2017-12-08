cmake_minimum_required(VERSION 3.4)
project(TCPSERVERPRJ)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_subdirectory(src/App)
add_subdirectory(src/Network)
add_subdirectory(src/Logging)
add_subdirectory(src/OSAL)
add_subdirectory(src/Tools)