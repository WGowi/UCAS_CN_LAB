#ifndef basefun_h
#define basefun_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//read方法需要的头文件
#include <unistd.h>
//socket方法需要的头文件
#include <sys/socket.h>
#include <sys/types.h>
//htonl 方法需要的头文件
#include <netinet/in.h>
//inet_ntop方法需要的头文件
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sys/wait.h>


#define BUFFER_SIZE 1024
#define SERVER_PORT_HTTP 80
#define SERVER_PORT_HTTPS 443

int Socket(int family, int type, int protocol);

void Bind(int fd, const struct sockaddr *sa, socklen_t salen);

void Listen(int fd, int backlog);

int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);

long Read(int fd, void *buf, size_t count);

void Write(int fd, void *buf, size_t count);

long SSL_Read(SSL *ssl, void *buf, size_t count);

void SSL_Write(SSL *ssl, void *buf, size_t count);

void Close(int fd);

long Recv(int sockfd, void *buf, size_t len, int flags);

long Send(int sockfd, void *buf, size_t len, int flags);

SSL_CTX *load_SSL();

void extract_filename_from_url(char *url, char *filename);

void get_filetype(char *filename, char *filetype);

void http_handle_client(int client_sockfd);

void https_handle_client(SSL *ssl);

char *parse_https_request(char *request, char *method, char *url, char *version);

#endif
