#include "tsfi_pointssizes.h"

void tsfi_pointssizes_init(TSFiPointsSizes *ps) {
    if (!ps) return;
    ps->count = 0;
    for (int i = 0; i < TSFI_MAX_POINTS; i++) {
        ps->sizes[i] = 1.0;
    }
}

bool tsfi_pointssizes_add(TSFiPointsSizes *ps, double size_val) {
    if (!ps || ps->count >= TSFI_MAX_POINTS) return false;
    if (size_val < 0.0) size_val = 0.0;
    ps->sizes[ps->count++] = size_val;
    return true;
}

double tsfi_pointssizes_get(const TSFiPointsSizes *ps, int index) {
    if (!ps || index < 0 || index >= ps->count) return 1.0;
    return ps->sizes[index];
}
