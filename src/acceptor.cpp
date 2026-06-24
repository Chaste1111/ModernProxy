#include"acceptor.h"
#include<sys/socket.h>
#include<iostream>  
#include<unistd.h>
#include<fcntl.h>
#include<sys/epoll.h>


Acceptor::Acceptor(int listen_fd,int epoll_fd,Acceptor::Callback cb) :
    listen_fd_(listen_fd),
    epoll_fd_(epoll_fd),
    cb_(std::move(cb))
{
    
}
void Acceptor::handle_accept()
{
    int c=accept(listen_fd_, nullptr, nullptr);
    if(c<0) 
    {
        std::cerr<<"accept error"<<std::endl;
        return;
    }
    fcntl(c,F_SETFL,fcntl(c,F_GETFL,0)|O_NONBLOCK);

    epoll_event ev;
    ev.data.fd=c;
    ev.events=EPOLLIN;
    epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, c, &ev);
    std::cout<<"[ACCEPT] new client fd="<<c<<std::endl;

    if(cb_)
    {
        cb_(c);
    }
}