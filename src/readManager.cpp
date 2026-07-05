#include "readManager.h"
#include "event_loop.h"
#include "upstream.h"
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>

ReadManager::ReadManager(EventLoop& loop) : loop_(loop) {}

void ReadManager::handle_read(int fd) {
    auto it = contexts_.find(fd);
    if (it != contexts_.end()) {
        if (fd == it->second->upstream_fd())
            on_upstream_read(fd, it->second);
        else
            on_client_read(fd, it->second);
        return;
    }
    on_client_read(fd, nullptr);
}

void ReadManager::handle_write(int fd) {
    auto it = contexts_.find(fd);
    if (it != contexts_.end() && fd == it->second->upstream_fd())
        on_upstream_write(fd, it->second);
}

// 根据解析结果重建发往上游的请求行：绝对URL → path-only
static void buildUpstreamRequest(ProxyContext* ctx) {
    const auto& r = ctx->req;
    ctx->upstream_req = r.method + " " + r.path + " HTTP/1.1\r\n"
        + "Host: " + r.host;
    if (r.port != 80)
        ctx->upstream_req += ":" + std::to_string(r.port);
    ctx->upstream_req += "\r\nConnection: close\r\n\r\n";

    // 如果解析时 body 已经收到（如小 POST），补上
    size_t body_off = ctx->parser.headers_len();
    if (body_off < ctx->client_buf.size())
        ctx->upstream_req += ctx->client_buf.substr(body_off);
}

void ReadManager::on_client_read(int fd, ProxyContext* ctx) {
    // 新连接 → 建上下文
    if (ctx == nullptr) {
        ctx = new ProxyContext(fd);
        contexts_[fd] = ctx;
    }

    char buffer[8192];
    int r = read(fd, buffer, sizeof(buffer) - 1);

    if (r <= 0) {
        if (r == 0) std::cout << "[CLOSE] client fd=" << fd << std::endl;
        cleanup(ctx);
        return;
    }

    ctx->client_buf.append(buffer, r);

    if (!ctx->parsed) {
        // 尝试解析
        auto res = ctx->parser.feed(ctx->client_buf, ctx->req);
        if (res == HttpParser::ERROR) {
            std::cerr << "[PARSE ERROR] fd=" << fd << std::endl;
            cleanup(ctx);
            return;
        }
        if (res == HttpParser::INCOMPLETE) {
            std::cout << "[INCOMPLETE] fd=" << fd
                      << " waiting for more data" << std::endl;
            return;
        }

        // 解析完成 → 建立 upstream 连接
        ctx->parsed = true;
        buildUpstreamRequest(ctx);
        std::cout << "[PARSED] " << ctx->req.method << " " << ctx->req.url
                  << " -> " << ctx->req.host << ":" << ctx->req.port
                  << ctx->req.path << std::endl;

        UpstreamConn upstream(ctx->req.host, ctx->req.port);
        if (!upstream.ok()) {
            std::cerr << "[UPSTREAM FAIL] " << ctx->req.host
                      << ":" << ctx->req.port << std::endl;
            cleanup(ctx);
            return;
        }

        int up_fd = upstream.release();
        ctx->set_upstream_fd(up_fd);
        contexts_[up_fd] = ctx;

        if (upstream.connecting()) {
            ctx->set_state(ProxyContext::CONNECTING);
            loop_.add_fd(up_fd, EPOLLOUT);
            std::cout << "[STATE] fd=" << fd << " CONNECTING" << std::endl;
            return;
        }

        // 同步连上 → 直接发送
        ctx->set_state(ProxyContext::SENDING);
        write(up_fd, ctx->upstream_req.data(), ctx->upstream_req.size());
        ctx->set_state(ProxyContext::RECEIVING);
        loop_.add_fd(up_fd, EPOLLIN);
        return;
    }

    // 已解析完成，后续是 body 透传
    if (ctx->state() == ProxyContext::CONNECTING) {
        // body 在 connect 完成前到达，塞进 upstream_req
        ctx->upstream_req.append(buffer, r);
        return;
    }
    if (ctx->upstream_fd() >= 0) {
        write(ctx->upstream_fd(), buffer, r);
    }
}

void ReadManager::on_upstream_write(int fd, ProxyContext* ctx) {
    // 异步 connect 完成 → 检查连接结果
    if (ctx->state() == ProxyContext::CONNECTING) {
        int err = 0;
        socklen_t len = sizeof(err);
        getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &len);
        if (err) {
            std::cerr << "[UPSTREAM] connect error fd=" << fd << std::endl;
            cleanup(ctx);
            return;
        }
        std::cout << "[UPSTREAM] async connected fd=" << fd << std::endl;
    }

    // 发送请求
    ctx->set_state(ProxyContext::SENDING);
    write(fd, ctx->upstream_req.data(), ctx->upstream_req.size());
    ctx->set_state(ProxyContext::RECEIVING);
    loop_.mod_fd(fd, EPOLLIN);

    std::cout << "[STATE] fd=" << ctx->client_fd()
              << " SENDING→RECEIVING" << std::endl;
}

void ReadManager::on_upstream_read(int fd, ProxyContext* ctx) {
    ctx->set_state(ProxyContext::RECEIVING);

    char resp[8192];
    int n = read(fd, resp, sizeof(resp) - 1);

    if (n > 0) {
        write(ctx->client_fd(), resp, n);
        return;
    }

    if (n == 0) {
        std::cout << "[UPSTREAM CLOSED] fd=" << fd << std::endl
                  << "[STATE] fd=" << ctx->client_fd()
                  << " RECEIVING→DONE" << std::endl;
        cleanup(ctx);
        return;
    }

    if (errno != EAGAIN) {
        std::cerr << "[UPSTREAM READ ERR] fd=" << fd << std::endl;
        cleanup(ctx);
    }
}

void ReadManager::cleanup(ProxyContext* ctx) {
    int client_fd = ctx->client_fd();
    int up_fd = ctx->upstream_fd();

    contexts_.erase(client_fd);
    if (up_fd >= 0) contexts_.erase(up_fd);

    loop_.del_fd(client_fd);
    if (up_fd >= 0) loop_.del_fd(up_fd);

    delete ctx;
}
