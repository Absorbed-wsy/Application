// src/modules/mcp4725/mcp4725.h
#ifndef MCP4725_H
#define MCP4725_H

#define MCP4725_ADDR    0x61
#define VREF_5V         4950

int MCP4725_Init(const char *i2c_path);
void MCP4725_DeInit(int fd);
void MCP4725_WriteData_Voltage(int fd, unsigned int Vout);

#endif // MCP4725_H
