// src/app/main.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>

#include "app.h"
#include "../core/logger.h"
#include "../core/config.h"
#include "../core/process_pool.h"
#include "../core/thread_pool.h"
#include "../util/cmdline.h"


void PrivateTask(void *arg);


int main(int argc, char *argv[]) 
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
        sleep(1);
    }

    thpool_wait(thpool);
	thpool_destroy(thpool);

    LOG_INFO("Program over");
    
    exit(0);
}

void PrivateTask(void* arg) {
    //int value = (int)(uintptr_t)arg;
    app_init();
    for(;;) {
        app_loop();
        sleep(1);
    }
}
