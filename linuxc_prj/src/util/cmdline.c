// src/util/cmdline.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "cmdline.h"


static void print_usage(char **cmdline)
{
    printf("\nUsage:\n");
    printf("%s gpio <gpiochip path> <gpio number> <direction: in/out> [<value: 0/1>]\n", cmdline[0]);
    printf("\n");
}

int command_parsing(char **cmdline)
{
    const char *cmd = cmdline[1];

    if(!strcmp(cmd, "loop")) {
        return 1;
    } else if(!strcmp(cmd, "gpio")) {

    } else {
        print_usage(cmdline);
    }

    return 0;
}