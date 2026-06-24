#pragma once

class TcpListener {
public:
    explicit TcpListener(int port);
    ~TcpListener();

    TcpListener(const TcpListener&)=delete;
    TcpListener& operator=(const TcpListener&)=delete;

    int fd() const { return fd_; }
    bool ok() const { return fd_>=0; }
    int accept() const;

private:
    int fd_=-1;
};
