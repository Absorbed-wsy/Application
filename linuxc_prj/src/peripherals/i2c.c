#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>

#include "i2c.h"

/**
 * @brief Initialize I2C device
 * @param dev_path I2C device path (e.g. "/dev/i2c-1")
 * @param addr Slave device address (7-bit)
 * @return Returns I2CDevice pointer on success, NULL on failure
 */
I2CDevice* i2c_init(const char* dev_path, unsigned char addr) 
{
    int fd = open(dev_path, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "Failed to open I2C device %s: %s\n", dev_path, strerror(errno));
        return NULL;
    }

    if (ioctl(fd, I2C_SLAVE, addr) < 0) {
        fprintf(stderr, "Failed to set I2C slave address 0x%02X: %s\n", addr, strerror(errno));
        close(fd);
        return NULL;
    }

    I2CDevice* dev = (I2CDevice*)malloc(sizeof(I2CDevice));
    if (!dev) {
        fprintf(stderr, "Memory allocation failed\n");
        close(fd);
        return NULL;
    }

    dev->fd = fd;
    dev->addr = addr;
    return dev;
}

/**
 * @brief Close I2C device
 * @param dev I2C device pointer
 */
void i2c_close(I2CDevice* dev) 
{
    if (dev) {
        close(dev->fd);
        free(dev);
    }
}

// ================== Basic Byte Operations ==================

/**
 * @brief SMBus read byte (no register address)
 * @param dev I2C device pointer
 * @return Returns read byte on success, -1 on failure
 */
int i2c_smbus_read_byte(I2CDevice* dev) 
{
    struct i2c_smbus_ioctl_data args;
    union i2c_smbus_data data;

    memset(&args, 0, sizeof(args));
    memset(&data, 0, sizeof(data));

    args.read_write = I2C_SMBUS_READ;
    args.command = 0;
    args.size = I2C_SMBUS_BYTE;
    args.data = &data;

    for (int i = 0; i < I2C_MAX_RETRIES; i++) {
        if (ioctl(dev->fd, I2C_SMBUS, &args) == 0) {
            return data.byte & 0xFF;
        }
        usleep(I2C_RETRY_DELAY_MS * 1000);
    }
    fprintf(stderr, "SMBus read byte failed: %s\n", strerror(errno));
    return -1;
}

/**
 * @brief SMBus write byte (no register address)
 * @param dev I2C device pointer
 * @param value Byte value to write
 * @return Returns 0 on success, -1 on failure
 */
int i2c_smbus_write_byte(I2CDevice* dev, unsigned char value) 
{
    struct i2c_smbus_ioctl_data args;
    union i2c_smbus_data data;

    memset(&args, 0, sizeof(args));
    memset(&data, 0, sizeof(data));

    data.byte = value;
    args.read_write = I2C_SMBUS_WRITE;
    args.command = 0;
    args.size = I2C_SMBUS_BYTE;
    args.data = &data;

    for (int i = 0; i < I2C_MAX_RETRIES; i++) {
        if (ioctl(dev->fd, I2C_SMBUS, &args) == 0) {
            return 0;
        }
        usleep(I2C_RETRY_DELAY_MS * 1000);
    }
    fprintf(stderr, "SMBus write byte failed: %s\n", strerror(errno));
    return -1;
}

// ================== Byte Data Operations (with register address) ==================

/**
 * @brief SMBus read byte data (with register address)
 * @param dev I2C device pointer
 * @param reg Register address
 * @return Returns read byte on success, -1 on failure
 */
int i2c_smbus_read_byte_data(I2CDevice* dev, unsigned char reg) 
{
    struct i2c_smbus_ioctl_data args;
    union i2c_smbus_data data;

    memset(&args, 0, sizeof(args));
    memset(&data, 0, sizeof(data));

    args.read_write = I2C_SMBUS_READ;
    args.command = reg;
    args.size = I2C_SMBUS_BYTE_DATA;
    args.data = &data;

    for (int i = 0; i < I2C_MAX_RETRIES; i++) {
        if (ioctl(dev->fd, I2C_SMBUS, &args) == 0) {
            return data.byte & 0xFF;
        }
        usleep(I2C_RETRY_DELAY_MS * 1000);
    }
    fprintf(stderr, "SMBus read byte data at reg 0x%02X failed: %s\n", reg, strerror(errno));
    return -1;
}

/**
 * @brief SMBus write byte data (with register address)
 * @param dev I2C device pointer
 * @param reg Register address
 * @param value Byte value to write
 * @return Returns 0 on success, -1 on failure
 */
int i2c_smbus_write_byte_data(I2CDevice* dev, unsigned char reg, unsigned char value) 
{
    struct i2c_smbus_ioctl_data args;
    union i2c_smbus_data data;

    memset(&args, 0, sizeof(args));
    memset(&data, 0, sizeof(data));

    data.byte = value;
    args.read_write = I2C_SMBUS_WRITE;
    args.command = reg;
    args.size = I2C_SMBUS_BYTE_DATA;
    args.data = &data;

    for (int i = 0; i < I2C_MAX_RETRIES; i++) {
        if (ioctl(dev->fd, I2C_SMBUS, &args) == 0) {
            return 0;
        }
        usleep(I2C_RETRY_DELAY_MS * 1000);
    }
    fprintf(stderr, "SMBus write byte data to reg 0x%02X failed: %s\n", reg, strerror(errno));
    return -1;
}

