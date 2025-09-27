#ifndef _UDP_SERVER_CLIENT_
#define _UDP_SERVER_CLIENT_

#ifdef __cplusplus
extern "C" {
#endif

#include <netinet/in.h>
#include <arpa/inet.h>

/* =================================== API ======================================= */
// UDP 服务器 初始化
int udp_server_init(int port);
// UDP 客户端 初始化
int udp_client_init(struct sockaddr_in *server);

// UDP 服务器 关闭
void udp_server_exit(int sockfd);
// UDP 客户端 关闭
void udp_client_exit(int sockfd);

// UDP 服务器 接收数据
int udp_server_recv(int sockfd, char *buf, int len, int timeout, struct sockaddr_in *client_addr);
// UDP 客户端 接收数据
int udp_client_recv(int sockfd, char *buf, int len, int timeout);

// UDP 服务器 发送数据
int udp_server_send(int sockfd, char *buf, int len, int timeout, struct sockaddr_in *client_addr);
// UDP 客户端 发送数据
int udp_client_send(int sockfd, char *buf, int len, int timeout, struct sockaddr_in *server_addr);


#ifdef __cplusplus
}
#endif

#endif