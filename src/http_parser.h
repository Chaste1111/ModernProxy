#pragma once
#include<string_view>
#include"http_request.h"


class HttpParser
{
    public:
        bool parse(std::string_view data,HttpRequest& req);
};