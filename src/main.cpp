#include "config.h"
#include "tcp_listener.h"
#include <iostream>
#include <unistd.h>
#include <cstring>
#include "event_loop.h"

int main(int argc, char* argv[]) {
    Config cfg;
    cfg.parse(argc, argv);

    TcpListener listener(cfg.port());
    if(!listener.ok()) 
    {
        std::cerr << "Failed to start TCP listener on port " << cfg.port() << std::endl;
        return 1;
    }
    EventLoop loop;
    loop.run(listener.fd());
    return 0;

}
