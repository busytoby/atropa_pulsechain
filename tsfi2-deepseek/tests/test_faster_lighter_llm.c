#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "tsfi_faster_lighter_llm.h"

int main(void) {
    printf("================ TEST ARNAV CHAVAN ET AL. (2024) FASTER & LIGHTER LLMS ================\n");

    float k_cache[16];
    for (int i = 0; i < 16; i++) {
        k_cache[i] = (i % 2 == 0) ? (float)i * 0.10f : 0.01f;
    }

    float sparsity = 0.0f;
    bool ok_sparse = tsfi_faster_lighter_sparsify_kv(k_cache, 16, 0.05f, &sparsity);
    assert(ok_sparse);
    assert(sparsity > 0.30f);
    printf("  [PASS] KV-Cache Key Vector sparsification verified (Sparsity = %.2f%%).\n", sparsity * 100.0f);

    float x_curr[16], x_prev[16];
    for (int i = 0; i < 16; i++) {
        x_curr[i] = 1.0000f;
        x_prev[i] = 1.0001f;
    }
    bool should_exit = false;
    bool ok_exit = tsfi_faster_lighter_check_early_exit(x_curr, x_prev, 16, 18, 32, &should_exit);
    assert(ok_exit);
    assert(should_exit);
    printf("  [PASS] Layer-Skipping Early Exit condition verified (Exit Layer = 18/32).\n");

    printf("=======================================================================================\n");
    return 0;
}
