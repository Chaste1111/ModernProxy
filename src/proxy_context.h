#pragma once
#include <string>

class ProxyContext {
public:
    enum State { CONNECTING, SENDING, RECEIVING, DONE };

    ProxyContext(int client_fd);
    ~ProxyContext();

    ProxyContext(const ProxyContext&)=delete;
    ProxyContext& operator=(const ProxyContext&)=delete;

    int client_fd() const { return client_fd_; }
    int upstream_fd() const { return upstream_fd_; }
    void set_upstream_fd(int fd) { upstream_fd_=fd; }
    State state() const { return state_; }
    void set_state(State s) { state_=s; }

    std::string send_buf;
    std::string recv_buf;

private:
    int client_fd_;
    int upstream_fd_=-1;
    State state_=CONNECTING;
};
