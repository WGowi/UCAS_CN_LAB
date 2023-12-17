cmake_minimum_required(VERSION 3.20)
project(httpserver C)

set(CMAKE_C_STANDARD 11)

find_package(OpenSSL REQUIRED)

# 包括 OpenSSL 头文件
include_directories(${OPENSSL_INCLUDE_DIR})

# 假设 basefun.c 实现了 basefun.h 的功能
add_library(basefun basefun.c)

# 链接 basefun 库到 OpenSSL
target_link_libraries(basefun ${OPENSSL_LIBRARIES})

# 添加可执行文件
add_executable(server server.c)


# 链接 http_server 到 basefun 和 OpenSSL
target_link_libraries(server basefun ${OPENSSL_LIBRARIES})

