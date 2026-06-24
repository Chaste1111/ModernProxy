#include"event_loop.h"
#include<unistd.h>
#include<iostream>
#include<fcntl.h>
#include <sys/socket.h>
#include <cstring>
#include "acceptor.h"
EventLoop::EventLoop()
{
    epoll_fd_=epoll_create1(0);
    if(epoll_fd_<0)
    {
        std::cerr<<"Failed to create epoll instance"<<std::endl;
        exit(1);
    }
}

EventLoop::~EventLoop()
{
    if(epoll_fd_>=0)
    {
        close(epoll_fd_);
    }
}

void EventLoop::add_fd(int fd, uint32_t events) {
    epoll_event ev{};
    ev.data.fd=fd;
    ev.events=events;
    epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev);
}

void EventLoop::del_fd(int fd) {
    epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr);
}

void EventLoop::run(int listen_fd)
{
    Acceptor acceptor(listen_fd,epoll_fd_,on_accept_);
    acceptor_=&acceptor;
    ReadManager read_manager(*this);
    read_manager_=&read_manager;
    
    epoll_event ev;
    ev.data.fd=listen_fd;
    ev.events=EPOLLIN;
    epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, listen_fd, &ev);
    epoll_event events[64];
    char buf[4096];
    while(true)
    {
        int n=epoll_wait(epoll_fd_,events,64,-1);
        for(int i=0;i<n;i++)
        {
            int fd=events[i].data.fd;
            if(fd==listen_fd)
            {
                //accept+注册
                acceptor_->handle_accept();
               
            }
            else if(events[i].events & EPOLLIN)
            {
                read_manager_->handle_read(fd);
            }
            else if(events[i].events & EPOLLOUT)
            {
                read_manager_->handle_write(fd);
            }
        }
    }
}
