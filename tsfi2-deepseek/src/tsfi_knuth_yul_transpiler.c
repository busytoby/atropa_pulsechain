#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tsfi_knuth_yul_transpiler.h"
#include "tsfi_knuth_storage.h"

int tsfi_knuth_transpile_yul_sstore(
    const char *slot_a,
    const char *var_x,
    const char *slot_b,
    const char *var_y,
    tsfi_knuth_yul_transpile_result_t *out_res
) {
    if (!slot_a || !var_x || !slot_b || !var_y || !out_res) return -1;
    memset(out_res, 0, sizeof(tsfi_knuth_yul_transpile_result_t));

    // Format unoptimized Yul code
    snprintf(out_res->original_yul, sizeof(out_res->original_yul),
             "sstore(%s, %s)\nsstore(%s, %s)", slot_a, var_x, slot_b, var_y);

    // Format gas-optimized Knuth Base 2i Yul code
    snprintf(out_res->optimized_yul, sizeof(out_res->optimized_yul),
             "let packed_%s_%s := pack_base2i(%s, %s)\nsstore(%s, packed_%s_%s)",
             var_x, var_y, var_x, var_y, slot_a, var_x, var_y);

    out_res->gas_saved = tsfi_knuth_calculate_gas_savings(1); // 20,000 gas saved per pair

    printf("[KNUTH YUL TRANSPILER] Optimized Dual SSTORE -> Saved %u Gas on PulseChain!\nOriginal:\n%s\nOptimized:\n%s\n",
           out_res->gas_saved, out_res->original_yul, out_res->optimized_yul);

    return 0; // Transpilation complete
}
