#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

#include "udp_server_client.h"


static void set_socket_timeout(int sockfd, int milliseconds) 
{
    struct timeval tv;
    tv.tv_sec = milliseconds / 1000;
    tv.tv_usec = (milliseconds % 1000) * 1000;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
}

// UDP 服务器 初始化
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

// UDP 服务器 关闭
void udp_server_exit(int sockfd)
{
    close(sockfd);
}

// UDP 服务器 接收数据
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
    if (len < 0) {
        perror("UDP recvfrom error");
        return -1;
    }

    //printf("Received from %s:%d: %s\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), buffer);

    return len_bytes;
}

// UDP 服务器 发送数据
int udp_server_send(int sockfd, char *buf, int len, int timeout, struct sockaddr_in *client_addr)
{
    socklen_t addr_len = sizeof(struct sockaddr_in);

    set_socket_timeout(sockfd, timeout);

    ssize_t len_bytes = sendto(sockfd, buf, len, 0, (struct sockaddr*)client_addr, addr_len);

    return len_bytes;
}

// UDP 客户端 初始化
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

// UDP 客户端 关闭
void udp_client_exit(int sockfd)
{
    close(sockfd);
}

// UDP 客户端 接收数据
int udp_client_recv(int sockfd, char *buf, int len, int timeout)
{
    struct sockaddr_in server_addr;
    socklen_t addr_len = sizeof(struct sockaddr_in);

    set_socket_timeout(sockfd, timeout);

    ssize_t len_bytes = recvfrom(sockfd, buf, len, 0, (struct sockaddr*)&server_addr, &addr_len);
    if (len < 0) {
        perror("UDP recvfrom error");
        return -1;
    }

    //printf("Received from %s:%d: %s\n", inet_ntoa(from_addr.sin_addr), ntohs(from_addr.sin_port), buffer);

    return len_bytes;
}

// UDP 客户端 发送数据
int udp_client_send(int sockfd, char *buf, int len, int timeout, struct sockaddr_in *server_addr)
{
    socklen_t addr_len = sizeof(struct sockaddr_in);

    set_socket_timeout(sockfd, timeout);

    ssize_t len_bytes = sendto(sockfd, buf, len, 0, (struct sockaddr*)server_addr, addr_len);

    return len_bytes;
}
