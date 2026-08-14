#ifndef TSFI_HELMHOLTZ_RING_H
#define TSFI_HELMHOLTZ_RING_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Helmholtz Thunk Function Signature: Operates over activation vector x
typedef bool (*tsfi_helmholtz_thunk_fn)(float *x, int dim, double resonance_k, void *user_ctx);

// Dynamic Helmholtz Array Object representing a Ring Domain \Omega
typedef struct {
    uint32_t ring_id;                     // Unique Ring Domain Identifier
    uint32_t count;                       // Total thunk functions in ring (N)
    uint32_t active_head;                 // Current SO(2) head index
    double   willms_radial_k2;            // N. B. Willms (1994) radial wavenumber k^2
    tsfi_helmholtz_thunk_fn *thunks;     // Array of N thunk function pointers
    uint32_t *execution_hashes;           // Execution integrity FNV-1a hashes
} tsfi_helmholtz_ring_domain_t;

// Creates a Dynamic Helmholtz Ring Domain Object with N thunk slots
tsfi_helmholtz_ring_domain_t* tsfi_helmholtz_ring_create(uint32_t ring_id, uint32_t count, double willms_k2);

// Registers a Thunk Function into a specific slot in the Ring Domain
bool tsfi_helmholtz_ring_register_thunk(tsfi_helmholtz_ring_domain_t *ring, uint32_t slot, tsfi_helmholtz_thunk_fn thunk_fn, uint32_t hash);

// Dispatches Thunk Functions sequentially or flexibly under N. B. Willms SO(2) Radial Symmetry
bool tsfi_helmholtz_ring_dispatch(tsfi_helmholtz_ring_domain_t *ring, float *x, int dim, bool sequential, void *user_ctx);

// N. B. Willms & Gladwell (1994) Bifurcation Auditor: Detects local symmetry-breaking points
bool tsfi_willms_bifurcation_audit(const float *x, int dim, float *bifurcation_metric_out);

// Willms Cauchy Boundary Pair Auditor: Validates inner hardware (Gamma_in) & outer syntax (Gamma_out) bounds
bool tsfi_willms_cauchy_boundary_check(const float *x, int dim, int grammar_state, uint32_t hardware_flags);

// Willms Serrin-Type Maximum Principle Hopf Boundary Normal Gradient Gate
bool tsfi_willms_hopf_boundary_gate(const float *x, int dim);

// Performs ACID Rollback of Ring Domain state rewind upon symmetry breakdown
bool tsfi_helmholtz_ring_acid_rollback(tsfi_helmholtz_ring_domain_t *ring, float *x, const float *x_checkpoint, int dim);

// Destroys and frees a Dynamic Helmholtz Ring Domain Object
void tsfi_helmholtz_ring_destroy(tsfi_helmholtz_ring_domain_t *ring);

#endif // TSFI_HELMHOLTZ_RING_H
