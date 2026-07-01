#include "tsfi_dysnomia_circuit_simulation.h"
#include <stdio.h>

// Internal 64-bit modular exponentiation helper matching Xiao.modExp64
static uint64_t circuit_mod_exp(uint64_t base, uint64_t exp, uint64_t mod) {
    uint64_t res = 1;
    base = base % mod;
    while (exp > 0) {
        if (exp % 2 == 1) res = (__uint128_t)res * base % mod;
        base = (__uint128_t)base * base % mod;
        exp /= 2;
    }
    return res;
}

void tsfi_circuit_init(DysnomiaLockCircuit *circuit) {
    if (!circuit) return;
    
    // Standard known FUSE values of the LAU
    circuit->prime = 953467954114363; // MotzkinPrime
    circuit->base = 381387181645745;  // MotzkinPrime / 2.5
    circuit->secret_rod = 88776655;
    circuit->secret_cone = 44332211;
    circuit->signal = 1000;
    circuit->alpha = 24;
    circuit->beta = 100;

    circuit->acc_npn_latched = 0;
    circuit->acc_pnp_latched = 0;
    circuit->is_locked = false;

    circuit->manifold = 0;
    circuit->monopole = 0;
    circuit->ring = 0;
    circuit->barn = 0;
}

void tsfi_circuit_lock(DysnomiaLockCircuit *circuit, uint64_t npn_input, uint64_t pnp_input) {
    if (!circuit) return;
    circuit->acc_npn_latched = npn_input;
    circuit->acc_pnp_latched = pnp_input;
    circuit->is_locked = true;
}

void tsfi_circuit_unlock(DysnomiaLockCircuit *circuit) {
    if (!circuit) return;
    circuit->is_locked = false;
}

bool tsfi_circuit_execute_cascade(DysnomiaLockCircuit *circuit) {
    if (!circuit) return false;

    // Use latched/secondary accumulator values to modulate the base parameters if locked
    uint64_t modulated_base = circuit->base;
    if (circuit->is_locked) {
        modulated_base = (circuit->base + circuit->acc_npn_latched - circuit->acc_pnp_latched) % circuit->prime;
    }

    // 1. AVAIL Step (Initialize contour)
    uint64_t contour_rod = circuit_mod_exp(modulated_base, circuit->secret_rod, circuit->prime);
    uint64_t contour_cone = circuit_mod_exp(modulated_base, circuit->secret_cone, circuit->prime);

    // 2. FORM & POLARIZE Steps (Transitive Coupling)
    uint64_t base_rod = circuit_mod_exp(contour_cone, circuit->secret_rod, circuit->prime);
    uint64_t base_cone = circuit_mod_exp(contour_rod, circuit->secret_cone, circuit->prime);

    // 3. CONIFY & SATURATE Steps (Impedance modulations)
    uint64_t element_rod = circuit->beta + circuit->alpha;
    uint64_t element_cone = circuit->beta + circuit->alpha;
    uint64_t chin_rod = circuit->beta + circuit->alpha;
    uint64_t chin_cone = circuit->beta + circuit->alpha;

    // 4. BOND Step (Seal Dynamos)
    uint64_t dynamo_rod = circuit_mod_exp(base_rod, circuit->signal, element_rod);
    uint64_t dynamo_cone = circuit_mod_exp(base_cone, circuit->signal, element_cone);

    // 5. MAGNETIZE Step (Swap dynamos and verify convergence)
    uint64_t manifold_rod = circuit_mod_exp(dynamo_cone, circuit->signal, element_rod);
    uint64_t manifold_cone = circuit_mod_exp(dynamo_rod, circuit->signal, element_cone);

    if (manifold_rod != manifold_cone) {
        // Manifold Inequality
        return false;
    }

    uint64_t coordinate_rod = base_rod;
    uint64_t coordinate_cone = base_cone;
    uint64_t ring_rod = circuit_mod_exp(coordinate_rod, manifold_rod, element_rod);
    uint64_t ring_cone = circuit_mod_exp(coordinate_cone, manifold_cone, element_cone);

    if (ring_rod != ring_cone) {
        // Ring Inequality
        return false;
    }

    // Seal output registers in circuit
    circuit->manifold = manifold_rod;
    circuit->ring = ring_rod;
    circuit->barn = circuit_mod_exp(ring_rod, manifold_rod, element_rod);
    circuit->monopole = circuit_mod_exp(chin_rod, chin_cone, circuit->prime);

    return true;
}
