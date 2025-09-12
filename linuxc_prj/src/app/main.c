// src/app/main.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>

#include "../core/logger.h"
#include "../core/config.h"
#include "../core/process_pool.h"
#include "../core/thread_pool.h"

void PrivateTask(void *arg);


void print_helper(char **cmdline)
{
    printf("\nUsage:\n");
    printf("%s gpio <gpiochip path> <gpio number> <direction: in/out> [<value: 0/1>]\n", cmdline[0]);
}

int config_initialize(const char* filename)
{
    Config* conf = config_create();
    config_load(conf, filename);

    //log level
    int debug = config_get_int(conf, "Logging", "level", LOG_LEVEL_INFO);
    logger_init(NULL, debug, 1);
    LOG_DEBUG("Main debug level = %d",debug);

    //other config

    config_free(conf);

    return 0;
}

int command_parsing(char **cmdline)
{
    const char *cmd = cmdline[1];

    if(!strcmp(cmd, "loop")) {
        return 1;
    } else if(!strcmp(cmd, "gpio")) {

    } else {
        print_helper(cmdline);
    }

    return 0;
}

int main(int argc, char *argv[]) 
{
    int loop_status=0;
    
    //log init
    logger_init(NULL, LOG_LEVEL_INFO, 1);
    LOG_INFO("Program started");

    //config init
    config_initialize("app.conf");

    //command parsing
    if(argc > 1)
        loop_status = command_parsing(argv);

    //process thread task
    LOG_INFO("Making threadpool with 4 threads");
    threadpool thpool = thpool_init(4);

    LOG_INFO("Adding 4 tasks to threadpool");
	for (int i=0; i<4; i++){
		thpool_add_work(thpool, PrivateTask, (void*)(uintptr_t)i);
	};

    while (loop_status) {
        LOG_INFO("main loop");
        sleep(1);
    }

    LOG_INFO("Killing threadpool");
    thpool_wait(thpool);
	thpool_destroy(thpool);

    LOG_INFO("Program over");
    
    exit(0);
}

void PrivateTask(void* arg) {
    int value = (int)(uintptr_t)arg;
    for(;;) {
        printf("Thread #%lu working on %d\n", (unsigned long)pthread_self(), value);
        sleep(1);
    }
}
