#pragma once

class Acceptor {
public:
    Acceptor(int listen_fd, int epoll_fd);
    void handle_accept();

private:
    int listen_fd_;
    int epoll_fd_;
};
