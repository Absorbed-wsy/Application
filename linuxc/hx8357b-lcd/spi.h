#ifndef SPI_H
#define SPI_H

#include <stdint.h>

#define SPI_DEVICE "/dev/spidev0.0"
#define SPI_MODE 3
#define SPI_BITS_PER_WORD 16
#define SPI_SPEED 10000000

int spi_init(const char *device, uint8_t mode, uint8_t bits, uint32_t speed);
void spi_close(int fd);
int spi_transfer(int fd, uint8_t *tx, uint8_t *rx, size_t len);

#endif // SPI_H
