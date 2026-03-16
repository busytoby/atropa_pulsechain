#ifndef TSFI_PLUGIN_H
#define TSFI_PLUGIN_H

#include <stdbool.h>

// The VTable for hot-swappable logic
typedef struct {
    void (*logic_epoch)(int*);
    bool (*logic_state)(void*);
    void (*logic_directive)(int*, char*);
    void (*logic_scramble)(void*);
    void (*logic_provenance)(void*);
    void (*logic_hilbert)(void*, float, float, float*);
    void (*logic_hilbert_batch)(void*, const float*, float*, int);
    void (*logic_evolve)(void*, float);
} TSFiLogicTable;

#endif // TSFI_PLUGIN_H