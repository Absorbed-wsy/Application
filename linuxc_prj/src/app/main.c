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
#include "../net/udp_server_client.h"
#include "../net/tcp_server_client.h"


typedef struct main_config {
    int loop;
    int debug;
    int nthread;
} Aconf;

void PrivateTask(void *arg);


void print_helper(char **cmdline)
{
    printf("\nUsage:\n");
    printf("%s gpio <gpiochip path> <gpio number> <direction: in/out> [<value: 0/1>]\n", cmdline[0]);
}

int config_initialize(const char* filename, struct main_config *app)
{
    Config* conf = config_create();
    config_load(conf, filename);

    //log level
    app->debug = config_get_int(conf, "Logging", "level", LOG_LEVEL_INFO);
    logger_init(NULL, app->debug, 1);
    LOG_DEBUG("Main debug level = %d",app->debug);

    //main loop enable
    app->loop = config_get_bool(conf, "Config", "main_loop", 1);
    LOG_DEBUG("Main loop %s",app->loop ? "Enable" : "Disable");

    //Number of application threads
    app->nthread = config_get_int(conf, "Thread", "nthread", 1);
    LOG_DEBUG("The number of application threads is %d",app->nthread);


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
    Aconf *config = malloc(sizeof(Aconf));
    
    //log init
    logger_init(NULL, LOG_LEVEL_INFO, 1);
    LOG_INFO("Program started");

    //config init
    config_initialize("app.conf", config);

    //command parsing
    if(argc > 1)
        config->loop = command_parsing(argv);

    //process thread task
    threadpool thpool = thpool_init(config->nthread);
	thpool_add_work(thpool, PrivateTask, NULL);

    while (config->loop) {
        //LOG_INFO("main loop");
        sleep(1);
    }

    thpool_wait(thpool);
	thpool_destroy(thpool);

    LOG_INFO("Program over");
    
    exit(0);
}

void PrivateTask(void* arg) {
    //int value = (int)(uintptr_t)arg;

    for(;;) {

        sleep(1);
    }
}
