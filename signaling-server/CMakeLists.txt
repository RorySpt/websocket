

add_executable(websocket main.cpp
        signaling_server.cpp
        signaling_server.h
)

target_link_libraries(websocket PRIVATE hv_static bimap)