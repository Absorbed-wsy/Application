// src/app/main.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "app.h"

/**
 * @brief 程序主入口函数
 * @param argc 命令行参数个数
 * @param argv 命令行参数数组
 * @return 程序退出状态码
 */
int main(int argc, char *argv[]) 
{
    main_init(argc, argv);
    exit(0);
}
