// src/core/logger.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <pthread.h>
#include "logger.h"

static FILE *log_file = NULL;
static LogLevel log_level = LOG_LEVEL_INFO;
static int log_console = 1;
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * @brief Initialize logging system
 * @param filename Log file name (NULL means no file output)
 * @param level Log level
 * @param console Whether to output to console (1:yes, 0:no)
 */
void logger_init(const char *filename, LogLevel level, int console) {
    log_level = level;
    log_console = console;

    if (filename != NULL) {
        log_file = fopen(filename, "a");
        if (log_file == NULL) {
            fprintf(stderr, "Failed to open log file: %s\n", filename);
            exit(EXIT_FAILURE);
        }
    }
}

/**
 * @brief Log message
 * @param level Log level
 * @param format Format string
 * @param ... Variable argument list
 */
void logger_log(LogLevel level, const char *format, ...) {
    if (level < log_level) {
        return;
    }

    pthread_mutex_lock(&log_mutex);

    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char time_str[20];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);

    const char *level_str;
    switch (level) {
        case LOG_LEVEL_DEBUG: level_str = "DEBUG"; break;
        case LOG_LEVEL_INFO:  level_str = "INFO";  break;
        case LOG_LEVEL_WARN:  level_str = "WARN";  break;
        case LOG_LEVEL_ERROR: level_str = "ERROR"; break;
        case LOG_LEVEL_FATAL: level_str = "FATAL"; break;
        default:              level_str = "UNKNOWN";
    }

    va_list args;
    va_start(args, format);

    if (log_console) {
        fprintf(stderr, "[%s] [%s] ", time_str, level_str);
        vfprintf(stderr, format, args);
        fprintf(stderr, "\n");
    }

    if (log_file != NULL) {
        fprintf(log_file, "[%s] [%s] ", time_str, level_str);
        vfprintf(log_file, format, args);
        fprintf(log_file, "\n");
        fflush(log_file);
    }

    va_end(args);
    pthread_mutex_unlock(&log_mutex);
}
