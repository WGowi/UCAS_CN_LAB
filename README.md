# UCAS_CN_LAB

# HTTP服务器实验


## 实验要求

•实现：使用C语言实现最简单的HTTP服务器

•同时支持HTTP（80端口）和HTTPS（443端口）

•使用两个线程分别监听各自端口

•只需支持GET方法，解析请求报文，返回相应应答及内容

![image-20231217164710387](https://gowi-picgo.oss-cn-shenzhen.aliyuncs.com/202312171647444.png)



## 实验环境

-   编程语言：C
-   系统环境：Ubuntu22.04
-   依赖库：OpenSSL（用于HTTPS）



## 程序设计思路概述

`server.c` 文件是一个用C语言编写的程序，它使用在 `basefun.h` 中声明并在 `basefun.c` 中定义的函数来创建一个能够同时处理 HTTP 和 HTTPS 请求的服务器。它定义了两个主要功能，`http_server` 和 `https_server`，并使用 `main` 函数来初始化和运行这些服务器。以下是其功能的详细描述：

1. **HTTP 服务器（`http_server` 函数）**：
    - 初始化用于 HTTP 的服务器套接字
    - 将套接字绑定到指定的 HTTP 端口（`SERVER_PORT_HTTP`），并监听传入的连接
    - 使用 `while` 循环不断地接受新的客户端连接
    - 对于每个客户端连接，它会 fork 一个新进程来处理请求
    - 子进程调用 `http_handle_client` 来处理 HTTP 请求，然后退出
    - 父进程关闭客户端套接字，继续接受新的连接

2. **HTTPS 服务器（`https_server` 函数）**：
    - 使用 `load_SSL` 初始化 SSL 上下文
    - 为 HTTPS 设置服务器套接字
    - 将套接字绑定到指定的 HTTPS 端口（`SERVER_PORT_HTTPS`）并监听连接
    - 类似于 HTTP 服务器，它在循环中接受连接，并为每个客户端 fork 一个新进程
    - 在子进程中，创建一个新的 SSL 对象，将其与客户端套接字关联，并使用 `SSL_accept` 进行 SSL 握手
    - 握手成功后，调用 `https_handle_client` 来处理 HTTPS 请求
    - 子进程随后关闭客户端套接字并退出

3. **同时运行的服务器**：
    - `main` 函数 fork 了两次，以创建两个单独的进程
    - 第一个子进程运行 HTTP 服务器，第二个子进程运行 HTTPS 服务器
    - 这种设计允许服务器同时处理 HTTP 和 HTTPS 请求

4. **进程管理**：
    - `main` 函数使用 `waitpid` 等待两个子进程终止
    - 包含了适当的错误处理，以确保在 fork 过程中出现问题时服务器能够正确地运行

5. **套接字和进程管理**：
    - 在 fork 后的子进程中关闭服务器套接字，这是并发服务器设计中的一个好习惯
    - 每个子进程负责在处理请求后关闭其客户端套接字
    - 其中子进程首先关闭监听套接字再处理



## 相关文件概述

1.  **basefun.h**：该文件包含用于服务器网络编程的函数声明。它引入了必要的库，定义了缓冲区大小，HTTP 和 HTTPS 的服务端口，并声明了一系列函数，包括套接字操作、SSL操作、文件处理、以及HTTP和HTTPS请求处理
2.  **basefun.c**：此文件实现了 `basefun.h` 中声明的函数。主要功能包括：
    -   网络套接字操作：创建、绑定、监听、接受连接、读写数据
    -   SSL 操作：SSL 读写、SSL 上下文加载
    -   请求处理：解析请求、处理 HTTP 和 HTTPS 客户端请求、文件类型判断、URL解析
3.  **server.c**：这是服务器的主体部分，包括两个主要函数 `http_server` 和 `https_server`，以及用于初始化和运行这些服务器的 `main` 函数
    -   `http_server`：设置 HTTP 服务器，监听端口80，对每个接入的客户端请求 fork 一个新进程，并在子进程中调用 `http_handle_client`
    -   `https_server`：设置 HTTPS 服务器，监听端口443，加载 SSL 上下文，对每个客户端请求进行类似处理，但使用 SSL 加密的读写函数
    -   `main`：在主函数中，程序 fork 两次，分别运行 HTTP 和 HTTPS 服务器，允许它们并发处理请求

此程序实现了一个能够处理 HTTP 和 HTTPS 请求的基本 Web 服务器模型。它主要应用来 C 语言中的网络编程、进程管理和 SSL/TLS 处理

## 运行结果显示

1.   在虚拟机上执行 `sudo python3 topo.py`

![image-20231217165222524](https://gowi-picgo.oss-cn-shenzhen.aliyuncs.com/202312171652547.png)

2.   在虚拟机上编译并c语言程序

```shell
mkdir build
cd build
cmake ..
make
```

3.   在虚拟机上执行程序`sudo ./server`

![image-20231217165552736](https://gowi-picgo.oss-cn-shenzhen.aliyuncs.com/202312171655757.png)

4.   在本地运行测试程序，验证程序正确性`python3 test.py`

![image-20231217172636510](https://gowi-picgo.oss-cn-shenzhen.aliyuncs.com/202312171726541.png)



5.   请求服务器上的视频，浏览器输入`https://10.37.129.41/dir/video.mp4`

![image-20231217173300690](https://gowi-picgo.oss-cn-shenzhen.aliyuncs.com/202312171733710.png)

6.   请求服务器上的html文档，浏览器输入`https://10.37.129.41/dir/index.html`

![image-20231217185119474](https://gowi-picgo.oss-cn-shenzhen.aliyuncs.com/202312171851503.png)



## 实验结论

-   实验展示了如何使用 C 语言和相关网络编程技术来创建简单的 HTTP 和 HTTPS 服务器
-   通过对 socket 编程和 SSL加密的应用，服务器能够有效地处理网络请求
-   并发处理机制（通过 `fork`）使服务器能够同时处理多个客户端请求



## 实验总结

`basefun.c`, `basefun.h`, 和 `server.c` 文件实现的 HTTP 和 HTTPS 服务器能够在基本水平上处理网络请求，展示了网络编程和SSL集成的基本原理。但是，对于生产环境，还需要更多的安全性、稳定性和可扩展性考虑。
