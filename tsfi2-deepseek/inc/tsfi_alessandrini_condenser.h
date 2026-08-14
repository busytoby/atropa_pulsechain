#ifndef TSFI_ALESSANDRINI_CONDENSER_H
#define TSFI_ALESSANDRINI_CONDENSER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Alessandrini Condenser Field State Struct
typedef struct {
    float capacity_potential;         // \mathcal{C}(u): Total electrostatic capacity
    float constant_flux_value;        // c: Constant boundary flux |\nabla u|
    float flux_variance;             // Variance of boundary flux across candidate slice
    bool  is_concentric_symmetric;    // Alessandrini symmetry certificate flag
} tsfi_alessandrini_condenser_t;

// Evaluates Alessandrini Condenser Capacity Potential over activation vector x
bool tsfi_alessandrini_eval_capacity(
    const float *x,
    int dim,
    tsfi_alessandrini_condenser_t *condenser_out
);

// Alessandrini Constant-Flux Gate: Prunes candidates violating flux symmetry
bool tsfi_alessandrini_constant_flux_gate(
    const float *x,
    int dim,
    float max_flux_var
);

// Method of Moving Planes Hyperplane Reflection Test
bool tsfi_alessandrini_moving_planes_reflection(
    const float *x,
    int dim,
    float lambda_plane,
    float *reflection_diff_out
);

#endif // TSFI_ALESSANDRINI_CONDENSER_H
