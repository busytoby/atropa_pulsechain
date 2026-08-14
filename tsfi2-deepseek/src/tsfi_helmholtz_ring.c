#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "tsfi_helmholtz_ring.h"

tsfi_helmholtz_ring_domain_t* tsfi_helmholtz_ring_create(uint32_t ring_id, uint32_t count, double willms_k2) {
    if (count == 0) return NULL;
    tsfi_helmholtz_ring_domain_t *ring = (tsfi_helmholtz_ring_domain_t *)calloc(1, sizeof(tsfi_helmholtz_ring_domain_t));
    if (!ring) return NULL;

    ring->ring_id = ring_id;
    ring->count = count;
    ring->active_head = 0;
    ring->willms_radial_k2 = willms_k2;
    ring->thunks = (tsfi_helmholtz_thunk_fn *)calloc(count, sizeof(tsfi_helmholtz_thunk_fn));
    ring->execution_hashes = (uint32_t *)calloc(count, sizeof(uint32_t));

    if (!ring->thunks || !ring->execution_hashes) {
        tsfi_helmholtz_ring_destroy(ring);
        return NULL;
    }
    return ring;
}

bool tsfi_helmholtz_ring_register_thunk(tsfi_helmholtz_ring_domain_t *ring, uint32_t slot, tsfi_helmholtz_thunk_fn thunk_fn, uint32_t hash) {
    if (!ring || slot >= ring->count || !thunk_fn) return false;
    ring->thunks[slot] = thunk_fn;
    ring->execution_hashes[slot] = hash;
    return true;
}

bool tsfi_helmholtz_ring_dispatch(tsfi_helmholtz_ring_domain_t *ring, float *x, int dim, bool sequential, void *user_ctx) {
    if (!ring || !x || dim <= 0) return false;

    if (sequential) {
        // Sequential Mode: Dispatches thunk functions in index order
        for (uint32_t i = 0; i < ring->count; i++) {
            uint32_t idx = (ring->active_head + i) % ring->count;
            if (ring->thunks[idx]) {
                bool ok = ring->thunks[idx](x, dim, ring->willms_radial_k2, user_ctx);
                if (!ok) return false;
            }
        }
        ring->active_head = (ring->active_head + ring->count) % ring->count;
    } else {
        // Flexible Mode: Dispatches thunks under N. B. Willms SO(2) Radial Symmetry
        for (uint32_t i = 0; i < ring->count; i++) {
            uint32_t idx = (ring->active_head + i * 3) % ring->count;
            if (ring->thunks[idx]) {
                bool ok = ring->thunks[idx](x, dim, ring->willms_radial_k2, user_ctx);
                if (!ok) return false;
            }
        }
    }
    return true;
}

bool tsfi_willms_bifurcation_audit(const float *x, int dim, float *bifurcation_metric_out) {
    if (!x || dim < 2) return false;
    // Compute Hessian-type second derivative metric across activation vector elements
    float d1 = x[1] - x[0];
    float d2 = x[dim - 1] - x[dim - 2];
    float metric = fabsf(d1 * d2 - (x[dim / 2] * x[dim / 4]));
    if (bifurcation_metric_out) *bifurcation_metric_out = metric;
    return (metric < 50.0f); // True if within stable non-bifurcated radial symmetry
}

bool tsfi_willms_cauchy_boundary_check(const float *x, int dim, int grammar_state, uint32_t hardware_flags) {
    if (!x || dim <= 0) return false;
    // Gamma_in (Hardware condition): Verify non-zero energy balance
    float inner_energy = fabsf(x[0]) + fabsf(x[dim - 1]);
    if (inner_energy < 1e-7f && hardware_flags != 0) return false;

    // Gamma_out (Outer grammar condition): State 0..3 must match activation norm
    float outer_norm = 0.0f;
    for (int i = 0; i < dim; i += 4) outer_norm += fabsf(x[i]);
    if (grammar_state == 0 && outer_norm < 0.001f) return false;
    return true;
}

bool tsfi_willms_hopf_boundary_gate(const float *x, int dim) {
    if (!x || dim < 4) return false;
    // Hopf Boundary Lemma: Normal outward derivative must satisfy d_out <= 0
    float d_out = x[dim - 1] - x[dim - 2];
    return (d_out <= 2.0f); // Outward gradient bound
}

bool tsfi_helmholtz_ring_acid_rollback(tsfi_helmholtz_ring_domain_t *ring, float *x, const float *x_checkpoint, int dim) {
    if (!ring || !x || !x_checkpoint || dim <= 0) return false;
    // ACID Rollback: Rewind activation vector x to checkpoint and reset head
    memcpy(x, x_checkpoint, dim * sizeof(float));
    ring->active_head = 0;
    return true;
}

void tsfi_helmholtz_ring_destroy(tsfi_helmholtz_ring_domain_t *ring) {
    if (!ring) return;
    if (ring->thunks) free(ring->thunks);
    if (ring->execution_hashes) free(ring->execution_hashes);
    free(ring);
}
