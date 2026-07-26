#ifndef TSFI_POINTSSIZES_H
#define TSFI_POINTSSIZES_H

#include <stdint.h>
#include <stdbool.h>

#define TSFI_MAX_POINTS 128

typedef struct {
    double sizes[TSFI_MAX_POINTS];
    int count;
} TSFiPointsSizes;

// Initialize the PointsSizes context
void tsfi_pointssizes_init(TSFiPointsSizes *ps);

// Adds a size value to the point size array
bool tsfi_pointssizes_add(TSFiPointsSizes *ps, double size_val);

// Gets the point size at a specific index
double tsfi_pointssizes_get(const TSFiPointsSizes *ps, int index);

#endif // TSFI_POINTSSIZES_H
