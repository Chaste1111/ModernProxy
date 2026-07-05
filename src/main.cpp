#include "config.h"
#include "tcp_listener.h"
#include "event_loop.h"
#include <iostream>
#include <csignal>

// 全局标志，SIGINT/SIGTERM 时置 0
volatile sig_atomic_t g_running = 1;
extern "C" void handleSignal(int) { g_running = 0; }

int main(int argc, char* argv[]) {
    signal(SIGINT,  handleSignal);
    signal(SIGTERM, handleSignal);

    Config cfg;
    cfg.parse(argc, argv);

    TcpListener listener(cfg.port());
    if (!listener.ok()) {
        std::cerr << "Failed to start TCP listener on port "
                  << cfg.port() << std::endl;
        return 1;
    }

    EventLoop loop;
    loop.run(listener.fd());
    std::cout << "[EXIT] bye" << std::endl;
    return 0;
}
