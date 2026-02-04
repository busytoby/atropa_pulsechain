#ifndef TSFI_PLUGIN_H
#define TSFI_PLUGIN_H

#include <stdbool.h>

// The VTable for hot-swappable logic
typedef struct {
    void (*logic_epoch)(int*);
    void (*logic_state)(bool*);
    void (*logic_directive)(int*, char*);
    void (*logic_scramble)(void*);
    void (*logic_provenance)(void*);
} TSFiLogicTable;

#endif // TSFI_PLUGIN_H