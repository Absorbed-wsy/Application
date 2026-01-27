#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

#include "udp_server_client.h"

/**
 * @brief Set socket timeout
 * @param sockfd Socket file descriptor
 * @param milliseconds Timeout in milliseconds
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
 * @brief UDP server initialization
 * @param port Server port number
 * @return Returns socket descriptor on success, -1 on failure
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
 * @brief UDP server shutdown
 * @param sockfd Socket descriptor
 */
void udp_server_exit(int sockfd)
{
    close(sockfd);
}

/**
 * @brief UDP server receives data
 * @param sockfd Socket descriptor
 * @param buf Receive data buffer
 * @param len Buffer length
 * @param timeout Timeout in milliseconds
 * @param client_addr Client address structure pointer (can be NULL)
 * @return Returns number of bytes received on success, -1 on failure
 */
int udp_server_recv(int sockfd, char *buf, int len, int timeout, struct sockaddr_in *client_addr)
{
    struct sockaddr_in temp_addr; // Temporary storage address
    socklen_t addr_len = sizeof(struct sockaddr_in);
    struct sockaddr_in *target_addr = client_addr; // Points to target address structure

    // Handle NULL pointer case
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
 * @brief UDP server sends data
 * @param sockfd Socket descriptor
 * @param buf Send data buffer
 * @param len Send data length
 * @param timeout Timeout in milliseconds
 * @param client_addr Client address structure pointer
 * @return Returns number of bytes sent on success, -1 on failure
 */
int udp_server_send(int sockfd, char *buf, int len, int timeout, struct sockaddr_in *client_addr)
{
    socklen_t addr_len = sizeof(struct sockaddr_in);

    set_socket_timeout(sockfd, timeout);

    ssize_t len_bytes = sendto(sockfd, buf, len, 0, (struct sockaddr*)client_addr, addr_len);

    return len_bytes;
}

/**
 * @brief UDP client initialization
 * @param server Server address structure pointer
 * @return Returns socket descriptor on success, -1 on failure
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
 * @brief UDP client shutdown
 * @param sockfd Socket descriptor
 */
void udp_client_exit(int sockfd)
{
    close(sockfd);
}

/**
 * @brief UDP client receives data
 * @param sockfd Socket descriptor
 * @param buf Receive data buffer
 * @param len Buffer length
 * @param timeout Timeout in milliseconds
 * @return Returns number of bytes received on success, -1 on failure
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
 * @brief UDP client sends data
 * @param sockfd Socket descriptor
 * @param buf Send data buffer
 * @param len Send data length
 * @param timeout Timeout in milliseconds
 * @param server_addr Server address structure pointer
 * @return Returns number of bytes sent on success, -1 on failure
 */
int udp_client_send(int sockfd, char *buf, int len, int timeout, struct sockaddr_in *server_addr)
{
    socklen_t addr_len = sizeof(struct sockaddr_in);

    set_socket_timeout(sockfd, timeout);

    ssize_t len_bytes = sendto(sockfd, buf, len, 0, (struct sockaddr*)server_addr, addr_len);

    return len_bytes;
}
