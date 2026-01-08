// src/peripherals/gpio.c
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/gpio.h>
#include <gpiod.h>
#include <signal.h>
#include <pthread.h>
#include "gpio.h"

// GPIO中断相关全局变量
static struct gpiod_chip *interrupt_chip = NULL;
static struct gpiod_line *interrupt_lines[MAX_INTERRUPT_NUM] = {NULL};
static gpio_interrupt_callback_t interrupt_callbacks[MAX_INTERRUPT_NUM] = {NULL};
static pthread_t interrupt_thread;
static volatile int interrupt_running = 0;

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

// GPIO中断线程函数
static void* interrupt_thread_func(void* arg) {
    (void)arg;
    
    while (interrupt_running) {
        for (int i = 0; i < MAX_INTERRUPT_NUM; i++) {
            if (interrupt_lines[i] != NULL && interrupt_callbacks[i] != NULL) {
                struct gpiod_line_event event;
                
                // 等待中断事件（非阻塞，超时100ms）
                if (gpiod_line_event_wait(interrupt_lines[i], &(struct timespec){.tv_sec = 0, .tv_nsec = 100000000}) > 0) {
                    if (gpiod_line_event_read(interrupt_lines[i], &event) == 0) {
                        gpio_event_type_t event_type;
                        
                        switch (event.event_type) {
                            case GPIOD_LINE_EVENT_RISING_EDGE:
                                event_type = GPIO_EVENT_RISING_EDGE;
                                break;
                            case GPIOD_LINE_EVENT_FALLING_EDGE:
                                event_type = GPIO_EVENT_FALLING_EDGE;
                                break;
                            default:
                                event_type = GPIO_EVENT_BOTH_EDGES;
                                break;
                        }
                        
                        // 调用回调函数
                        interrupt_callbacks[i](i, event_type, event.ts.tv_sec, event.ts.tv_nsec);
                    }
                }
            }
        }
    }
    
    return NULL;
}

/**
 * @brief 初始化GPIO中断
 * @param gpiochip_path GPIO芯片设备路径
 * @param gpio GPIO引脚编号
 * @param event_type 中断事件类型
 * @return 成功返回0，失败返回-1
 */
int gpio_interrupt_init(const char *gpiochip_path, int gpio, gpio_event_type_t event_type) {
    if (gpio < 0 || gpio >= MAX_INTERRUPT_NUM) {
        fprintf(stderr, "GPIO pin number out of range: %d\n", gpio);
        return -1;
    }
    
    // 打开GPIO芯片（如果尚未打开）
    if (interrupt_chip == NULL) {
        interrupt_chip = gpiod_chip_open(gpiochip_path);
        if (interrupt_chip == NULL) {
            perror("Failed to open gpiochip");
            return -1;
        }
    }
    
    // 获取GPIO线
    interrupt_lines[gpio] = gpiod_chip_get_line(interrupt_chip, gpio);
    if (interrupt_lines[gpio] == NULL) {
        perror("Failed to get GPIO line");
        return -1;
    }
    
    // 配置中断事件类型
    int ret = -1;
    switch (event_type) {
        case GPIO_EVENT_RISING_EDGE:
            ret = gpiod_line_request_rising_edge_events(interrupt_lines[gpio], "gpio-interrupt");
            break;
        case GPIO_EVENT_FALLING_EDGE:
            ret = gpiod_line_request_falling_edge_events(interrupt_lines[gpio], "gpio-interrupt");
            break;
        case GPIO_EVENT_BOTH_EDGES:
            ret = gpiod_line_request_both_edges_events(interrupt_lines[gpio], "gpio-interrupt");
            break;
        default:
            fprintf(stderr, "Invalid event type: %d\n", event_type);
            return -1;
    }
    
    if (ret < 0) {
        perror("Failed to configure GPIO interrupt");
        gpiod_line_release(interrupt_lines[gpio]);
        interrupt_lines[gpio] = NULL;
        return -1;
    }
    
    // 启动中断线程（如果尚未启动）
    if (!interrupt_running) {
        interrupt_running = 1;
        if (pthread_create(&interrupt_thread, NULL, interrupt_thread_func, NULL) != 0) {
            perror("Failed to create interrupt thread");
            interrupt_running = 0;
            gpiod_line_release(interrupt_lines[gpio]);
            interrupt_lines[gpio] = NULL;
            return -1;
        }
    }
    
    return 0;
}

/**
 * @brief 注册GPIO中断回调函数
 * @param gpio GPIO引脚编号
 * @param callback 回调函数指针
 * @return 成功返回0，失败返回-1
 */
int gpio_interrupt_register_callback(int gpio, gpio_interrupt_callback_t callback) {
    if (gpio < 0 || gpio >= 64) {
        fprintf(stderr, "GPIO pin number out of range: %d\n", gpio);
        return -1;
    }
    
    if (interrupt_lines[gpio] == NULL) {
        fprintf(stderr, "GPIO %d not initialized for interrupt\n", gpio);
        return -1;
    }
    
    interrupt_callbacks[gpio] = callback;
    return 0;
}

/**
 * @brief 注销GPIO中断回调函数
 * @param gpio GPIO引脚编号
 * @return 成功返回0，失败返回-1
 */
int gpio_interrupt_unregister_callback(int gpio) {
    if (gpio < 0 || gpio >= 64) {
        fprintf(stderr, "GPIO pin number out of range: %d\n", gpio);
        return -1;
    }
    
    interrupt_callbacks[gpio] = NULL;
    return 0;
}

/**
 * @brief 清理GPIO中断资源
 */
void gpio_interrupt_cleanup(void) {
    // 停止中断线程
    interrupt_running = 0;
    if (interrupt_thread) {
        pthread_join(interrupt_thread, NULL);
    }
    
    // 释放所有GPIO线
    for (int i = 0; i < 64; i++) {
        if (interrupt_lines[i] != NULL) {
            gpiod_line_release(interrupt_lines[i]);
            interrupt_lines[i] = NULL;
            interrupt_callbacks[i] = NULL;
        }
    }
    
    // 关闭GPIO芯片
    if (interrupt_chip != NULL) {
        gpiod_chip_close(interrupt_chip);
        interrupt_chip = NULL;
    }
}
