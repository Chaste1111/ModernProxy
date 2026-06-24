#include "tcp_listener.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>

TcpListener::TcpListener(int port) {
    fd_=socket(AF_INET, SOCK_STREAM, 0);
    if(fd_<0)
    {
        std::cerr<<"socket() failed"<<std::endl;
        return;
    }

    int opt=1;
    setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr=INADDR_ANY;
    addr.sin_port=htons(port);

    if(bind(fd_, (sockaddr*)&addr, sizeof(addr))<0)
    {
        std::cerr<<"bind() failed"<<std::endl;
        close(fd_);
        fd_=-1;
        return;
    }

    if(listen(fd_, 128)<0)
    {
        std::cerr<<"listen() failed"<<std::endl;
        close(fd_);
        fd_=-1;
        return;
    }

    std::cout<<"[OK] listening on port "<<port<<std::endl;
}

TcpListener::~TcpListener() {
    if(fd_>=0)
    {
        close(fd_);
    }
}

int TcpListener::accept() const {
    return ::accept(fd_, nullptr, nullptr);
}
