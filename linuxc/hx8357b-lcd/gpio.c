#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "gpio.h"

#define SYSFS_GPIO_DIR "/sys/class/gpio"
#define BUFFER_MAX 3
#define DIRECTION_MAX 35
#define VALUE_MAX 30

int gpio_export(int pin) 
{
    char buffer[BUFFER_MAX];
    int len;
    int fd;
	int ret;

    fd = open(SYSFS_GPIO_DIR "/export", O_WRONLY);
    if (fd < 0) {
        perror("gpio/open-export");
        return fd;
    }

    len = snprintf(buffer, BUFFER_MAX, "%d", pin);
    ret = write(fd, buffer, len);
	if(ret < 0) {
		perror("gpio/write-export");
	}
	
    close(fd);

    return 0;
}

int gpio_unexport(int pin) 
{
    char buffer[BUFFER_MAX];
    int len;
    int fd;
	int ret;

    fd = open(SYSFS_GPIO_DIR "/unexport", O_WRONLY);
    if (fd < 0) {
        perror("gpio/open-unexport");
        return fd;
    }

    len = snprintf(buffer, BUFFER_MAX, "%d", pin);
    ret = write(fd, buffer, len);
	if(ret < 0) {
		perror("gpio/write-unexport");
	}
	
    close(fd);

    return 0;
}

int gpio_direction(int pin, int dir) 
{
    static const char directions_str[] = "in\0out";
    char path[DIRECTION_MAX];
    int fd;

    snprintf(path, DIRECTION_MAX, SYSFS_GPIO_DIR "/gpio%d/direction", pin);
    fd = open(path, O_WRONLY);
    if (fd < 0) {
        perror("gpio/direction");
        return fd;
    }

    if (write(fd, &directions_str[dir == 0 ? 0 : 3], dir == 0 ? 2 : 3) < 0) {
        perror("gpio/set-direction");
        return fd;
    }

    close(fd);
    return 0;
}

int gpio_write(int pin, int value) 
{
    static const char values_str[] = "01";
    char path[VALUE_MAX];
    int fd;

    snprintf(path, VALUE_MAX, SYSFS_GPIO_DIR "/gpio%d/value", pin);
    fd = open(path, O_WRONLY);
    if (fd < 0) {
        perror("gpio/set-value");
        return fd;
    }

    if (write(fd, &values_str[value == 0 ? 0 : 1], 1) < 0) {
        perror("gpio/set-value");
        return fd;
    }

    close(fd);
    return 0;
}

int gpio_read(int pin) 
{
    char path[VALUE_MAX];
    char value_str[3];
    int fd;

    snprintf(path, VALUE_MAX, SYSFS_GPIO_DIR "/gpio%d/value", pin);
    fd = open(path, O_RDONLY);
    if (fd < 0) {
        perror("gpio/get-value");
        return fd;
    }

    if (read(fd, value_str, 3) < 0) {
        perror("gpio/get-value");
        return fd;
    }

    close(fd);

    return atoi(value_str);
}

void blacklight_gpio(void)
{
	gpio_export(GPIO_BL);
	gpio_direction(GPIO_BL, 1);
}

void cs_gpio(void)
{
	gpio_export(GPIO_CS);
	gpio_direction(GPIO_CS, 1);
}

void res_gpio(void)
{
	gpio_export(GPIO_RST);
	gpio_direction(GPIO_RST, 1);
}

void setup_gpio(void) 
{
	printf("setup_gpio...\n");
	
    gpio_export(GPIO_CLK);
    gpio_export(GPIO_MOSI);

    gpio_direction(GPIO_CLK, 1);
    gpio_direction(GPIO_MOSI, 1);
}

void cleanup_gpio(void) 
{
    gpio_unexport(GPIO_CLK);
    gpio_unexport(GPIO_MOSI);
    gpio_unexport(GPIO_CS);
    gpio_unexport(GPIO_RST);
    gpio_unexport(GPIO_BL);
}