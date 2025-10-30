#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

#include "tcp_server_client.h"


/**
 * @brief 设置socket超时时间
 * @param sockfd socket文件描述符
 * @param milliseconds 超时时间（毫秒）
 */
static void set_socket_timeout(int sockfd, int milliseconds) 
{
    struct timeval tv;
    tv.tv_sec = milliseconds / 1000;
    tv.tv_usec = (milliseconds % 1000) * 1000;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
}

/**
 * @brief TCP服务器初始化
 * @param port 服务器端口号
 * @return 成功返回服务器socket描述符，失败返回-1
 */
int tcp_server_init(int port)
{
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("TCP socket creation failed");
        return -1;
    }

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("TCP bind failed");
        close(server_fd);
        return -1;
    }

    if (listen(server_fd, 5) < 0) {
        perror("TCP listen failed");
        close(server_fd);
        return -1;
    }

    return server_fd;
}

/**
 * @brief TCP服务器接受客户端连接
 * @param sockfd 服务器socket描述符
 * @return 成功返回客户端socket描述符，失败返回-1
 */
int tcp_server_accept(int sockfd)
{
    int server_fd = sockfd;
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(struct sockaddr_in);

    int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);
    if (client_fd < 0) {
        perror("TCP accept failed");
        close(server_fd);
        return -1;
    }

    //printf("Connected by %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    return client_fd;
}

/**
 * @brief TCP服务器关闭
 * @param sockfd socket描述符
 */
void tcp_server_exit(int sockfd)
{
    close(sockfd);
}

/**
 * @brief TCP服务器接收数据
 * @param sockfd 客户端socket描述符
 * @param buf 接收数据缓冲区
 * @param len 缓冲区长度
 * @param timeout 超时时间（毫秒）
 * @return 成功返回接收字节数，失败返回-1
 */
int tcp_server_recv(int sockfd, char *buf, int len, int timeout)
{
    int client_fd = sockfd;

    set_socket_timeout(client_fd, timeout);

    ssize_t len_bytes = recv(client_fd, buf, len, 0);
    if (len <= 0) {
        if (len == 0) {
            perror("Client disconnected gracefully\n");
        } else {
            perror("TCP recv error");
        }
        return -1;
    }

    return len_bytes;
}

/**
 * @brief TCP服务器发送数据
 * @param sockfd 客户端socket描述符
 * @param buf 发送数据缓冲区
 * @param len 发送数据长度
 * @param timeout 超时时间（毫秒）
 * @return 成功返回发送字节数，失败返回-1
 */
int tcp_server_send(int sockfd, char *buf, int len, int timeout)
{
    int client_fd = sockfd;

    set_socket_timeout(client_fd, timeout);

    ssize_t len_bytes = send(client_fd, buf, len, 0);

    return len_bytes;
}


/**
 * @brief TCP客户端初始化
 * @param server 服务器地址结构体指针
 * @return 成功返回socket描述符，失败返回-1
 */
int tcp_client_init(struct sockaddr_in *server)
{
    //char ip[16]="192.168.3.189";int port=10000;
    //struct sockaddr_in* addr = malloc(sizeof(struct sockaddr_in));
    //memset(addr, 0, sizeof(*addr));
    //addr->sin_family = AF_INET;
    //addr->sin_port = htons(port);
    //inet_pton(AF_INET, ip, &addr->sin_addr);

    struct sockaddr_in* server_addr = (struct sockaddr_in*)server;

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("TCP client socket creation failed");
        return -1;
    }

    if (connect(sockfd, (struct sockaddr*)server_addr, sizeof(struct sockaddr_in)) < 0) {
        perror("TCP connect failed");
        close(sockfd);
        return -1;
    }

    //printf("TCP Client connected to %s:%d\n", inet_ntoa(server_addr->sin_addr), ntohs(server_addr->sin_port));

    return sockfd;
}

/**
 * @brief TCP客户端关闭
 * @param sockfd socket描述符
 */
void tcp_client_exit(int sockfd)
{
    close(sockfd);
}

/**
 * @brief TCP客户端接收数据
 * @param sockfd socket描述符
 * @param buf 接收数据缓冲区
 * @param len 缓冲区长度
 * @param timeout 超时时间（毫秒）
 * @return 成功返回接收字节数，失败返回-1
 */
int tcp_client_recv(int sockfd, char *buf, int len, int timeout)
{
    set_socket_timeout(sockfd, timeout);

    ssize_t len_bytes = recv(sockfd, buf, len, 0);
    if (len <= 0) {
        if (len == 0) {
            perror("Client disconnected gracefully\n");
        } else {
            perror("TCP recv error");
        }
        return -1;
    }

    return len_bytes;
}

/**
 * @brief TCP客户端发送数据
 * @param sockfd socket描述符
 * @param buf 发送数据缓冲区
 * @param len 发送数据长度
 * @param timeout 超时时间（毫秒）
 * @return 成功返回发送字节数，失败返回-1
 */
int tcp_client_send(int sockfd, char *buf, int len, int timeout)
{
    set_socket_timeout(sockfd, timeout);

    ssize_t len_bytes = send(sockfd, buf, len, 0);

    return len_bytes;
}
