#include "upstream.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <iostream>

UpstreamConn::UpstreamConn(const std::string& host, int port) {
    fd_=socket(AF_INET, SOCK_STREAM, 0);
    if(fd_<0)
    {
        std::cerr<<"upstream socket() failed"<<std::endl;
        return;
    }

    fcntl(fd_, F_SETFL, fcntl(fd_, F_GETFL, 0) | O_NONBLOCK);

    hostent* he=gethostbyname(host.c_str());
    if(!he)
    {
        std::cerr<<"gethostbyname failed: "<<host<<std::endl;
        close(fd_);
        fd_=-1;
        return;
    }

    sockaddr_in addr{};
    addr.sin_family=AF_INET;
    addr.sin_port=htons(port);
    std::memcpy(&addr.sin_addr, he->h_addr, he->h_length);

    if(connect(fd_, (sockaddr*)&addr, sizeof(addr))<0)
    {
        if(errno==EINPROGRESS)
        {
            connecting_=true;
            std::cout<<"[UPSTREAM] connecting "<<host<<":"<<port<<" fd="<<fd_<<std::endl;
            return;
        }
        std::cerr<<"connect failed: "<<host<<":"<<port<<std::endl;
        close(fd_);
        fd_=-1;
        return;
    }

    std::cout<<"[UPSTREAM] connected "<<host<<":"<<port<<" fd="<<fd_<<std::endl;
}

UpstreamConn::~UpstreamConn() {
    if(fd_>=0)
    {
        close(fd_);
    }
}

int UpstreamConn::release() {
    int fd=fd_;
    fd_=-1;
    return fd;
}
