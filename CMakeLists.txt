cmake_minimum_required(VERSION 4.0)
project(websocket)

set(CMAKE_CXX_STANDARD 20)

add_executable(websocket signaling-server/main.cpp)

find_package(libhv CONFIG REQUIRED)
target_link_libraries(websocket PRIVATE hv)