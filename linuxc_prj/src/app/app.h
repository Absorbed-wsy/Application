// src/app/app.h
#ifndef APP_H
#define APP_H

#include "logger.h"
#include "config.h"
#include "process_pool.h"
#include "thread_pool.h"
#include "cmdline.h"

#include "adc.h"
#include "gpio.h"
#include "i2c.h"
#include "uart.h"

#include "mcp4725/mcp4725.h"
#include "ssd1306/linux_oled.h"

#include "tcp_server_client.h"
#include "udp_server_client.h"

int main_init(int argc, char *argv[]);

#endif // APP_H
