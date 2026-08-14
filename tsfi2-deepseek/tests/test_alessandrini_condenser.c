#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "tsfi_alessandrini_condenser.h"

int main(void) {
    printf("================ TEST GIOVANNI ALESSANDRINI CONDENSER SYMMETRY ================\n");

    float x_symmetric[16];
    for (int i = 0; i < 8; i++) {
        x_symmetric[i] = (float)(i + 1) * 0.5f;
        x_symmetric[15 - i] = x_symmetric[i]; // Symmetric reflection plane
    }

    tsfi_alessandrini_condenser_t cond;
    bool ok_cap = tsfi_alessandrini_eval_capacity(x_symmetric, 16, &cond);
    assert(ok_cap);
    printf("  [PASS] Condenser Capacity Potential evaluated (Cap = %.4f, FluxVar = %.4f).\n", cond.capacity_potential, cond.flux_variance);
    assert(cond.is_concentric_symmetric);

    // Constant-Flux Gate Test
    bool ok_gate = tsfi_alessandrini_constant_flux_gate(x_symmetric, 16, 0.50f);
    assert(ok_gate);
    printf("  [PASS] Alessandrini Constant-Flux Gate verified.\n");

    // Method of Moving Planes Hyperplane Reflection Test
    float refl_diff = 0.0f;
    bool ok_refl = tsfi_alessandrini_moving_planes_reflection(x_symmetric, 16, 0.0f, &refl_diff);
    assert(ok_refl);
    assert(refl_diff < 1e-4f);
    printf("  [PASS] Moving Planes Hyperplane Reflection Test verified (ReflDiff = %.6f).\n", refl_diff);

    printf("=================================================================================\n");
    return 0;
}
