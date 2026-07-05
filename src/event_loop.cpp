#include "event_loop.h"
#include "acceptor.h"
#include "readManager.h"
#include <unistd.h>
#include <iostream>
#include <cerrno>
#include <csignal>

extern volatile sig_atomic_t g_running;

EventLoop::EventLoop() {
    epoll_fd_ = epoll_create1(0);
    if (epoll_fd_ < 0) {
        std::cerr << "Failed to create epoll instance" << std::endl;
        exit(1);
    }
}

EventLoop::~EventLoop() {
    if (epoll_fd_ >= 0) close(epoll_fd_);
}

void EventLoop::add_fd(int fd, uint32_t events) {
    epoll_event ev{};
    ev.data.fd = fd;
    ev.events = events;
    epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev);
}

void EventLoop::del_fd(int fd) {
    epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr);
}

void EventLoop::mod_fd(int fd, uint32_t events) {
    epoll_event ev{};
    ev.data.fd = fd;
    ev.events = events;
    epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &ev);
}

void EventLoop::run(int listen_fd) {
    Acceptor acceptor(listen_fd, epoll_fd_);
    ReadManager read_manager(*this);

    epoll_event ev{};
    ev.data.fd = listen_fd;
    ev.events = EPOLLIN;
    epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, listen_fd, &ev);

    epoll_event events[64];
    while (g_running) {
        int n = epoll_wait(epoll_fd_, events, 64, 1000);
        if (n < 0) {
            if (errno == EINTR) break;
            continue;
        }
        for (int i = 0; i < n; i++) {
            int fd = events[i].data.fd;
            if (fd == listen_fd) {
                acceptor.handle_accept();
            } else if (events[i].events & EPOLLIN) {
                read_manager.handle_read(fd);
            } else if (events[i].events & EPOLLOUT) {
                read_manager.handle_write(fd);
            }
        }
    }

    std::cout << "[STOP] event loop" << std::endl;
}
