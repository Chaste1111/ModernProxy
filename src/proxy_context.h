#pragma once
#include <string>
#include "http_parser.h"
#include "http_request.h"

class ProxyContext {
public:
    enum State { CONNECTING, SENDING, RECEIVING, DONE };

    ProxyContext(int client_fd);
    ~ProxyContext();

    ProxyContext(const ProxyContext&) = delete;
    ProxyContext& operator=(const ProxyContext&) = delete;

    int client_fd() const { return client_fd_; }
    int upstream_fd() const { return upstream_fd_; }
    void set_upstream_fd(int fd) { upstream_fd_ = fd; }
    State state() const { return state_; }
    void set_state(State s) { state_ = s; }

    // per-connection 状态
    std::string client_buf;   // 累积 client 端请求数据
    std::string upstream_req; // 重建后发往上游的请求
    HttpParser parser;
    HttpRequest req;
    bool parsed = false;      // Host 头是否已解析完成

private:
    int client_fd_;
    int upstream_fd_ = -1;
    State state_ = CONNECTING;
};
