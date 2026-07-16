#ifndef TSFI_OT_ACCUMULATOR_H
#define TSFI_OT_ACCUMULATOR_H

#include "tsfi_akb.h"

#define MAX_ACCUMULATED_PATHS 64

// OT Accumulated State Entry
typedef struct {
    char coordinate[128];
    float path_weight;
} TSFiOTEntry;

// The OT Accumulator Context
typedef struct {
    TSFiOTEntry entries[MAX_ACCUMULATED_PATHS];
    int count;
    float cumulative_potential;
} TSFiOTAccumulator;

// Initialize the OT Accumulator
void tsfi_ot_accumulator_init(TSFiOTAccumulator *acc);

// Accumulate an undecidable coordinate and its path weight
int tsfi_ot_accumulator_add(TSFiOTAccumulator *acc, const char *coord, float weight);

// Calculate global strain potential over all accumulated nodes
float tsfi_ot_accumulator_get_potential(const TSFiOTAccumulator *acc);

#endif // TSFI_OT_ACCUMULATOR_H
