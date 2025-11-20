cmake_minimum_required(VERSION 4.0)
project(websocket)

set(CMAKE_CXX_STANDARD 23)

add_subdirectory(deps)

add_subdirectory(signaling-server)
