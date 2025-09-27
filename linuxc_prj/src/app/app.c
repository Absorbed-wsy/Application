// src/app/config.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "../core/logger.h"
#include "../algo/pid.h"
#include "../peripherals/adc.h"
#include "../modules/mcp4725/mcp4725.h"

unsigned long long get_current_time_us(void) 
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (unsigned long long)ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
}

int app_init(void)
{
    LOG_INFO("App Init");
    return 0;
}

int app_loop(void)
{
    //LOG_INFO("App Loop");
    return 0;
}

int main_loop(void)
{
    //LOG_INFO("Main Loop");
    return 0;
}