#include "basefun.h"


int Socket(int family, int type, int protocol) {
    int sockfd;
    if ((sockfd = socket(family, type, protocol)) < 0) {
        perror("Socket Error");
        exit(1);
    }
    return sockfd;
}

void Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    if (bind(sockfd, addr, addrlen) < 0) {
        perror("Bind Error");
        exit(1);
    }
}

void Listen(int sockfd, int backlog) {
    if (listen(sockfd, backlog) < 0) {
        perror("Listen Error");
        exit(1);
    }
}

int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
    int connfd;
    if ((connfd = accept(sockfd, addr, addrlen)) < 0) {
        perror("Accept Error");
        exit(1);
    }
    return connfd;
}

long Read(int fd, void *buf, size_t count) {
    long n;
    if ((n = read(fd, buf, count)) < 0) {
        perror("Read Error");
        exit(1);
    }
    return n;
}

void Write(int fd, void *buf, size_t count) {
    if (write(fd, buf, count) < 0) {
        perror("Write Error");
        exit(1);
    }
}

long SSL_Read(SSL *ssl, void *buf, size_t count) {
    long n;
    if ((n = SSL_read(ssl, buf, count)) < 0) {
        perror("Read Error");
        exit(1);
    }
    return n;
}

void SSL_Write(SSL *ssl, void *buf, size_t count) {
    if (SSL_write(ssl, buf, count) < 0) {
        perror("Write Error");
        exit(1);
    }
}

void Close(int fd) {
    if (close(fd) < 0) {
        perror("Close Error");
        exit(1);
    }
}

long Recv(int sockfd, void *buf, size_t len, int flags) {
    int bytes_received = recv(sockfd, buf, len, flags);
    if (bytes_received < 0) {
        perror("Error receving the data!");
        exit(1);
    }
    return bytes_received;
}

long Send(int sockfd, void *buf, size_t len, int flags) {
    int bytes_send = send(sockfd, buf, len, flags);
    if (bytes_send < 0) {
        perror("Error sending the data!");
        exit(1);
    }
    return bytes_send;
}

