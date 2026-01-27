#ifndef _TCP_SERVER_CLIENT_
#define _TCP_SERVER_CLIENT_

#ifdef __cplusplus
extern "C" {
#endif

#include <netinet/in.h>
#include <arpa/inet.h>

/* =================================== API ======================================= */
// TCP server initialization
int tcp_server_init(int port);
// TCP client initialization
int tcp_client_init(struct sockaddr_in *server);

// TCP server accepts connection
int tcp_server_accept(int sockfd);

// TCP server shutdown
void tcp_server_exit(int sockfd);
// TCP client shutdown
void tcp_client_exit(int sockfd);

// TCP server receives data
int tcp_server_recv(int sockfd, char *buf, int len, int timeout);
// TCP client receives data
int tcp_client_recv(int sockfd, char *buf, int len, int timeout);

// TCP server sends data
int tcp_server_send(int sockfd, char *buf, int len, int timeout);
// TCP client sends data
int tcp_client_send(int sockfd, char *buf, int len, int timeout);


#ifdef __cplusplus
}
#endif

#endif