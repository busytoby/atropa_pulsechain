#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "tsfi_alessandrini_condenser.h"

bool tsfi_alessandrini_eval_capacity(
    const float *x,
    int dim,
    tsfi_alessandrini_condenser_t *condenser_out
) {
    if (!x || dim < 2 || !condenser_out) return false;

    // Compute electrostatic potential energy integral \int |\nabla x|^2
    float total_grad_sq = 0.0f;
    float sum_flux = 0.0f;
    for (int i = 0; i < dim - 1; i++) {
        float grad = x[i + 1] - x[i];
        float grad_sq = grad * grad;
        total_grad_sq += grad_sq;
        sum_flux += fabsf(grad);
    }

    float mean_flux = sum_flux / (float)(dim - 1);
    float flux_var_sum = 0.0f;
    for (int i = 0; i < dim - 1; i++) {
        float diff = fabsf(x[i + 1] - x[i]) - mean_flux;
        flux_var_sum += diff * diff;
    }
    float flux_var = flux_var_sum / (float)(dim - 1);

    condenser_out->capacity_potential = total_grad_sq / (float)dim;
    condenser_out->constant_flux_value = mean_flux;
    condenser_out->flux_variance = flux_var;
    condenser_out->is_concentric_symmetric = (flux_var < 0.25f);

    return true;
}

bool tsfi_alessandrini_constant_flux_gate(
    const float *x,
    int dim,
    float max_flux_var
) {
    if (!x || dim < 2) return false;
    tsfi_alessandrini_condenser_t cond;
    if (!tsfi_alessandrini_eval_capacity(x, dim, &cond)) return false;
    return (cond.flux_variance <= max_flux_var);
}

bool tsfi_alessandrini_moving_planes_reflection(
    const float *x,
    int dim,
    float lambda_plane,
    float *reflection_diff_out
) {
    if (!x || dim < 2) return false;
    (void)lambda_plane;

    // Alexandrov-Alessandrini Hyperplane Reflection Test across mid-plane
    float diff_sum = 0.0f;
    int half = dim / 2;
    for (int i = 0; i < half; i++) {
        float orig = x[i];
        float reflected = x[dim - 1 - i];
        float diff = fabsf(orig - reflected);
        diff_sum += diff * diff;
    }
    float mean_diff = sqrtf(diff_sum / (float)half);
    if (reflection_diff_out) *reflection_diff_out = mean_diff;
    return (mean_diff < 1.0f);
}
