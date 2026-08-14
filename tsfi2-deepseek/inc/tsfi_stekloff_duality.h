#ifndef TSFI_STEKLOFF_DUALITY_H
#define TSFI_STEKLOFF_DUALITY_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Stekloff Mode Identifiers relative to Qing Domain Boundary
#define STEKLOFF_MODE_EPIBAR  0x45504942  // "EPIB" Epibar Boundary Flux Expansion
#define STEKLOFF_MODE_HYPOBAR 0x4859504F  // "HYPO" Hypobar Interior Energy Compression

// Stekloff Eigenvalue State Struct relative to Qing
typedef struct {
    float stekloff_sigma;              // \sigma: Stekloff eigenvalue ratio (\nabla x \cdot n) / x
    float sigma_variance;              // Variance of \sigma across boundary
    float qing_reference_radius;       // Qing domain reference radius r_qing
    uint32_t active_stekloff_mode;     // STEKLOFF_MODE_EPIBAR or STEKLOFF_MODE_HYPOBAR
    bool  is_symmetric_stekloff_mode;  // Alessandrini-Magnanini symmetry flag
} tsfi_stekloff_state_t;

// Evaluates Stekloff Eigenvalue Ratio over activation vector x relative to a Qing domain
bool tsfi_stekloff_eval_eigenvalue_qing(
    const float *x,
    int dim,
    float qing_radius,
    tsfi_stekloff_state_t *stekloff_out
);

// Stekloff Epibar / Hypobar Mode Classifier relative to Qing
bool tsfi_stekloff_classify_mode_qing(
    const tsfi_stekloff_state_t *stekloff,
    uint32_t *mode_out
);

#endif // TSFI_STEKLOFF_DUALITY_H
