// src/mosules/mcp4725/mcp4725.c
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>

#include "mcp4725.h"

int MCP4725_Init(const char *i2c_path)
{
    int fd = 0;
    if ((fd = open(i2c_path, O_RDWR)) < 0) {
        perror("I2C bus open failed");
        return -1;
    }

    if (ioctl(fd, I2C_SLAVE, MCP4725_ADDR) < 0) {
        perror("Failed to set I2C address");
        close(fd);
        return -1;
    }

    return fd;
}

void MCP4725_DeInit(int fd)
{
    if (fd >= 0) {
        close(fd);
        printf("I2C connection closed\n");
    }
}

void MCP4725_WriteData_Voltage(int fd, unsigned int Vout)
{
    struct i2c_msg msgs[1];
    struct i2c_rdwr_ioctl_data msgset;
    unsigned char buffer[2];
    
    buffer[0] = (((Vout*4096)/VREF_5V)&0x0f00) >> 8;
    buffer[1] = (((Vout*4096)/VREF_5V)&0x00ff);
    
    msgs[0].addr = MCP4725_ADDR;
    msgs[0].flags = 0;
    msgs[0].len = 3;
    msgs[0].buf = buffer;
    
    msgset.msgs = msgs;
    msgset.nmsgs = 1;
    
    ioctl(fd, I2C_RDWR, &msgset);
}
