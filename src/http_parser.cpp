#include "http_parser.h"
#include <cstdlib>

HttpParser::Result HttpParser::feed(std::string_view data, HttpRequest& req) {
    buf_.append(data.data(), data.size());

    while (state_ != DONE) {
        if (state_ == REQUEST_LINE) {
            auto pos = buf_.find("\r\n", parsed_pos_);
            if (pos == std::string::npos) return INCOMPLETE;

            std::string line = buf_.substr(parsed_pos_, pos - parsed_pos_);
            parsed_pos_ = pos + 2;

            if (!parseRequestLine(line, req)) return ERROR;
            state_ = HEADERS;
        }

        if (state_ == HEADERS) {
            while (true) {
                auto pos = buf_.find("\r\n", parsed_pos_);
                if (pos == std::string::npos) return INCOMPLETE;

                if (pos == parsed_pos_) {
                    // 空行 → 头部结束
                    parsed_pos_ += 2;  // 跳过空行
                    state_ = DONE;
                    return COMPLETE;
                }

                std::string line = buf_.substr(parsed_pos_, pos - parsed_pos_);
                parsed_pos_ = pos + 2;

                if (!parseHeaderLine(line, req)) return ERROR;
            }
        }
    }

    return COMPLETE;
}

void HttpParser::reset() {
    state_ = REQUEST_LINE;
    buf_.clear();
    parsed_pos_ = 0;
}

bool HttpParser::parseRequestLine(const std::string& line, HttpRequest& req) {
    auto a = line.find(' ');
    auto b = line.find(' ', a + 1);
    if (a == line.npos || b == line.npos) return false;

    req.method = line.substr(0, a);
    req.url = line.substr(a + 1, b - a - 1);
    req.path = req.url;   // 默认用原始 URL
    req.port = 80;

    // 解析绝对 URL http://host[:port]/path
    if (req.url.size() > 7 &&
        (req.url[0] == 'h' || req.url[0] == 'H') &&
        (req.url[1] == 't' || req.url[1] == 'T') &&
        (req.url[2] == 't' || req.url[2] == 'T') &&
        (req.url[3] == 'p' || req.url[3] == 'P') &&
        req.url[4] == ':' && req.url[5] == '/' && req.url[6] == '/') {
        parseAbsoluteUrl(req.url, req);
    }

    return true;
}

bool HttpParser::parseHeaderLine(const std::string& line, HttpRequest& req) {
    if (line.size() < 6) return true;

    // Host: 头 — URL 解析优先，URL 已有 host 就不再覆盖
    if ((line[0] == 'H' || line[0] == 'h') &&
        (line[1] == 'o' || line[1] == 'O') &&
        (line[2] == 's' || line[2] == 'S') &&
        (line[3] == 't' || line[3] == 'T') &&
        line[4] == ':') {
        if (!req.host.empty()) return true;
        // 仅当 URL 没有 host 时才从这里取（非代理模式）
        auto val = line.substr(5);
        auto start = val.find_first_not_of(" \t");
        if (start != val.npos) req.host = val.substr(start);
        return true;
    }
    return true;
}

void HttpParser::parseAbsoluteUrl(const std::string& url, HttpRequest& req) {
    // 跳过 http:// (7 字符, 已确认)
    std::string rest = url.substr(7);

    // 找路径开始的第一斜杠
    auto slash = rest.find('/');
    std::string authority;

    if (slash == rest.npos) {
        // http://host 无路径
        authority = rest;
        req.path = "/";
    } else {
        authority = rest.substr(0, slash);
        req.path = rest.substr(slash);
    }

    // 从 authority 解析 host:port
    auto colon = authority.find(':');
    if (colon == authority.npos) {
        req.host = authority;
        req.port = 80;
    } else {
        req.host = authority.substr(0, colon);
        req.port = std::atoi(authority.substr(colon + 1).c_str());
        if (req.port <= 0) req.port = 80;
    }
}
