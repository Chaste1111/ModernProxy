#pragma once
#include <string>

struct HttpRequest {
    std::string method;
    std::string url;       // 原始 URL（来自请求行）
    std::string path;      // 提取后的路径（绝对URL → path-only）
    std::string host;      // 目标主机
    int         port = 80; // 目标端口
};
