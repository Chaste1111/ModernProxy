# ModernProxy

基于 epoll 的异步 HTTP 反向代理器，C++17 实现。

## 功能

- epoll 非阻塞 I/O，单线程事件循环
- 异步 DNS 解析（getaddrinfo）
- HTTP 请求/响应的流式转发，支持跨 TCP 包解析
- 非阻塞 connect（EINPROGRESS）
- 优雅退出（Ctrl+C）
- 端口号自动解析（支持非标准端口代理）

## 快速开始

```bash
# 编译
mkdir build && cd build && cmake .. && make

# 运行（默认端口 8080）
./build/ModernProxy

# 指定端口
./build/ModernProxy -p 9090

# 通过代理访问
curl --proxy http://localhost:8080 http://example.com
```

## 架构

```
main → Config → TcpListener → EventLoop (epoll)
                                  │
                    ┌─── Acceptor (accept + 注册 EPOLLIN)
                    │
                    └─── ReadManager
                           ├── HttpParser (状态机)
                           ├── ProxyContext
                           └── UpstreamConn (getaddrinfo → connect)
```

## 状态

v1.0 — 核心代理链路已跑通。
