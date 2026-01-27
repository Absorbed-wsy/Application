#ifndef _UDP_SERVER_CLIENT_
#define _UDP_SERVER_CLIENT_

#ifdef __cplusplus
extern "C" {
#endif

#include <netinet/in.h>
#include <arpa/inet.h>

/* =================================== API ======================================= */
// UDP server initialization
int udp_server_init(int port);
// UDP client initialization
int udp_client_init(struct sockaddr_in *server);

// UDP server shutdown
void udp_server_exit(int sockfd);
// UDP client shutdown
void udp_client_exit(int sockfd);

// UDP server receives data
int udp_server_recv(int sockfd, char *buf, int len, int timeout, struct sockaddr_in *client_addr);
// UDP client receives data
int udp_client_recv(int sockfd, char *buf, int len, int timeout);

// UDP server sends data
int udp_server_send(int sockfd, char *buf, int len, int timeout, struct sockaddr_in *client_addr);
// UDP client sends data
int udp_client_send(int sockfd, char *buf, int len, int timeout, struct sockaddr_in *server_addr);


#ifdef __cplusplus
}
#endif

#endif