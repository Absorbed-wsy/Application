#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

#include "udp_server_client.h"

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
 * @brief UDP服务器初始化
 * @param port 服务器端口号
 * @return 成功返回socket描述符，失败返回-1
 */
int udp_server_init(int port)
{
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("UDP socket creation failed");
        return -1;
    }

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("UDP bind failed");
        close(sockfd);
        return -1;
    }

    return sockfd;
}

/**
 * @brief UDP服务器关闭
 * @param sockfd socket描述符
 */
void udp_server_exit(int sockfd)
{
    close(sockfd);
}

/**
 * @brief UDP服务器接收数据
 * @param sockfd socket描述符
 * @param buf 接收数据缓冲区
 * @param len 缓冲区长度
 * @param timeout 超时时间（毫秒）
 * @param client_addr 客户端地址结构体指针（可为NULL）
 * @return 成功返回接收字节数，失败返回-1
 */
int udp_server_recv(int sockfd, char *buf, int len, int timeout, struct sockaddr_in *client_addr)
{
    struct sockaddr_in temp_addr; // 临时存储地址
    socklen_t addr_len = sizeof(struct sockaddr_in);
    struct sockaddr_in *target_addr = client_addr; // 指向目标地址结构

    // 处理 NULL 指针情况
    if (target_addr == NULL) {
        target_addr = &temp_addr;
    }

    set_socket_timeout(sockfd, timeout);

    ssize_t len_bytes = recvfrom(sockfd, buf, len, 0, (struct sockaddr*)target_addr, &addr_len);
    if (len_bytes < 0) {
        perror("UDP recvfrom error");
        return -1;
    }

    //printf("Received from %s:%d: %s\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), buffer);

    return len_bytes;
}

/**
 * @brief UDP服务器发送数据
 * @param sockfd socket描述符
 * @param buf 发送数据缓冲区
 * @param len 发送数据长度
 * @param timeout 超时时间（毫秒）
 * @param client_addr 客户端地址结构体指针
 * @return 成功返回发送字节数，失败返回-1
 */
int udp_server_send(int sockfd, char *buf, int len, int timeout, struct sockaddr_in *client_addr)
{
    socklen_t addr_len = sizeof(struct sockaddr_in);

    set_socket_timeout(sockfd, timeout);

    ssize_t len_bytes = sendto(sockfd, buf, len, 0, (struct sockaddr*)client_addr, addr_len);

    return len_bytes;
}

/**
 * @brief UDP客户端初始化
 * @param server 服务器地址结构体指针
 * @return 成功返回socket描述符，失败返回-1
 */
int udp_client_init(struct sockaddr_in *server)
{
    //char ip[16]="x.x.x.x";int port;
    //struct sockaddr_in* addr = malloc(sizeof(struct sockaddr_in));
    //memset(addr, 0, sizeof(*addr));
    //addr->sin_family = AF_INET;
    //addr->sin_port = htons(port);
    //inet_pton(AF_INET, ip, &addr->sin_addr);

    struct sockaddr_in* server_addr = (struct sockaddr_in*)server;

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("UDP client socket creation failed");
        return -1;
    }

    return sockfd;
}

/**
 * @brief UDP客户端关闭
 * @param sockfd socket描述符
 */
void udp_client_exit(int sockfd)
{
    close(sockfd);
}

/**
 * @brief UDP客户端接收数据
 * @param sockfd socket描述符
 * @param buf 接收数据缓冲区
 * @param len 缓冲区长度
 * @param timeout 超时时间（毫秒）
 * @return 成功返回接收字节数，失败返回-1
 */
int udp_client_recv(int sockfd, char *buf, int len, int timeout)
{
    struct sockaddr_in server_addr;
    socklen_t addr_len = sizeof(struct sockaddr_in);

    set_socket_timeout(sockfd, timeout);

    ssize_t len_bytes = recvfrom(sockfd, buf, len, 0, (struct sockaddr*)&server_addr, &addr_len);
    if (len_bytes < 0) {
        perror("UDP recvfrom error");
        return -1;
    }

    //printf("Received from %s:%d: %s\n", inet_ntoa(from_addr.sin_addr), ntohs(from_addr.sin_port), buffer);

    return len_bytes;
}

/**
 * @brief UDP客户端发送数据
 * @param sockfd socket描述符
 * @param buf 发送数据缓冲区
 * @param len 发送数据长度
 * @param timeout 超时时间（毫秒）
 * @param server_addr 服务器地址结构体指针
 * @return 成功返回发送字节数，失败返回-1
 */
int udp_client_send(int sockfd, char *buf, int len, int timeout, struct sockaddr_in *server_addr)
{
    socklen_t addr_len = sizeof(struct sockaddr_in);

    set_socket_timeout(sockfd, timeout);

    ssize_t len_bytes = sendto(sockfd, buf, len, 0, (struct sockaddr*)server_addr, addr_len);

    return len_bytes;
}
