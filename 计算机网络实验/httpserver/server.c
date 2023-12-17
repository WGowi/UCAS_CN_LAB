#include "basefun.h"


#define BACKLOG 20


void http_server() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;

    socklen_t client_addr_len = sizeof(client_addr);

    server_socket = Socket(AF_INET, SOCK_STREAM, 0);

    if (server_socket < 0) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // 设置服务器地址结构
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVER_PORT_HTTP);

    // 绑定套接字到指定地址和端口
    Bind(server_socket, (struct sockaddr *) &server_addr, sizeof(server_addr));

    // 监听连接请求
    Listen(server_socket, BACKLOG);

    printf("HTTP Server listening on port %d...\n", SERVER_PORT_HTTP);

    while (1) {
        client_socket = Accept(server_socket, (struct sockaddr *) &client_addr, &client_addr_len);
        if (client_socket < 0) {
            perror("Error accepting connection");
            exit(EXIT_FAILURE);
        }
        // 创建子进程处理客户端请求
        pid_t pid = fork();
        if (pid < 0) {
            perror("Error forking process");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {   // 子进程
            Close(server_socket);               //子进程关闭监听套接字
            http_handle_client(client_socket);  //处理客户端请求
            exit(EXIT_SUCCESS);
        } else {
            // 父进程
            Close(client_socket);
            // 父进程关闭客户端套接字，继续等待新连接
        }

    }
    // 关闭服务器套接字
    Close(server_socket);
}


void https_server() {
    SSL_CTX *ctx = load_SSL();
    SSL *ssl;
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;

    socklen_t client_addr_len = sizeof(client_addr);

    server_socket = Socket(AF_INET, SOCK_STREAM, 0);

    if (server_socket < 0) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // 设置服务器地址结构
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVER_PORT_HTTPS);

    // 绑定套接字到指定地址和端口
    Bind(server_socket, (struct sockaddr *) &server_addr, sizeof(server_addr));

    // 监听连接请求
    Listen(server_socket, BACKLOG);

    printf("HTTPS Server listening on port %d...\n", SERVER_PORT_HTTPS);

    while (1) {
        client_socket = Accept(server_socket, (struct sockaddr *) &client_addr, &client_addr_len);
        if (client_socket < 0) {
            perror("Error accepting connection");
            exit(EXIT_FAILURE);
        }
        // 创建子进程处理客户端请求
        pid_t pid = fork();
        if (pid < 0) {
            perror("Error forking process");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            // 子进程
            Close(server_socket);               //子进程关闭监听套接字
            ssl = SSL_new(ctx);
            SSL_set_fd(ssl, client_socket);
            if (SSL_accept(ssl) <= 0) {
                ERR_print_errors_fp(stderr);
            } else {
                https_handle_client(ssl);
            }
            Close(client_socket);
            exit(EXIT_SUCCESS);
        } else {
            // 父进程
            Close(client_socket);
            // 父进程关闭客户端套接字，继续等待新连接
        }
    }

}

int main(int argc, char *argv[]) {
    pid_t pid1, pid2;
    pid1 = fork(); // 创建进程
    if (pid1 < 0) {
        printf("fork1 error\n");
    } else if (pid1 == 0) {
        //printf("1\n");
        http_server();
    }

    pid2 = fork();
    if (pid2 < 0) {
        printf("fork2 error\n");
    } else if (pid2 == 0) {
        //printf("2\n");
        https_server();
    }

    int st1, st2;
    waitpid(pid1, &st1, 0);
    waitpid(pid2, &st2, 0);
    return 0;
}
