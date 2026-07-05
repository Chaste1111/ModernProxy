#include "upstream.h"
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <iostream>

UpstreamConn::UpstreamConn(const std::string& host, int port) {
    std::string port_str = std::to_string(port);
    addrinfo hints{};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    addrinfo* result = nullptr;
    int rc = getaddrinfo(host.c_str(), port_str.c_str(), &hints, &result);
    if (rc != 0) {
        std::cerr << "getaddrinfo failed: " << host << ":" << port
                  << " (" << gai_strerror(rc) << ")" << std::endl;
        fd_ = -1;
        return;
    }

    // 遍历结果，连第一个可达的地址
    for (addrinfo* p = result; p; p = p->ai_next) {
        fd_ = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (fd_ < 0) continue;

        fcntl(fd_, F_SETFL, fcntl(fd_, F_GETFL, 0) | O_NONBLOCK);

        if (connect(fd_, p->ai_addr, p->ai_addrlen) < 0) {
            if (errno == EINPROGRESS) {
                connecting_ = true;
                std::cout << "[UPSTREAM] connecting " << host
                          << ":" << port << " fd=" << fd_ << std::endl;
                freeaddrinfo(result);
                return;
            }
            close(fd_);
            fd_ = -1;
            continue;  // 换下一个地址
        }

        // 同步连上
        std::cout << "[UPSTREAM] connected " << host
                  << ":" << port << " fd=" << fd_ << std::endl;
        freeaddrinfo(result);
        return;
    }

    // 所有地址都失败
    std::cerr << "connect failed: " << host << ":" << port << std::endl;
    freeaddrinfo(result);
}

UpstreamConn::~UpstreamConn() {
    if (fd_ >= 0) close(fd_);
}

int UpstreamConn::release() {
    int fd = fd_;
    fd_ = -1;
    return fd;
}
