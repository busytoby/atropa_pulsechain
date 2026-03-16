#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_manifest.h"
#include "lau_memory.h"

// Test the Gibbs Rule Hardening (Solving for PHI)

int main() {
    printf("[TEST] Hardened Gibbs Rule (Phi-Solver)...\n");

    // 1. Setup Manifold
    TSFiHelmholtzManifold manifold;
    manifold.count = 5;
    manifold.capacity = 16;
    manifold.centrifugal_force = 1; // Active Spike-Guard
    
    manifold.lattice = (TSFiPhiAtom*)lau_memalign(64, manifold.capacity * sizeof(TSFiPhiAtom));
    memset(manifold.lattice, 0, manifold.capacity * sizeof(TSFiPhiAtom));

    // Define consistent Secret
    uint32_t secret = 0xABC;

    // Atom 0: Target Well
    for(int i=0; i<16; i++) manifold.lattice[0].state_vector[i] = 0x11111111;
    manifold.lattice[0].secret_provenance = secret;
    manifold.lattice[0].state_vector[15] = (manifold.lattice[0].state_vector[15] & ~TSFI_SECRET_MASK) | secret;

    // Atom 1: Dirichlet Spike (Identical to Atom 0)
    for(int i=0; i<16; i++) manifold.lattice[1].state_vector[i] = 0x11111111;
    manifold.lattice[1].secret_provenance = secret;
    manifold.lattice[1].state_vector[15] = (manifold.lattice[1].state_vector[15] & ~TSFI_SECRET_MASK) | secret;

    // Atom 2: Near match
    for(int i=0; i<16; i++) manifold.lattice[2].state_vector[i] = 0x11111111;
    manifold.lattice[2].state_vector[0] ^= 0xFFFF; // High distance
    manifold.lattice[2].secret_provenance = secret;

    // 3. Test Case A: Single Target (No Spike)
    printf("  [CASE A] Single Target Resolution...\n");
    manifold.count = 1; // Only atom 0
    uint32_t mu[16];
    memcpy(mu, manifold.lattice[0].state_vector, 64);
    
    uint32_t tension = 0;
    int32_t res = tsfi_phi_resolve(mu, &manifold, &tension);
    if (res == 0) {
        printf("    PASS: Resolved uniquely to Atom 0.\n");
    } else {
        printf("    FAIL: Expected 0, got %d\n", res);
        return 1;
    }

    // 4. Test Case B: Dirichlet Spike Detection
    printf("  [CASE B] Dirichlet Spike Denial...\n");
    manifold.count = 2; // Atom 0 and 1 are identical
    res = tsfi_phi_resolve(mu, &manifold, &tension);
    if (res == -2) {
        printf("    PASS: Spike Detected and Blocked (Z-Logic Denial).\n");
    } else {
        printf("    FAIL: Allowed manifold collapse (res=%d).\n", res);
        return 1;
    }

    // 5. Test Case C: Crossover Utility
    printf("  [CASE C] Crossover Utility...\n");
    TSFiPhiAtom child;
    tsfi_phi_crossover(&manifold.lattice[0], &manifold.lattice[2], &child);
    if (child.secret_provenance & 0x80000000) {
        printf("    PASS: Child correctly flagged as evolved.\n");
    } else {
        printf("    FAIL: Inheritance failure.\n");
        return 1;
    }

    lau_free(manifold.lattice);
    printf("[TEST] Gibbs Hardening Complete.\n");
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
