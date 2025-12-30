// src/app/app.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>

#include "app.h"
#include "logger.h"
#include "config.h"
#include "process_pool.h"
#include "thread_pool.h"
#include "cmdline.h"

#include "uart.h"
#include "ssd1306/linux_oled.h"


int main_loop(void);
void PrivateTask(void* arg);

/**
 * @brief 获取当前时间戳（微秒）
 * @return 当前时间戳（微秒）
 */
unsigned long long get_current_time_us(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (unsigned long long)ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
}

/**
 * @brief 应用主初始化函数
 * @param argc 命令行参数个数
 * @param argv 命令行参数数组
 * @return 程序执行状态 (0: 正常退出, 非0: 异常退出)
 */
int main_init(int argc, char *argv[])
{
    Aconf *config = malloc(sizeof(Aconf));
    
    //log init
    logger_init(NULL, LOG_LEVEL_INFO, 1);
    LOG_INFO("Program started");

    //config init
    config_initialize("app.conf", config);
    logger_init(NULL, config->debug, 1);

    //command parsing
    if(argc > 1) {
        config->loop = command_parsing(argv);
        if(config->loop == 0)
            return 0;
    }

    //process thread task
    threadpool thpool = thpool_init(config->nthread);
	thpool_add_work(thpool, PrivateTask, NULL);

    while (config->loop) {
        main_loop();
    }

    thpool_wait(thpool);
	thpool_destroy(thpool);

    LOG_INFO("Program over");

    return 0;
}

/**
 * @brief 主循环函数
 * @return 执行状态
 */
int main_loop(void)
{
    for(;;) {
        
        sleep(1);
    }
    return 0;
}

/**
 * @brief 私有任务处理函数
 * @param arg 任务参数（当前未使用）
 */
void PrivateTask(void* arg)
{
    for(;;) {

        sleep(1);
    }
}
