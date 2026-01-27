// src/app/app.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>

#include "app.h"


int main_loop(void);
void PrivateTask(void* arg);

/**
 * @brief Get current timestamp (microseconds)
 * @return Current timestamp (microseconds)
 */
unsigned long long get_current_time_us(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (unsigned long long)ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
}

/**
 * @brief Application main initialization function
 * @param argc Command line argument count
 * @param argv Command line argument array
 * @return Program execution status (0: normal exit, non-zero: abnormal exit)
 */
int main_init(int argc, char *argv[])
{
    Aconf *config = malloc(sizeof(Aconf));
    
    // Log initialization
    logger_init(NULL, LOG_LEVEL_INFO, 1);
    LOG_INFO("Program started");

    // Config initialization
    config_initialize("app.conf", config);
    logger_init(NULL, config->debug, 1);

    // Command parsing
    if(argc > 1) {
        config->loop = command_parsing(argv);
        if(config->loop == 0)
            return 0;
    }

    // Process thread task
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
 * @brief Main loop function
 * @return Execution status
 */
int main_loop(void)
{
    for(;;) {
        
        sleep(1);
    }
    return 0;
}

/**
 * @brief Private task processing function
 * @param arg Task parameter (currently unused)
 */
void PrivateTask(void* arg)
{
    for(;;) {

        sleep(1);
    }
}
