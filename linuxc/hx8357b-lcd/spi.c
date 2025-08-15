#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

#include "spi.h"

int spi_init(const char *device, uint8_t mode, uint8_t bits, uint32_t speed) 
{
	printf("spi_init...\n");
	
    int fd = open(device, O_RDWR);
    if (fd < 0) {
        perror("spi_init: can't open device");
        return -1;
    }

    if (ioctl(fd, SPI_IOC_WR_MODE, &mode) == -1) {
        perror("spi_init: can't set spi mode");
        close(fd);
        return -1;
    }

    if (ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits) == -1) {
        perror("spi_init: can't set bits per word");
        close(fd);
        return -1;
    }

    if (ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) == -1) {
        perror("spi_init: can't set max speed hz");
        close(fd);
        return -1;
    }

    return fd;
}

void spi_close(int fd) 
{
    close(fd);
}

int spi_transfer(int fd, uint8_t *tx, uint8_t *rx, size_t len) 
{
    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)tx,
        .rx_buf = (unsigned long)rx,
        .len = len,
        .speed_hz = SPI_SPEED,
        .bits_per_word = SPI_BITS_PER_WORD,
    };

    if (ioctl(fd, SPI_IOC_MESSAGE(1), &tr) < 1) {
        perror("spi_transfer: can't send spi message");
        return -1;
    }

    return 0;
}
