// src/peripherals/gpio.h
#ifndef GPIO_H
#define GPIO_H

int gpio_get_value(int fd, int gpio, int* value) ;
int gpio_set_direction_and_value(int fd, int gpio, int value);

int gpio_control(const char *gpiochip_path, int gpio, const char *direction, int value);

#endif // GPIO_H
