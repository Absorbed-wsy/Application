#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/gpio.h>

// 函数：设置GPIO方向并设置初始值（如果是输出模式）
int gpio_set_direction_and_value(int fd, int gpio, int value) {
    struct gpiohandle_request req;
    memset(&req, 0, sizeof(req));
    req.lineoffsets[0] = gpio;
    req.flags = GPIOHANDLE_REQUEST_OUTPUT;
    req.lines = 1;

     // 设置值
     req.default_values[0] = value;

    if (ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &req) < 0) {
        perror("GPIO_GET_LINEHANDLE_IOCTL failed");
        return -1;
    }

    // 关闭请求句柄
    close(req.fd);
    return 0;
}

// 函数：读取GPIO值
int gpio_get_value(int fd, int gpio, int* value) {
    struct gpiohandle_request req;
    struct gpiohandle_data data;

    memset(&req, 0, sizeof(req));
    req.lineoffsets[0] = gpio;
    req.flags = GPIOHANDLE_REQUEST_INPUT;
    req.lines = 1;

    if (ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &req) < 0) {
        perror("GPIO_GET_LINEHANDLE_IOCTL failed");
        return -1;
    }

    if (ioctl(req.fd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &data) < 0) {
        perror("GPIOHANDLE_GET_LINE_VALUES_IOCTL failed");
        close(req.fd);
        return -1;
    }

    *value = data.values[0];

    close(req.fd);
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 4 || argc > 5) {
        printf("Usage: %s <gpiochip path> <gpio number> <direction: in/out> [<value: 0/1>]\n", argv[0]);
        return -1;
    }

    const char *gpiochip_path = argv[1];
    int gpio = atoi(argv[2]);
    const char *direction = argv[3];
    int value = (argc == 5) ? atoi(argv[4]) : 0;

    int fd = open(gpiochip_path, O_RDONLY);
    if (fd < 0) {
        perror("Failed to open gpiochip");
        return -1;
    }

    int dir_flag = strcmp(direction, "out") == 0 ? GPIOHANDLE_REQUEST_OUTPUT : GPIOHANDLE_REQUEST_INPUT;

    // 设置方向，如果是输出方向，则设置值
    if (dir_flag == GPIOHANDLE_REQUEST_OUTPUT) {
        if (gpio_set_direction_and_value(fd, gpio, value) < 0) {
            close(fd);
            return -1;
        }
    } else {
        if (gpio_get_value(fd, gpio, &value) < 0) {
            close(fd);
            return -1;
        }
        printf("GPIO %d value: %d\n", gpio, value);
    }

    close(fd);
    return 0;
}