SSL_CTX *load_SSL() {

    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();

    SSL_CTX *ctx = SSL_CTX_new(TLS_server_method()); //创建服务端SSL会话环境

    if (!ctx) {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    SSL_CTX_set_ecdh_auto(ctx, 1);

    printf("Loading the certificate ...\n");

    if (SSL_CTX_use_certificate_file(ctx, "./../keys/cnlab.cert", SSL_FILETYPE_PEM) <= 0) {
        //加载公钥证书
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
    printf("Loading the private key ...\n");
    if (SSL_CTX_use_PrivateKey_file(ctx, "./../keys/cnlab.prikey", SSL_FILETYPE_PEM) <= 0) {
        //加载私钥
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
    printf("Check private key ...\n");
    if (SSL_CTX_check_private_key(ctx) <= 0) {
        //检查私钥
        fprintf(stderr, "Private key does not match the certificate.\n");
        exit(EXIT_FAILURE);
    }
    return ctx;
}

void extract_filename_from_url(char *url, char *filename) {
    char *ptr;
    if (!strstr(url, "cgi-bin")) {
        //说明是静态网页
        sscanf(url, "/%s", filename);
    }
}


void get_filetype(char *filename, char *filetype) {
    if (strstr(filename, ".html") || strstr(filename, ".php"))
        strcpy(filetype, "text/html");
    else if (strstr(filename, ".gif"))
        strcpy(filetype, "image/gif");
    else if (strstr(filename, ".png"))
        strcpy(filetype, "image/png");
    else if (strstr(filename, ".jpg"))
        strcpy(filetype, "image/jpeg");
    else if (strstr(filename, ".mp4"))
        strcpy(filetype, "video/mp4");
    else
        strcpy(filetype, "text/plain");
}


char *parse_https_request(char *request, char *method, char *url, char *version) {
    char *range_header = strstr(request, "Range: bytes=");
    sscanf(request, "%s %s %s", method, url, version);
    // printf("\n");
    // printf("method = %s \n", method);
    // printf("url = %s \n", url);
    // printf("version = %s \n", version);
    // printf("range = %s\n", range_header);
    return range_header;
}

void http_handle_client(int client_sockfd) {
    char buffer[BUFFER_SIZE];
    char response[BUFFER_SIZE];
    char method[BUFFER_SIZE];
    char url[BUFFER_SIZE];
    char version[BUFFER_SIZE];


    ssize_t recvBytes = Recv(client_sockfd, buffer, sizeof(buffer), 0);
    printf("HTTP Receive Request:\n %s", buffer);
    char *range_header = parse_https_request(buffer, method, url, version);
    sprintf(response, "HTTP/1.1 301 Moved Permanently\r\nLocation: https://10.37.129.41%s\r\n\r\n", url);

    ssize_t sendBytes = Send(client_sockfd, response, strlen(response), 0);

    Close(client_sockfd);

}

void https_handle_client(SSL *ssl) {
    char request[BUFFER_SIZE];
    char response[BUFFER_SIZE];

    char method[BUFFER_SIZE];
    char url[BUFFER_SIZE];
    char version[BUFFER_SIZE];

    char file_name[BUFFER_SIZE];
    char file_path[BUFFER_SIZE] = "./../";
    char file_type[20];
    int status_code;
    long file_size;


    // 读取request
    long bytes_read = SSL_Read(ssl, request, sizeof(request));
    printf("HTTPS Receive Request:\n %s", request);
    char *range_header = parse_https_request(request, method, url, version);

    extract_filename_from_url(url, file_name);
    get_filetype(file_name, file_type);

    strcat(file_path, file_name);

    FILE *file;

    // 打开文件
    file = fopen(file_path, "rb");
    if (file == NULL) {
        // 文件不存在
        status_code = 404;
        sprintf(response, "HTTP/1.1 %d Not Found\r\n\r\n", status_code);
        SSL_Write(ssl, response, strlen(response));
        return;
    }

    // 获取文件大小
    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    long start_byte = 0;
    long end_byte = file_size - 1;

    if (range_header != NULL) {
        int params_num = sscanf(range_header, "Range: bytes=%ld-%ld", &start_byte, &end_byte);
        printf("start_byte:%ld\n", start_byte);
        if (params_num != 2) {
            end_byte = file_size - 1;
        }
        printf("end_byte:%ld\n", end_byte);
        if (end_byte >= file_size || start_byte > end_byte || start_byte < 0) {
            // 不合法的Range请求, 返回416 Range Not Satisfiable
            status_code = 416;
            sprintf(response, "HTTP/1.1 %d Range Not Satisfiable\r\n\r\n", status_code);
            SSL_Write(ssl, response, strlen(response));
            fclose(file);
            return;
        }
        status_code = 206;
        sprintf(response,
                "HTTP/1.1 %d Partial Content\r\nContent-Type: %s\r\nContent-Length: %ld\r\nContent-Range: bytes %ld-%ld/%ld\r\n\r\n",
                status_code, file_type, end_byte - start_byte + 1, start_byte, end_byte, file_size);
        SSL_Write(ssl, response, strlen(response));

        // 移动文件指针到对应位置
        fseek(file, start_byte, SEEK_SET);

        // 发送文件内容
        memset(response, 0, sizeof(response));
        long remaining_bytes = end_byte - start_byte + 1;
        while (remaining_bytes > 0) {
            size_t bytes_to_read = (remaining_bytes < sizeof(response)) ? remaining_bytes : sizeof(response);
            size_t bytes_read = fread(response, 1, bytes_to_read, file);
            if (bytes_read <= 0) break;
            SSL_Write(ssl, response, bytes_read);
            remaining_bytes -= bytes_read;
        }
        fclose(file);
    } else {
        // 请求整个文件，返回200 OK
        status_code = 200;
        sprintf(response, "HTTP/1.1 %d OK\r\nContent-Type: %s\r\nContent-Length: %ld\r\n\r\n", status_code, file_type,
                file_size);
        SSL_Write(ssl, response, strlen(response));
        while (1) {
            memset(response, 0, sizeof(response));
            size_t bytes_read = fread(response, 1, sizeof(response), file);
            if (bytes_read <= 0) break;
            SSL_Write(ssl, response, bytes_read);
        }
        fclose(file);
    }
    SSL_free(ssl);
}






