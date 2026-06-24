#pragma once
#include <string>

class UpstreamConn {
public:
    UpstreamConn(const std::string& host, int port=80);
    ~UpstreamConn();

    UpstreamConn(const UpstreamConn&)=delete;
    UpstreamConn& operator=(const UpstreamConn&)=delete;

    int fd() const { return fd_; }
    bool ok() const { return fd_>=0; }
    bool connecting() const { return connecting_; }
    int release();

private:
    int fd_=-1;
    bool connecting_=false;
};
