#pragma once
#include<sys/epoll.h>
#include<functional>
#include "acceptor.h"
#include "readManager.h"
class EventLoop
 {
    public:
        using Callback=std::function<void(int)>;
        EventLoop();
        ~EventLoop();

        EventLoop(const EventLoop&)=delete;
        EventLoop& operator=(const EventLoop&)=delete;

        void set_on_accept(Callback cb)
        {
            on_accept_=std::move(cb);
        };
        void set_on_read(Callback cb)
        {
            on_read_=std::move(cb);
        };

        void add_fd(int fd, uint32_t events);
        void del_fd(int fd);
        void run(int listen_fd);

    private:
        int epoll_fd_;
        Callback on_accept_;
        Callback on_read_;
        Acceptor* acceptor_=nullptr;
        ReadManager* read_manager_=nullptr;

};