#pragma once
#include <string>
#include <string_view>
#include "http_request.h"

class HttpParser {
public:
    enum Result { INCOMPLETE, COMPLETE, ERROR };

    Result feed(std::string_view data, HttpRequest& req);
    void reset();
    // 已解析的头部总字节数（含末尾空行），仅 COMPLETE 后有效
    size_t headers_len() const { return parsed_pos_; }

private:
    enum State { REQUEST_LINE, HEADERS, DONE };
    State state_ = REQUEST_LINE;
    std::string buf_;
    size_t parsed_pos_ = 0;

    bool parseRequestLine(const std::string& line, HttpRequest& req);
    bool parseHeaderLine(const std::string& line, HttpRequest& req);
    void parseAbsoluteUrl(const std::string& url, HttpRequest& req);
};
