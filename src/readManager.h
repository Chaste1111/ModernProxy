#pragma once
#include "proxy_context.h"
#include <unordered_map>

class EventLoop;

class ReadManager {
public:
    explicit ReadManager(EventLoop& loop);
    void handle_read(int fd);
    void handle_write(int fd);

private:
    void on_client_read(int fd, ProxyContext* ctx);
    void on_upstream_read(int fd, ProxyContext* ctx);
    void on_upstream_write(int fd, ProxyContext* ctx);
    void cleanup(ProxyContext* ctx);

    EventLoop& loop_;
    std::unordered_map<int, ProxyContext*> contexts_;
};
