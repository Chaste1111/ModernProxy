#include "proxy_context.h"
#include <unistd.h>

ProxyContext::ProxyContext(int client_fd)
    : client_fd_(client_fd) {}

ProxyContext::~ProxyContext() {
    if(client_fd_>=0) close(client_fd_);
    if(upstream_fd_>=0) close(upstream_fd_);
}
