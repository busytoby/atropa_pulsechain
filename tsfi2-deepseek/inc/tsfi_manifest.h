#ifndef TSFI_PHI_RESOLVE_H
#define TSFI_PHI_RESOLVE_H

#include <stdint.h>
#include <stddef.h>
#include "lau_memory.h"
#include "tsfi_types.h"

// --- The Helmholtz Phi Lattice (Non-Gaussian) ---
// Z-Logic Energetic Inevitability: We solve for PHI, not PSI.
// PHI represents the local Helmholtz Potential (Impedance + Tension).

#define TSFI_PHI_DIM 512
#define TSFI_SECRET_MASK 0xFFF
#define TSFI_SPIKE_THRESHOLD 1  // Dirichlet Spike: Too many lanes matching (Convergence)
#define TSFI_REJECTION_THRESHOLD 4 // Dielectric Breakdown: Too little resonance

typedef struct {
    uint32_t state_vector[16];   // 512-bit vector
    uint32_t secret_provenance;  // 12-bit Secret + Flags
    uint32_t local_phi;          // Resolved potential (last iteration)
    uint64_t svdag_node_id;      // SVDAG presence pointer
} TSFiPhiAtom;

// Use DEFINE_MAPPED_STRUCT to enable full bijective wiring/thunk capabilities
DEFINE_MAPPED_STRUCT(TSFiHelmholtzManifold,
    TSFiPhiAtom* lattice;        // The Helm Tensor (Z) in ReBAR
    size_t count;
    size_t capacity;
    uint32_t centrifugal_force;  // Force used to deny Dirichlet spikes
    
    // Thunked operations for Gibbs resolution
    int32_t (*resolve_phi)(const uint32_t* mu_vector, const void* manifold, uint32_t* out_manifold_tension);
    void (*crossover)(const TSFiPhiAtom* a, const TSFiPhiAtom* b, TSFiPhiAtom* child);
)

// --- The Gibbs Resolution Thunk ("The Clutch") ---

// Legacy wrappers for compatibility if needed, but the thunked function pointers
// in the manifold struct should be used for full capability.
int32_t tsfi_phi_resolve(
    const uint32_t* mu_vector,
    const TSFiHelmholtzManifold* manifold,
    uint32_t* out_manifold_tension
);

void tsfi_phi_crossover(
    const TSFiPhiAtom* parent_a,
    const TSFiPhiAtom* parent_b,
    TSFiPhiAtom* child_out
);

#endif // TSFI_PHI_RESOLVE_H
