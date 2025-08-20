#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdbool.h>
#include <ctype.h>

// 全局状态标志
volatile bool udp_server_running = false;
volatile bool tcp_server_running = false;
volatile bool udp_client_running = false;
volatile bool tcp_client_running = false;

// 互斥锁保护标准输出
pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;

// 数据处理函数
const char* handle_data(const char* data) {
    // 转换为小写进行比较
    char buffer[1024];
    strncpy(buffer, data, sizeof(buffer));
    for (char *p = buffer; *p; p++) *p = tolower(*p);

    // 移除换行符
    char *pos = strchr(buffer, '\n');
    if (pos) *pos = '\0';
    pos = strchr(buffer, '\r');
    if (pos) *pos = '\0';

    // 处理命令
    if (strcmp(buffer, "ping") == 0) {
        return "Pong!";
    } else if (strcmp(buffer, "time") == 0) {
        time_t now = time(NULL);
        return ctime(&now);
    }
    return "Unknown command";
}

// 设置套接字超时
void set_socket_timeout(int sockfd, int milliseconds) {
    struct timeval tv;
    tv.tv_sec = milliseconds / 1000;
    tv.tv_usec = (milliseconds % 1000) * 1000;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
}

// UDP 服务器线程
void* udp_server(void* arg) {
    int port = *((int*)arg);
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("UDP socket creation failed");
        return NULL;
    }

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("UDP bind failed");
        close(sockfd);
        return NULL;
    }

    pthread_mutex_lock(&print_mutex);
    printf("UDP Server started on port %d\n", port);
    pthread_mutex_unlock(&print_mutex);

    udp_server_running = true;
    char buffer[1024];
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    while (udp_server_running) {
        ssize_t len = recvfrom(sockfd, buffer, sizeof(buffer), 0,
                              (struct sockaddr*)&client_addr, &addr_len);
        if (len < 0) {
            perror("UDP recvfrom error");
            break;
        }
        buffer[len] = '\0';

        pthread_mutex_lock(&print_mutex);
        printf("Received from %s:%d: %s\n", 
               inet_ntoa(client_addr.sin_addr), 
               ntohs(client_addr.sin_port), 
               buffer);
        pthread_mutex_unlock(&print_mutex);

        const char* response = handle_data(buffer);
        sendto(sockfd, response, strlen(response), 0,
              (struct sockaddr*)&client_addr, addr_len);
    }

    close(sockfd);
    udp_server_running = false;
    pthread_mutex_lock(&print_mutex);
    printf("UDP Server stopped\n");
    pthread_mutex_unlock(&print_mutex);
    return NULL;
}

// TCP 服务器线程
void* tcp_server(void* arg) {
    int port = *((int*)arg);
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("TCP socket creation failed");
        return NULL;
    }

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("TCP bind failed");
        close(server_fd);
        return NULL;
    }

    if (listen(server_fd, 5) < 0) {
        perror("TCP listen failed");
        close(server_fd);
        return NULL;
    }

    pthread_mutex_lock(&print_mutex);
    printf("TCP Server started on port %d\n", port);
    pthread_mutex_unlock(&print_mutex);

    tcp_server_running = true;
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);
    if (client_fd < 0) {
        perror("TCP accept failed");
        close(server_fd);
        tcp_server_running = false;
        return NULL;
    }

    pthread_mutex_lock(&print_mutex);
    printf("Connected by %s:%d\n", 
           inet_ntoa(client_addr.sin_addr), 
           ntohs(client_addr.sin_port));
    pthread_mutex_unlock(&print_mutex);

    char buffer[1024];
    while (tcp_server_running) {
        ssize_t len = recv(client_fd, buffer, sizeof(buffer), 0);
        if (len <= 0) {
            if (len == 0) {
                pthread_mutex_lock(&print_mutex);
                printf("Client disconnected gracefully\n");
                pthread_mutex_unlock(&print_mutex);
            } else {
                perror("TCP recv error");
            }
            break;
        }
        buffer[len] = '\0';

        pthread_mutex_lock(&print_mutex);
        printf("Received: %s\n", buffer);
        pthread_mutex_unlock(&print_mutex);

        const char* response = handle_data(buffer);
        send(client_fd, response, strlen(response), 0);
    }

    close(client_fd);
    close(server_fd);
    tcp_server_running = false;
    pthread_mutex_lock(&print_mutex);
    printf("TCP Server stopped\n");
    pthread_mutex_unlock(&print_mutex);
    return NULL;
}

