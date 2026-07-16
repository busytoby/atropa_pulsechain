#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "tsfi_ot_accumulator.h"

int main(void) {
    printf("[OT Accumulator] Starting open question path weight accumulation tests...\n");
    fflush(stdout);

    TSFiOTAccumulator acc;
    tsfi_ot_accumulator_init(&acc);

    // 1. Accumulate coordinate paths
    int res = tsfi_ot_accumulator_add(&acc, "svdag/relation/question_1", 0.45f);
    assert(res == 0);
    assert(acc.count == 1);

    res = tsfi_ot_accumulator_add(&acc, "svdag/relation/question_2", 0.35f);
    assert(res == 0);
    assert(acc.count == 2);

    // 2. Verify cumulative potential calculations
    float potential = tsfi_ot_accumulator_get_potential(&acc);
    printf("  [Accumulate] Count: %d, Cumulative Potential: %.2f (Expected 0.80)\n", acc.count, potential);
    fflush(stdout);
    assert(fabs(potential - 0.80f) < 0.0001f);

    printf("[PASS] OT Accumulator verified successfully!\n");
    fflush(stdout);
    return 0;
}
