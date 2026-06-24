#include "readManager.h"
#include "event_loop.h"
#include "upstream.h"
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <cstring>

ReadManager::ReadManager(EventLoop& loop) : loop_(loop) {}

void ReadManager::handle_read(int fd) {
    auto it=contexts_.find(fd);
    if(it!=contexts_.end())
    {
        on_upstream_read(fd, it->second);
        return;
    }
    on_client_read(fd);
}

void ReadManager::handle_write(int fd) {
    auto it=contexts_.find(fd);
    if(it!=contexts_.end())
    {
        on_upstream_write(fd, it->second);
    }
}

void ReadManager::on_client_read(int fd) {
    char buffer[4096];
    int r=read(fd, buffer, sizeof(buffer)-1);

    if(r<=0)
    {
        if(r==0) std::cout<<"[CLOSE] fd="<<fd<<std::endl;
        close(fd);
        return;
    }

    HttpRequest req;
    if(!parser_.parse(std::string_view(buffer, r), req))
    {
        std::cout<<"[INCOMPLETE] fd="<<fd<<std::endl;
        return;
    }

    std::cout<<"[PARSED] "<<req.method<<" "<<req.url
             <<" -> "<<req.host<<std::endl;

    ProxyContext* ctx=new ProxyContext(fd);
    ctx->set_state(ProxyContext::CONNECTING);

    ctx->send_buf=req.method+" "+req.url+" HTTP/1.1\r\n"
                 +"Host: "+req.host+"\r\n"
                 +"Connection: close\r\n\r\n";

    UpstreamConn upstream(req.host);
    if(!upstream.ok())
    {
        std::cerr<<"[UPSTREAM FAIL] "<<req.host<<std::endl;
        delete ctx;
        close(fd);
        return;
    }

    int up_fd=upstream.release();
    ctx->set_upstream_fd(up_fd);
    contexts_[up_fd]=ctx;

    if(upstream.connecting())
    {
        loop_.add_fd(up_fd, EPOLLOUT);
        std::cout<<"[STATE] fd="<<fd<<" CONNECTING (wait EPOLLOUT)"<<std::endl;
        return;
    }

    ctx->set_state(ProxyContext::SENDING);
    write(up_fd, ctx->send_buf.c_str(), ctx->send_buf.size());
    ctx->set_state(ProxyContext::RECEIVING);
    loop_.add_fd(up_fd, EPOLLIN);
    std::cout<<"[STATE] fd="<<fd<<" CONNECTING→SENDING→RECEIVING"<<std::endl;
}

void ReadManager::on_upstream_write(int fd, ProxyContext* ctx) {
    loop_.del_fd(fd);

    // 非阻塞 connect 完成 → 检查错误
    if(ctx->state()==ProxyContext::CONNECTING)
    {
        int err=0;
        socklen_t len=sizeof(err);
        getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &len);
        if(err)
        {
            std::cerr<<"[UPSTREAM] connect error fd="<<fd<<std::endl;
            close(ctx->client_fd());
            contexts_.erase(fd);
            delete ctx;
            return;
        }
        std::cout<<"[UPSTREAM] async connected fd="<<fd<<std::endl;
    }

    ctx->set_state(ProxyContext::SENDING);
    write(fd, ctx->send_buf.c_str(), ctx->send_buf.size());
    ctx->set_state(ProxyContext::RECEIVING);
    loop_.add_fd(fd, EPOLLIN);

    std::cout<<"[STATE] fd="<<ctx->client_fd()<<" SENDING→RECEIVING"<<std::endl;
}

void ReadManager::on_upstream_read(int fd, ProxyContext* ctx) {
    ctx->set_state(ProxyContext::RECEIVING);

    char resp[8192];
    int n=read(fd, resp, sizeof(resp)-1);
    if(n>0)
    {
        write(ctx->client_fd(), resp, n);
    }

    ctx->set_state(ProxyContext::DONE);

    loop_.del_fd(fd);

    contexts_.erase(fd);

    std::cout<<"[STATE] fd="<<ctx->client_fd()<<" RECEIVING→DONE"<<std::endl;

    delete ctx;
}
