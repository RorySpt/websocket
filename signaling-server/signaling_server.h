//
// Created by zhang on 2025/11/20.
//

#ifndef WEBSOCKET_SIGNALING_SERVER_H
#define WEBSOCKET_SIGNALING_SERVER_H


#include <iostream>
#include <string>
#include <unordered_map>
#include <memory>
#include <atomic>
#include "hv/WebSocketServer.h"
#include "hv/EventLoop.h"
#include "hv/HttpServer.h"
#include "hv/json.hpp"

using namespace hv;
using json = nlohmann::json;

// 全局客户端映射

class SignalingServer {
public:
    SignalingServer(int port = 8000, const std::string& host = "127.0.0.1");

    bool start();

    void stop();

private:
    void onWebSocketOpen(const WebSocketChannelPtr& channel, const HttpRequestPtr& req);

    void onWebSocketMessage(const WebSocketChannelPtr& channel, const std::string& msg) const;

    void onWebSocketClose(const WebSocketChannelPtr& channel) const;

    std::string extractClientId(const std::string& path);

private:


    struct Impl;
    std::unique_ptr<Impl> impl_;
};


#endif //WEBSOCKET_SIGNALING_SERVER_H