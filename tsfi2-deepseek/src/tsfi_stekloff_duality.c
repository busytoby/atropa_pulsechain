#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "tsfi_stekloff_duality.h"

bool tsfi_stekloff_eval_eigenvalue_qing(
    const float *x,
    int dim,
    float qing_radius,
    tsfi_stekloff_state_t *stekloff_out
) {
    if (!x || dim < 2 || !stekloff_out) return false;

    float r_qing = (qing_radius > 1e-5f) ? qing_radius : 1.0f;
    stekloff_out->qing_reference_radius = r_qing;

    // Calculate normal derivative \nabla x \cdot n at Qing domain boundary relative to r_qing
    float border_val = fabsf(x[dim - 1]) + 1e-6f;
    float normal_flux = fabsf(x[dim - 1] - x[dim - 2]) / r_qing;

    float sigma = normal_flux / border_val;
    stekloff_out->stekloff_sigma = sigma;

    // Calculate sigma variance across vector
    float sum_var = 0.0f;
    for (int i = 0; i < dim - 1; i++) {
        float local_flux = fabsf(x[i + 1] - x[i]) / r_qing;
        float local_val = fabsf(x[i]) + 1e-6f;
        float diff = (local_flux / local_val) - sigma;
        sum_var += diff * diff;
    }
    float var_sigma = sum_var / (float)(dim - 1);
    stekloff_out->sigma_variance = var_sigma;
    stekloff_out->is_symmetric_stekloff_mode = (var_sigma < 0.50f);

    // Classify Stekloff mode relative to Qing reference radius
    if (sigma >= (1.0f / r_qing)) {
        stekloff_out->active_stekloff_mode = STEKLOFF_MODE_EPIBAR;
    } else {
        stekloff_out->active_stekloff_mode = STEKLOFF_MODE_HYPOBAR;
    }

    return true;
}

bool tsfi_stekloff_classify_mode_qing(
    const tsfi_stekloff_state_t *stekloff,
    uint32_t *mode_out
) {
    if (!stekloff) return false;
    if (mode_out) *mode_out = stekloff->active_stekloff_mode;
    return true;
}
