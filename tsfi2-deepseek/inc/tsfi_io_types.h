#ifndef TSFI_IO_TYPES_H
#define TSFI_IO_TYPES_H

#include <stdio.h>

typedef enum {
    TSFI_LOG_INFO,
    TSFI_LOG_WARN,
    TSFI_LOG_FAIL,
    TSFI_LOG_ZHONG,
    TSFI_LOG_PHYSICAL,
    TSFI_LOG_MEMORY
} TSFiLogLevel;

// Log with automated timestamp, provenance, and tag.
void tsfi_io_log(FILE *stream, TSFiLogLevel level, const char *tag, const char *format, ...);

// Thread-safe binary write
size_t tsfi_io_write(FILE *stream, const void *ptr, size_t size);

#endif // TSFI_IO_TYPES_H
