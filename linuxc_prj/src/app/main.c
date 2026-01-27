// src/app/main.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "app.h"

/**
 * @brief Program main entry function
 * @param argc Command line argument count
 * @param argv Command line argument array
 * @return Program exit status code
 */
int main(int argc, char *argv[]) 
{
    main_init(argc, argv);
    exit(0);
}
