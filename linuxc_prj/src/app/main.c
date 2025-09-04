// src/app/main.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../core/logger.h"
#include "../core/config.h"


int config_initialize(const char* filename)
{
    Config* conf = config_create();
    config_load(conf, filename);

    //log level
    int debug = config_get_int(conf, "Logging", "level", LOG_LEVEL_INFO);
    logger_init(NULL, debug, 1);
    LOG_DEBUG("Main debug level = %d",debug);



    config_free(conf);

    return 0;
}


int main(int argc, char *argv[]) {
    
    //log init
    logger_init(NULL, LOG_LEVEL_INFO, 1);
    LOG_INFO("Program started");

    //config init
    config_initialize("app.conf");






    //while (1) {
    //    sleep(1);
    //}
    
    return 0;
}
