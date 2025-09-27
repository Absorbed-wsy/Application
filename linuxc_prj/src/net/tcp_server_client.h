#ifndef _TCP_SERVER_CLIENT_
#define _TCP_SERVER_CLIENT_

#ifdef __cplusplus
extern "C" {
#endif

#include <netinet/in.h>
#include <arpa/inet.h>

/* =================================== API ======================================= */
// TCP 服务器 初始化
int tcp_server_init(int port);
// TCP 客户端 初始化
int tcp_client_init(struct sockaddr_in *server);

// TCP 服务器 连接
int tcp_server_accept(int sockfd);

// TCP 服务器 关闭
void tcp_server_exit(int sockfd);
// TCP 客户端 关闭
void tcp_client_exit(int sockfd);

// TCP 服务器 接收数据
int tcp_server_recv(int sockfd, char *buf, int len, int timeout);
// TCP 客户端 接收数据
int tcp_client_recv(int sockfd, char *buf, int len, int timeout);

// TCP 服务器 发送数据
int tcp_server_send(int sockfd, char *buf, int len, int timeout);
// TCP 客户端 发送数据
int tcp_client_send(int sockfd, char *buf, int len, int timeout);


#ifdef __cplusplus
}
#endif

#endif