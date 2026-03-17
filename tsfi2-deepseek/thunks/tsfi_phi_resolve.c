#include <immintrin.h>
#include <limits.h>
#include "tsfi_manifest.h"

// --- The Gibbs Rule: Solver for PHI ---
// Implements Energetic Inevitability. Denies the Gaussian Mean.

__attribute__((noinline))
int32_t tsfi_phi_resolve(const uint32_t* mu_vector, const TSFiHelmholtzManifold* manifold, uint32_t* out_manifold_tension) {
    if (!mu_vector || !manifold || !manifold->lattice) return -1;

    __m512i v_mu = _mm512_loadu_si512((const void*)mu_vector);
    uint32_t mu_secret = mu_vector[15] & TSFI_SECRET_MASK;

    int32_t resolved_index = -1;
    uint32_t min_phi = UINT_MAX;
    uint32_t total_tension = 0;
    
    // We count how many atoms are "too close" to detect Dirichlet Spikes
    uint32_t spike_count = 0;

    for (size_t i = 0; i < manifold->count; i++) {
        const TSFiPhiAtom* atom = &manifold->lattice[i];
        
        // 1. The High-Impedance Secret Handshake
        if (mu_secret != (atom->secret_provenance & TSFI_SECRET_MASK)) {
            continue; 
        }

        // 2. Algebraic Resonance (XOR)
        __m512i v_atom = _mm512_loadu_si512((const void*)atom->state_vector);
        __m512i v_xor = _mm512_xor_si512(v_mu, v_atom);
        
        // Measure Hamming Distance (Lanes differing)
        uint32_t diff_mask = _mm512_test_epi32_mask(v_xor, v_xor);
        uint32_t current_phi = _mm_popcnt_u32(diff_mask);
        
        total_tension += current_phi;

        // 3. Spike-Guard: Deny Convergence
        // If we find multiple nodes with near-zero potential, the manifold is collapsing.
        if (current_phi <= TSFI_SPIKE_THRESHOLD) {
            spike_count++;
        }

        // 4. Inevitability Selection
        if (current_phi < min_phi) {
            min_phi = current_phi;
            resolved_index = (int32_t)i;
        }
        
        // Update local PHI for telemetry
        ((TSFiPhiAtom*)atom)->local_phi = current_phi;
    }

    // 5. Hardening: Final Resolution Logic
    *out_manifold_tension = total_tension;

    // --- CASE: Dirichlet Spike Detected ---
    // If we have more than one point of perfect resonance, it's a "Bad Manifestation"
    if (spike_count > 1 && manifold->centrifugal_force > 0) {
        // Manifold is too uniform. Denial of Convergence.
        return -2; // Special status: Spike Detected (Requires Mutation)
    }

    // --- CASE: Dielectric Breakdown ---
    if (min_phi > TSFI_REJECTION_THRESHOLD) {
        return -1; // No inevitable path found.
    }

    // --- CASE: The Clutch Locks ---
    return resolved_index;
}

void tsfi_phi_crossover(const TSFiPhiAtom* parent_a, const TSFiPhiAtom* parent_b, TSFiPhiAtom* child_out) {
    if (!parent_a || !parent_b || !child_out) return;

    __m512i v_a = _mm512_loadu_si512((const void*)parent_a->state_vector);
    __m512i v_b = _mm512_loadu_si512((const void*)parent_b->state_vector);

    // Recombination: XOR + Circular Lane Shift
    // Ensures child is bijectively unique from parents
    __m512i v_child = _mm512_xor_si512(v_a, v_b);
    v_child = _mm512_rol_epi32(v_child, 3); // Shift by 3 to diversify

    _mm512_storeu_si512((void*)child_out->state_vector, v_child);
    child_out->secret_provenance = parent_a->secret_provenance | 0x80000000; // Flag as evolved
    
    // Embed secret into the state vector itself (Last 32 bits)
    // This ensures the wavefront resolution finds it.
    child_out->state_vector[15] = (child_out->state_vector[15] & ~TSFI_SECRET_MASK) | (child_out->secret_provenance & TSFI_SECRET_MASK);

    child_out->local_phi = 0;
    child_out->svdag_node_id = 0;
}
