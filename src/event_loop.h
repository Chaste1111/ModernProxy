#pragma once
#include <sys/epoll.h>
#include <functional>

class Acceptor;
class ReadManager;

class EventLoop {
public:
    using Callback = std::function<void(int)>;
    EventLoop();
    ~EventLoop();

    EventLoop(const EventLoop&) = delete;
    EventLoop& operator=(const EventLoop&) = delete;

    void add_fd(int fd, uint32_t events);
    void mod_fd(int fd, uint32_t events);
    void del_fd(int fd);
    void run(int listen_fd);

private:
    int epoll_fd_;
};
