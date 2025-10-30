// src/peripherals/gpio.c
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/gpio.h>

/**
 * @brief 设置GPIO引脚方向和初始值
 * @param fd GPIO设备文件描述符
 * @param gpio GPIO引脚编号
 * @param value 初始输出值 (0或1)
 * @return 成功返回0，失败返回-1
 */
int gpio_set_direction_and_value(int fd, int gpio, int value) 
{
    struct gpiohandle_request req;
    memset(&req, 0, sizeof(req));
    req.lineoffsets[0] = gpio;
    req.flags = GPIOHANDLE_REQUEST_OUTPUT;
    req.lines = 1;

    req.default_values[0] = value;

    if (ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &req) < 0) {
        perror("GPIO_GET_LINEHANDLE_IOCTL failed");
        return -1;
    }

    close(req.fd);
    return 0;
}

/**
 * @brief 获取GPIO引脚当前值
 * @param fd GPIO设备文件描述符
 * @param gpio GPIO引脚编号
 * @param value 存储引脚值的指针
 * @return 成功返回0，失败返回-1
 */
int gpio_get_value(int fd, int gpio, int* value) 
{
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

/**
 * @brief GPIO控制主函数（设置方向/获取值）
 * @param gpiochip_path GPIO芯片设备路径
 * @param gpio GPIO引脚编号
 * @param direction 方向设置 ("in"或"out")
 * @param value 输出值（仅输出模式有效）
 * @return 成功返回0，失败返回-1
 */
int gpio_control(const char *gpiochip_path, int gpio, const char *direction, int value)
{
    int fd = open(gpiochip_path, O_RDONLY);
    if (fd < 0) {
        perror("Failed to open gpiochip");
        return -1;
    }

    int dir_flag = strcmp(direction, "out") == 0 ? GPIOHANDLE_REQUEST_OUTPUT : GPIOHANDLE_REQUEST_INPUT;

    if (dir_flag == GPIOHANDLE_REQUEST_OUTPUT) {
        if (gpio_set_direction_and_value(fd, gpio, value) < 0) {
            close(fd);
            return -1;
        }
    } else {
        int read_value;
        if (gpio_get_value(fd, gpio, &read_value) < 0) {
            close(fd);
            return -1;
        }
        printf("GPIO %d value: %d\n", gpio, read_value);
    }

    close(fd);
    return 0;
}
