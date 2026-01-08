// src/peripherals/gpio.h
#ifndef GPIO_H
#define GPIO_H

#include <gpiod.h>

#define MAX_INTERRUPT_NUM   64

// GPIO中断事件类型
typedef enum {
    GPIO_EVENT_RISING_EDGE = 1,   // 上升沿
    GPIO_EVENT_FALLING_EDGE = 2,  // 下降沿
    GPIO_EVENT_BOTH_EDGES = 3     // 双边沿
} gpio_event_type_t;

// GPIO中断回调函数类型
typedef void (*gpio_interrupt_callback_t)(int gpio, gpio_event_type_t event_type,
                                         unsigned long sec, unsigned long nsec);

// 基本GPIO操作函数
int gpio_get_value(int fd, int gpio, int* value);
int gpio_set_direction_and_value(int fd, int gpio, int value);
int gpio_control(const char *gpiochip_path, int gpio, const char *direction, int value);

// GPIO中断相关函数
int gpio_interrupt_init(const char *gpiochip_path, int gpio, gpio_event_type_t event_type);
int gpio_interrupt_register_callback(int gpio, gpio_interrupt_callback_t callback);
int gpio_interrupt_unregister_callback(int gpio);
void gpio_interrupt_cleanup(void);

#endif // GPIO_H
