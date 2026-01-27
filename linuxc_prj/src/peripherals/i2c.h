// src/peripherals/i2c.h
#ifndef I2C_H
#define I2C_H

#define I2C_MAX_RETRIES 3
#define I2C_RETRY_DELAY_MS 10
#define I2C_BLOCK_MAX 32  // SMBus block operation maximum length

typedef struct {
    int fd;              // File descriptor
    unsigned char addr;  // Slave device address (7-bit)
} I2CDevice;

I2CDevice* i2c_init(const char* dev_path, unsigned char addr);
void i2c_close(I2CDevice* dev);

int i2c_smbus_read_byte_data(I2CDevice* dev, unsigned char reg);
int i2c_smbus_write_byte_data(I2CDevice* dev, unsigned char reg, unsigned char value);
int i2c_smbus_read_word_data(I2CDevice* dev, unsigned char reg);
int i2c_smbus_write_word_data(I2CDevice* dev, unsigned char reg, unsigned short value);
int i2c_read_i2c_block_data(I2CDevice* dev, unsigned char reg, unsigned char length, unsigned char* values);
int i2c_write_i2c_block_data(I2CDevice* dev, unsigned char reg, unsigned char length, const unsigned char* values);

#endif // I2C_H
