//
// Created by zhang on 2025/11/20.
//

#include "signaling_server.h"

#include <bimap.hpp>


template<typename T>
class synchronized:public T
{
    std::mutex mutex_;
public:

    template<typename Callable>
        requires std::is_invocable_v<Callable, T&>
    auto apply(Callable&& fn)
    {
        using return_t = decltype(fn(std::declval<T&>()));
        std::lock_guard<std::mutex> lock(mutex_);
        if constexpr (std::is_same_v<return_t, void>)
        {
            std::invoke(fn, *this);
        }else
        {
            return std::invoke_r<return_t>(fn, *this);
        }

    }


    void lock()
    {
        mutex_.lock();
    }
    void unlock()
    {
        mutex_.unlock();
    }

    auto lock_guard()
    {
        return std::lock_guard<std::mutex>(mutex_);
    }
};



struct SignalingServer::Impl
{

    int port_;
    std::string host_;
    WebSocketServer websocket_server_;
    WebSocketService websocket_service_;
    HttpService http_service_;

    using clients_t = stde::bimap<std::string, WebSocketChannelPtr>;
    synchronized<clients_t> clients_;
};
SignalingServer::SignalingServer(int port, const std::string& host)
    : impl_(std::make_unique<Impl>())
{
    impl_->port_ = port;
    impl_->host_ = host;
}

bool SignalingServer::start()
{
    auto& [port_,host_,websocket_server_, websocket_service_, http_service_, clients_] = *impl_;
    // 创建WebSocket服务
    websocket_service_.onopen = [this](const WebSocketChannelPtr& channel, const HttpRequestPtr& req) {
        this->onWebSocketOpen(channel, req);
    };

    websocket_service_.onmessage = [this](const WebSocketChannelPtr& channel, const std::string& msg) {
        this->onWebSocketMessage(channel, msg);
    };

    websocket_service_.onclose = [this](const WebSocketChannelPtr& channel) {
        this->onWebSocketClose(channel);
    };
    http_service_.GET("/ping", [](const HttpContextPtr& ctx) {
        return ctx->send("pong");
    });
    // 启动服务器
    websocket_server_.setHost(host_.c_str());
    websocket_server_.setPort(port_);
    websocket_server_.registerWebSocketService(&websocket_service_);
    websocket_server_.registerHttpService(&http_service_);

    std::cout << "Listening on " << host_ << ":" << port_ << std::endl;
    return websocket_server_.start() == 0;
}

void SignalingServer::stop()
{
    impl_->websocket_server_.stop();
}

void SignalingServer::onWebSocketOpen(const WebSocketChannelPtr& channel, const HttpRequestPtr& req)
{
    // 从URL路径提取客户端ID
    const std::string path = req->Path();
    const std::string client_id = extractClientId(path);

    if (!client_id.empty()) {
        impl_->clients_.apply([&](Impl::clients_t & clients)
            {
                clients.insert({client_id, channel});
            });
        std::cout << "Client " << client_id << " connected" << std::endl;

        // 存储客户端ID到channel的上下文
    }
}

void SignalingServer::onWebSocketMessage(const WebSocketChannelPtr& channel, const std::string& msg) const
{


    auto client_id = impl_->clients_.apply([&](const Impl::clients_t & clients)
        {
            return clients.has_value(channel)?std::optional(clients.get_key(channel)) : std::optional<std::string>{};
        });
    std::cout << "Client " << client_id.value_or("Unknow client") << " << " << msg << std::endl;

    if (!client_id) return;

    try {
        // 解析JSON消息
        auto message = json::parse(msg);
        std::string destination_id = message["id"];

        auto guard = impl_->clients_.lock_guard();
        if (impl_->clients_.has_key(destination_id)) {
            // 修改消息中的ID为发送者ID
            message["id"] = client_id.value();
            std::string response = message.dump();

            std::cout << "Client " << destination_id << " >> " << response << std::endl;

            // 发送消息到目标客户端
            auto destination_channel = impl_->clients_.get_value(destination_id);
            destination_channel->send(response);
        } else {
            std::cout << "Client " << destination_id << " not found" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
    }
}

void SignalingServer::onWebSocketClose(const WebSocketChannelPtr& channel) const
{
    auto guard = impl_->clients_.lock_guard();
    if (impl_->clients_.has_value(channel)) {
        const auto client_id = impl_->clients_.get_key(channel);
        impl_->clients_.erase_value(channel);
        std::cout << "Client " << client_id << " disconnected" << std::endl;
    }
}

std::string SignalingServer::extractClientId(const std::string& path)
{
    // 解析路径格式：/client_id
    if (path.empty() || path == "/") {
        return "";
    }
    // 移除开头的斜杠
    std::string clean_path = (path[0] == '/') ? path.substr(1) : path;

    // 提取第一个路径段作为客户端ID
    size_t pos = clean_path.find('/');
    if (pos != std::string::npos) {
        return clean_path.substr(0, pos);
    }

    return clean_path;
}
