#ifndef TSFI_LOG_H
#define TSFI_LOG_H

#include <stdarg.h>

typedef enum {
    LOG_SILENT = 0,
    LOG_ERROR = 1,
    LOG_WARN = 2,
    LOG_INFO = 3,
    LOG_DEBUG = 4,
    LOG_TRACE = 5
} LogLevel;

void tsfi_set_log_level(LogLevel level);
LogLevel tsfi_get_log_level(void);

void tsfi_log(LogLevel level, const char *fmt, ...);
void tsfi_log_rate(LogLevel level, int *counter, int rate, const char *fmt, ...);

#endif