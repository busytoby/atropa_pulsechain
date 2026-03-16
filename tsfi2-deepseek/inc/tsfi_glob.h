#ifndef TSFI_GLOB_H
#define TSFI_GLOB_H

#include <stddef.h>
#include <stdbool.h>

typedef struct {
    char **paths;
    size_t count;
    size_t capacity;
} TSFiGlobResult;

// Recursively searches base_dir for files matching pattern.
TSFiGlobResult* tsfi_glob(const char *base_dir, const char *pattern);
void tsfi_glob_free(TSFiGlobResult *res);

#endif // TSFI_GLOB_H