// UDP 客户端接收线程
void* udp_client_receiver(void* arg) {
    int sockfd = *((int*)arg);
    char buffer[1024];
    struct sockaddr_in from_addr;
    socklen_t addr_len = sizeof(from_addr);

    while (udp_client_running) {
        ssize_t len = recvfrom(sockfd, buffer, sizeof(buffer), 0,
                              (struct sockaddr*)&from_addr, &addr_len);
        if (len > 0) {
            buffer[len] = '\0';
            time_t now = time(NULL);
            struct tm *tm = localtime(&now);
            
            pthread_mutex_lock(&print_mutex);
            printf("\n[%02d:%02d:%02d] Received: %s", 
                   tm->tm_hour, tm->tm_min, tm->tm_sec, buffer);
            printf("\nEnter message (or 'quit'): ");
            fflush(stdout);
            pthread_mutex_unlock(&print_mutex);
        }
        usleep(10000); // 10ms
    }
    return NULL;
}

// UDP 客户端主函数
void* udp_client(void* arg) {
    struct sockaddr_in* server_addr = (struct sockaddr_in*)arg;
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("UDP client socket creation failed");
        return NULL;
    }

    set_socket_timeout(sockfd, 100);

    pthread_mutex_lock(&print_mutex);
    printf("UDP Client sending to %s:%d\n", 
           inet_ntoa(server_addr->sin_addr), 
           ntohs(server_addr->sin_port));
    pthread_mutex_unlock(&print_mutex);

    udp_client_running = true;
    pthread_t recv_thread;
    pthread_create(&recv_thread, NULL, udp_client_receiver, &sockfd);

    char input[1024];
    while (udp_client_running) {
        printf("Enter message (or 'quit'): ");
        fflush(stdout);
        if (fgets(input, sizeof(input), stdin)) {
            // 移除换行符
            input[strcspn(input, "\n")] = '\0';
            
            if (strcmp(input, "quit") == 0) {
                break;
            }
            
            sendto(sockfd, input, strlen(input), 0,
                  (struct sockaddr*)server_addr, sizeof(*server_addr));
        }
    }

    udp_client_running = false;
    pthread_join(recv_thread, NULL);
    close(sockfd);
    
    pthread_mutex_lock(&print_mutex);
    printf("\nUDP Client stopped\n");
    pthread_mutex_unlock(&print_mutex);
    return NULL;
}

// TCP 客户端接收线程
void* tcp_client_receiver(void* arg) {
    int sockfd = *((int*)arg);
    char buffer[1024];

    while (tcp_client_running) {
        ssize_t len = recv(sockfd, buffer, sizeof(buffer), 0);
        if (len > 0) {
            buffer[len] = '\0';
            time_t now = time(NULL);
            struct tm *tm = localtime(&now);
            
            pthread_mutex_lock(&print_mutex);
            printf("\n[%02d:%02d:%02d] Received: %s", 
                   tm->tm_hour, tm->tm_min, tm->tm_sec, buffer);
            printf("\nEnter message (or 'quit'): ");
            fflush(stdout);
            pthread_mutex_unlock(&print_mutex);
        } else if (len == 0) {
            pthread_mutex_lock(&print_mutex);
            printf("\nServer disconnected\n");
            pthread_mutex_unlock(&print_mutex);
            tcp_client_running = false;
            break;
        }
        usleep(10000); // 10ms
    }
    return NULL;
}

// TCP 客户端主函数
void* tcp_client(void* arg) {
    struct sockaddr_in* server_addr = (struct sockaddr_in*)arg;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("TCP client socket creation failed");
        return NULL;
    }

    set_socket_timeout(sockfd, 100);

    if (connect(sockfd, (struct sockaddr*)server_addr, sizeof(*server_addr)) < 0) {
        perror("TCP connect failed");
        close(sockfd);
        return NULL;
    }

    pthread_mutex_lock(&print_mutex);
    printf("TCP Client connected to %s:%d\n", 
           inet_ntoa(server_addr->sin_addr), 
           ntohs(server_addr->sin_port));
    pthread_mutex_unlock(&print_mutex);

    tcp_client_running = true;
    pthread_t recv_thread;
    pthread_create(&recv_thread, NULL, tcp_client_receiver, &sockfd);

    char input[1024];
    while (tcp_client_running) {
        printf("Enter message (or 'quit'): ");
        fflush(stdout);
        if (fgets(input, sizeof(input), stdin)) {
            // 移除换行符
            input[strcspn(input, "\n")] = '\0';
            
            if (strcmp(input, "quit") == 0) {
                break;
            }
            
            send(sockfd, input, strlen(input), 0);
        }
    }

    tcp_client_running = false;
    pthread_join(recv_thread, NULL);
    close(sockfd);
    
    pthread_mutex_lock(&print_mutex);
    printf("\nTCP Client stopped\n");
    pthread_mutex_unlock(&print_mutex);
    return NULL;
}

