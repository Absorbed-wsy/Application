// src/util/cmdline.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "cmdline.h"

#include "adc.h"
#include "gpio.h"

/**
 * @brief Print command line usage instructions
 * @param cmdline Command line arguments array
 */
static void print_usage(char **cmdline)
{
    printf("\nUsage:\n");
    printf("%s adc <adc channel>\n", cmdline[0]);
    printf("%s gpio <gpiochip path> <gpio number> <direction: in/out> [value: 0/1]\n", cmdline[0]);
    printf("\n");
}

/**
 * @brief Command line argument parser
 * @param cmdline Command line arguments array
 * @return Returns program execution mode (1: main loop mode, 0: command mode)
 */
int command_parsing(char **cmdline)
{
    const char *cmd = cmdline[1];

    if(!strcmp(cmd, "loop")) {
        return 1;

    } else if(!strcmp(cmd, "adc")) {
        int chn = atoi(cmdline[2]);
        printf("IN%d Voltage: %.6f V\n", chn, get_adc_value(chn));

    } else if(!strcmp(cmd, "gpio")) {
        const char *gpiochip_path = cmdline[2];
        int gpio = atoi(cmdline[3]);
        const char *direction = cmdline[4];
        int value = (cmdline[5] != NULL) ? atoi(cmdline[5]) : 0;
        gpio_control(gpiochip_path, gpio, direction, value);

    } else if(!strcmp(cmd, "i2c")) {

    } else if(!strcmp(cmd, "spi")) {

    } else if(!strcmp(cmd, "uart")) {

    } else if(!strcmp(cmd, "pwm")) {

    } else if(!strcmp(cmd, "capture")) {

    } else {
        print_usage(cmdline);
    }

    return 0;
}
