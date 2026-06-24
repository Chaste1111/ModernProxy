#pragma once

class Config {
public:
    int port() const { return port_; }
    void parse(int argc, char* argv[]);

private:
    int port_=8080;
};
