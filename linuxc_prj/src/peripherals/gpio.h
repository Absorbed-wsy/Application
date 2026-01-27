// src/peripherals/gpio.h
#ifndef GPIO_H
#define GPIO_H

#include <gpiod.h>

#define MAX_INTERRUPT_NUM   64

// GPIO interrupt event types
typedef enum {
    GPIO_EVENT_RISING_EDGE = 1,   // Rising edge
    GPIO_EVENT_FALLING_EDGE = 2,  // Falling edge
    GPIO_EVENT_BOTH_EDGES = 3     // Both edges
} gpio_event_type_t;

// GPIO interrupt callback function type
typedef void (*gpio_interrupt_callback_t)(int gpio, gpio_event_type_t event_type,
                                         unsigned long sec, unsigned long nsec);

// Basic GPIO operation functions
int gpio_get_value(int fd, int gpio, int* value);
int gpio_set_direction_and_value(int fd, int gpio, int value);
int gpio_control(const char *gpiochip_path, int gpio, const char *direction, int value);

// GPIO interrupt related functions
int gpio_interrupt_init(const char *gpiochip_path, int gpio, gpio_event_type_t event_type);
int gpio_interrupt_register_callback(int gpio, gpio_interrupt_callback_t callback);
int gpio_interrupt_unregister_callback(int gpio);
void gpio_interrupt_cleanup(void);

#endif // GPIO_H
