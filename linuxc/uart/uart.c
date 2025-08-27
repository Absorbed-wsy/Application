#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>      // open()
#include <termios.h>    // tcgetattr(), tcsetattr()
#include <unistd.h>     // read(), write(), close()
#include <errno.h>      // errno

#define SERIAL_PORT "/dev/ttyS1"
#define BAUDRATE B115200
#define BUFFER_SIZE 128

// 打开串口
int open_serial(const char *device) {
    int fd = open(device, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0) {
        perror("Error opening serial port");
        return -1;
    }
    return fd;
}

// 配置串口参数
int configure_serial(int fd, speed_t baudrate) {
    struct termios tty;
    
    if (tcgetattr(fd, &tty) != 0) {
        perror("Error getting tty attributes");
        return -1;
    }

    cfsetospeed(&tty, baudrate); // 设置输出波特率
    cfsetispeed(&tty, baudrate); // 设置输入波特率

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8; // 8位数据位
    tty.c_cflag |= (CLOCAL | CREAD);            // 启用接收器，忽略调制解调器线状态
    tty.c_cflag &= ~(PARENB | PARODD);          // 无奇偶校验
    tty.c_cflag &= ~CSTOPB;                     // 1个停止位
    tty.c_cflag &= ~CRTSCTS;                    // 不使用RTS/CTS硬件流控

    tty.c_iflag &= ~(IXON | IXOFF | IXANY);     // 关闭软件流控
    tty.c_iflag &= ~(ICRNL | INLCR);            // 关闭CR-LF转换

    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // 关闭规范模式，关闭回显
    tty.c_oflag &= ~OPOST;                     // 关闭输出处理

    tty.c_cc[VMIN] = 0;  // 非阻塞模式
    tty.c_cc[VTIME] = 10; // 1秒超时

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        perror("Error setting tty attributes");
        return -1;
    }

    return 0;
}

// 发送数据
int send_data(int fd, const char *data) {
    int len = strlen(data);
    int bytes_written = write(fd, data, len);

    if (bytes_written < 0) {
        perror("Error writing to serial port");
        return -1;
    }
    return bytes_written;
}

// 接收数据
int receive_data(int fd, char *buffer, int buffer_size) {
    int bytes_read = read(fd, buffer, buffer_size);

    if (bytes_read < 0) {
        perror("Error reading from serial port");
        return -1;
    }
    return bytes_read;
}

// 关闭串口
void close_serial(int fd) {
    if (close(fd) != 0) {
        perror("Error closing serial port");
    }
}

int main() {
    int serial_fd;
    char recv_buffer[BUFFER_SIZE];

    // 打开串口
    serial_fd = open_serial(SERIAL_PORT);
    if (serial_fd < 0) return 1;

    // 配置串口
    if (configure_serial(serial_fd, BAUDRATE) != 0) {
        close_serial(serial_fd);
        return 1;
    }

    // 发送数据
    const char *message = "Hello World!\n";
    if (send_data(serial_fd, message) < 0) {
        close_serial(serial_fd);
        return 1;
    }
    printf("Sent: %s", message);

    // 接收数据
    int bytes_received = receive_data(serial_fd, recv_buffer, BUFFER_SIZE - 1);
    if (bytes_received > 0) {
        recv_buffer[bytes_received] = '\0'; // 添加字符串终止符
        printf("Received: %s\n", recv_buffer);
    } else if (bytes_received == 0) {
        printf("No data received.\n");
    }

    // 关闭串口
    close_serial(serial_fd);

    return 0;
}