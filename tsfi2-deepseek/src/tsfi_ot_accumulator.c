#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tsfi_ot_accumulator.h"

void tsfi_ot_accumulator_init(TSFiOTAccumulator *acc) {
    if (!acc) return;
    acc->count = 0;
    acc->cumulative_potential = 0.0f;
    memset(acc->entries, 0, sizeof(acc->entries));
}

int tsfi_ot_accumulator_add(TSFiOTAccumulator *acc, const char *coord, float weight) {
    if (!acc || !coord || acc->count >= MAX_ACCUMULATED_PATHS) return -1;

    // Insert entry
    strncpy(acc->entries[acc->count].coordinate, coord, 127);
    acc->entries[acc->count].path_weight = weight;
    acc->count++;

    // Accumulate potential (Sum of weights modeling boundary strain)
    acc->cumulative_potential += weight;
    return 0;
}

float tsfi_ot_accumulator_get_potential(const TSFiOTAccumulator *acc) {
    return acc ? acc->cumulative_potential : 0.0f;
}
