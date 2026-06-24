#pragma once
#include<functional>

class Acceptor
{
   public:
    using Callback=std::function<void(int fd)>;

   Acceptor(int listen_fd,int epoll_fd,Callback cb);
    void handle_accept();

   private:
    int listen_fd_;
    int epoll_fd_;
    Callback cb_;
};