// src/app/config.c
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

int main_loop(void);
void PrivateTask(void* arg);


unsigned long long get_current_time_us(void) 
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (unsigned long long)ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
}

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

int main_loop(void)
{
    for(;;) {

        sleep(1);
    }
    return 0;
}

void PrivateTask(void* arg)
{
    for(;;) {

        sleep(1);
    }
}