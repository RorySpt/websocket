#include <iostream>
#include <string>
#include <memory>

#include "signaling_server.h"


int main(int argc, char* argv[]) {
    // 解析命令行参数
    int port = 8000;
    std::string host = "0.0.0.0";

    if (argc > 1) {
        std::string endpoint = argv[1];
        size_t colon_pos = endpoint.find(':');
        if (colon_pos != std::string::npos) {
            host = endpoint.substr(0, colon_pos);
            port = std::stoi(endpoint.substr(colon_pos + 1));
        } else {
            port = std::stoi(endpoint);
        }
    }

    // 创建并启动服务器
    SignalingServer server(port, host);

    if (!server.start()) {
        std::cerr << "Failed to start server" << std::endl;
        return -1;
    }

    // 等待服务器运行
    while (getchar() != 'q') {
        // 按q退出
    }

    server.stop();
    return 0;
}
