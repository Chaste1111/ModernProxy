cmake_minimum_required(VERSION 3.14)
project(ModernProxy VERSION 1.0.0 LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(ModernProxy src/main.cpp src/config.cpp src/tcp_listener.cpp src/event_loop.cpp src/acceptor.cpp src/readManager.cpp src/http_parser.cpp src/upstream.cpp src/proxy_context.cpp)