// ================== Word Data Operations (16-bit) ==================

/**
 * @brief SMBus read word data (16-bit)
 * @param dev I2C device pointer
 * @param reg Register address
 * @return Returns read word data on success, -1 on failure
 */
int i2c_smbus_read_word_data(I2CDevice* dev, unsigned char reg) 
{
    struct i2c_smbus_ioctl_data args;
    union i2c_smbus_data data;

    memset(&args, 0, sizeof(args));
    memset(&data, 0, sizeof(data));

    args.read_write = I2C_SMBUS_READ;
    args.command = reg;
    args.size = I2C_SMBUS_WORD_DATA;
    args.data = &data;

    for (int i = 0; i < I2C_MAX_RETRIES; i++) {
        if (ioctl(dev->fd, I2C_SMBUS, &args) == 0) {
            return data.word & 0xFFFF;
        }
        usleep(I2C_RETRY_DELAY_MS * 1000);
    }
    fprintf(stderr, "SMBus read word data at reg 0x%02X failed: %s\n", reg, strerror(errno));
    return -1;
}

/**
 * @brief SMBus write word data (16-bit)
 * @param dev I2C device pointer
 * @param reg Register address
 * @param value Word data to write
 * @return Returns 0 on success, -1 on failure
 */
int i2c_smbus_write_word_data(I2CDevice* dev, unsigned char reg, unsigned short value) 
{
    struct i2c_smbus_ioctl_data args;
    union i2c_smbus_data data;

    memset(&args, 0, sizeof(args));
    memset(&data, 0, sizeof(data));

    data.word = value;
    args.read_write = I2C_SMBUS_WRITE;
    args.command = reg;
    args.size = I2C_SMBUS_WORD_DATA;
    args.data = &data;

    for (int i = 0; i < I2C_MAX_RETRIES; i++) {
        if (ioctl(dev->fd, I2C_SMBUS, &args) == 0) {
            return 0;
        }
        usleep(I2C_RETRY_DELAY_MS * 1000);
    }
    fprintf(stderr, "SMBus write word data to reg 0x%02X failed: %s\n", reg, strerror(errno));
    return -1;
}

// ================== Block Operations ==================

/**
 * @brief I2C read block data (direct I2C operation)
 * @param dev I2C device pointer
 * @param reg Register address
 * @param length Length to read
 * @param values Buffer to store data
 * @return Returns number of bytes read on success, -1 on failure
 */
int i2c_read_i2c_block_data(I2CDevice* dev, unsigned char reg, unsigned char length, unsigned char* values) 
{
    // First write register address
    if (i2c_smbus_write_byte(dev, reg) < 0) {
        return -1;
    }
    
    // Then read data
    struct i2c_msg msgs[1];
    struct i2c_rdwr_ioctl_data msgset;
    
    msgs[0].addr = dev->addr;
    msgs[0].flags = I2C_M_RD;
    msgs[0].len = length;
    msgs[0].buf = values;
    
    msgset.msgs = msgs;
    msgset.nmsgs = 1;
    
    for (int i = 0; i < I2C_MAX_RETRIES; i++) {
        if (ioctl(dev->fd, I2C_RDWR, &msgset) >= 0) {
            return length;
        }
        usleep(I2C_RETRY_DELAY_MS * 1000);
    }
    fprintf(stderr, "I2C read block data at reg 0x%02X failed: %s\n", reg, strerror(errno));
    return -1;
}

/**
 * @brief I2C write block data (direct I2C operation)
 * @param dev I2C device pointer
 * @param reg Register address
 * @param length Data length
 * @param values Data to write
 * @return Returns 0 on success, -1 on failure
 */
int i2c_write_i2c_block_data(I2CDevice* dev, unsigned char reg, unsigned char length, const unsigned char* values) 
{
    struct i2c_msg msgs[1];
    struct i2c_rdwr_ioctl_data msgset;
    unsigned char* buffer = (unsigned char*)malloc(length + 1);
    
    if (!buffer) {
        fprintf(stderr, "Memory allocation failed\n");
        return -1;
    }
    
    buffer[0] = reg;
    memcpy(buffer + 1, values, length);
    
    msgs[0].addr = dev->addr;
    msgs[0].flags = 0; // Write operation
    msgs[0].len = length + 1;
    msgs[0].buf = buffer;
    
    msgset.msgs = msgs;
    msgset.nmsgs = 1;
    
    int result = -1;
    for (int i = 0; i < I2C_MAX_RETRIES; i++) {
        if (ioctl(dev->fd, I2C_RDWR, &msgset) >= 0) {
            result = 0;
            break;
        }
        usleep(I2C_RETRY_DELAY_MS * 1000);
    }
    
    free(buffer);
    if (result < 0) {
        fprintf(stderr, "I2C write block data to reg 0x%02X failed: %s\n", reg, strerror(errno));
    }
    return result;
}
