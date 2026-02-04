#include "tsfi_log.h"
#include <stdio.h>
#include <stdarg.h>

static LogLevel g_log_level = LOG_INFO;

void tsfi_set_log_level(LogLevel level) {
    g_log_level = level;
}

LogLevel tsfi_get_log_level(void) {
    return g_log_level;
}

void tsfi_log(LogLevel level, const char *fmt, ...) {
    if (level > g_log_level) return;

    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    // Auto-flush for debugging, though might hurt perf. Keeping it for CLI responsiveness.
    if (level <= LOG_INFO) fflush(stdout);
}

void tsfi_log_rate(LogLevel level, int *counter, int rate, const char *fmt, ...) {
    if (level > g_log_level) return;
    
    if (*counter % rate == 0) {
        va_list args;
        va_start(args, fmt);
        vprintf(fmt, args);
        va_end(args);
        if (level <= LOG_INFO) fflush(stdout);
    }
    (*counter)++;
}