// 主菜单
int main_menu() {
    printf("\n--- Main Menu ---\n");
    printf("1. Start UDP Server\n");
    printf("2. Start TCP Server\n");
    printf("3. Start UDP Client\n");
    printf("4. Start TCP Client\n");
    printf("5. Exit\n");
    
    printf("Enter your choice: ");
    int choice;
    scanf("%d", &choice);
    getchar(); // 消耗换行符
    
    if (choice < 1 || choice > 5) {
        printf("Invalid choice, please try again\n");
        return 0;
    }
    return choice;
}

int main() {
    pthread_t udp_server_th = 0, tcp_server_th = 0, udp_client_th = 0, tcp_client_th = 0;
    int choice;

    choice = main_menu();

    while (choice != 5) {
        switch (choice) {
            case 1: // UDP Server
                if (!udp_server_running) {
                    printf("Enter UDP Server port: ");
                    int port;
                    scanf("%d", &port);
                    getchar();
                    
                    int* port_ptr = malloc(sizeof(int));
                    *port_ptr = port;
                    pthread_create(&udp_server_th, NULL, udp_server, port_ptr);
                } else {
                    printf("UDP Server is already running\n");
                }
                break;
                
            case 2: // TCP Server
                if (!tcp_server_running) {
                    printf("Enter TCP Server port: ");
                    int port;
                    scanf("%d", &port);
                    getchar();
                    
                    int* port_ptr = malloc(sizeof(int));
                    *port_ptr = port;
                    pthread_create(&tcp_server_th, NULL, tcp_server, port_ptr);
                } else {
                    printf("TCP Server is already running\n");
                }
                break;
                
            case 3: // UDP Client
                if (!udp_client_running) {
                    char ip[16];
                    printf("Enter server IP: ");
                    scanf("%15s", ip);
                    printf("Enter server port: ");
                    int port;
                    scanf("%d", &port);
                    getchar();
                    
                    struct sockaddr_in* addr = malloc(sizeof(struct sockaddr_in));
                    memset(addr, 0, sizeof(*addr));
                    addr->sin_family = AF_INET;
                    addr->sin_port = htons(port);
                    inet_pton(AF_INET, ip, &addr->sin_addr);
                    
                    pthread_create(&udp_client_th, NULL, udp_client, addr);
                } else {
                    printf("UDP Client is already running\n");
                }
                break;
                
            case 4: // TCP Client
                if (!tcp_client_running) {
                    char ip[16];
                    printf("Enter server IP: ");
                    scanf("%15s", ip);
                    printf("Enter server port: ");
                    int port;
                    scanf("%d", &port);
                    getchar();
                    
                    struct sockaddr_in* addr = malloc(sizeof(struct sockaddr_in));
                    memset(addr, 0, sizeof(*addr));
                    addr->sin_family = AF_INET;
                    addr->sin_port = htons(port);
                    inet_pton(AF_INET, ip, &addr->sin_addr);
                    
                    pthread_create(&tcp_client_th, NULL, tcp_client, addr);
                } else {
                    printf("TCP Client is already running\n");
                }
                break;
        }

        sleep(2);
        
        // 等待活动线程完成
        if (udp_server_running || tcp_server_running || udp_client_running || tcp_client_running) {
            choice = 0;
        } else {
            choice = main_menu();
        }
    }
    
    printf("Exiting...\n");
    
    // 清理资源
    if (udp_server_th) pthread_join(udp_server_th, NULL);
    if (tcp_server_th) pthread_join(tcp_server_th, NULL);
    if (udp_client_th) pthread_join(udp_client_th, NULL);
    if (tcp_client_th) pthread_join(tcp_client_th, NULL);
    
    return 0;
}
