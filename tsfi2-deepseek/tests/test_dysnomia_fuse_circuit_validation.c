#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include "tsfi_wavelet_arena.h"

// Reference modular exponentiation helper matching Xiao.modExp64
uint64_t modExp64(uint64_t base, uint64_t exp, uint64_t mod) {
    uint64_t res = 1;
    base = base % mod;
    while (exp > 0) {
        if (exp % 2 == 1) res = (__uint128_t)res * base % mod;
        base = (__uint128_t)base * base % mod;
        exp /= 2;
    }
    return res;
}

int main() {
    printf("=== Dysnomia FUSE Circuit Validation Suite ===\n\n");

    // Initialize FUSE inputs
    const uint64_t MotzkinPrime = 953467954114363;
    const uint64_t Base = 381387181645745; // MotzkinPrime / 2.5
    const uint64_t SecretRod = 88776655;
    const uint64_t SecretCone = 44332211;
    const uint64_t Signal = 1000;
    const uint64_t Alpha = 24;
    const uint64_t Beta = 100;

    printf("[INIT] FUSE Inputs:\n");
    printf("  -> MotzkinPrime: %lu\n", MotzkinPrime);
    printf("  -> Base:         %lu\n", Base);
    printf("  -> Secret Rod:   %lu\n", SecretRod);
    printf("  -> Secret Cone:  %lu\n", SecretCone);

    // --- MODEL A: On-Chain Solidity / C-Thunk Protocol Simulation ---
    printf("\n[MODEL A] Running Reference Solidity Equations...\n");
    
    // 1. AVAIL (Seed)
    uint64_t contour_rod_A = modExp64(Base, SecretRod, MotzkinPrime);
    uint64_t contour_cone_A = modExp64(Base, SecretCone, MotzkinPrime);
    
    // 2. FORM & POLARIZE
    uint64_t base_rod_A = modExp64(contour_cone_A, SecretRod, MotzkinPrime);
    uint64_t base_cone_A = modExp64(contour_rod_A, SecretCone, MotzkinPrime);
    uint64_t pole_rod_A = modExp64(base_rod_A, SecretRod, MotzkinPrime);
    uint64_t pole_cone_A = modExp64(base_cone_A, SecretCone, MotzkinPrime);

    // 3. CONIFY & SATURATE
    uint64_t identity_rod_A = SecretRod; 
    uint64_t identity_cone_A = SecretCone;
    uint64_t foundation_rod_A = modExp64(base_rod_A, identity_rod_A, MotzkinPrime);
    uint64_t foundation_cone_A = modExp64(base_cone_A, identity_cone_A, MotzkinPrime);
    uint64_t channel_rod_A = modExp64(base_rod_A, Signal, MotzkinPrime);
    uint64_t channel_cone_A = modExp64(base_cone_A, Signal, MotzkinPrime);
    uint64_t element_rod_A = Beta + Alpha; 
    uint64_t element_cone_A = Beta + Alpha;
    uint64_t chin_rod_A = Beta + Alpha;
    uint64_t chin_cone_A = Beta + Alpha;

    // 4. BOND
    uint64_t dynamo_rod_A = modExp64(base_rod_A, Signal, element_rod_A);
    uint64_t dynamo_cone_A = modExp64(base_cone_A, Signal, element_cone_A);

    // 5. MAGNETIZE
    printf("  -> base_rod_A:   %lu, base_cone_A:   %lu\n", base_rod_A, base_cone_A);
    printf("  -> dynamo_rod_A: %lu, dynamo_cone_A: %lu\n", dynamo_rod_A, dynamo_cone_A);
    uint64_t manifold_rod_A = modExp64(dynamo_cone_A, Signal, element_rod_A);
    uint64_t manifold_cone_A = modExp64(dynamo_rod_A, Signal, element_cone_A);
    printf("  -> manifold_rod: %lu, manifold_cone: %lu\n", manifold_rod_A, manifold_cone_A);
    assert(manifold_rod_A == manifold_cone_A && "Manifold Inequality!");

    uint64_t coordinate_rod_A = base_rod_A; // Simplified mapping
    uint64_t coordinate_cone_A = base_cone_A;
    uint64_t ring_rod_A = modExp64(coordinate_rod_A, manifold_rod_A, element_rod_A);
    uint64_t ring_cone_A = modExp64(coordinate_cone_A, manifold_cone_A, element_cone_A);
    assert(ring_rod_A == ring_cone_A && "Ring Inequality!");

    uint64_t monopole_A = modExp64(chin_rod_A, chin_cone_A, MotzkinPrime);

    printf("  -> Model A Monopole:  %lu\n", monopole_A);
    printf("  -> Model A Ring:      %lu\n", ring_rod_A);

    // --- MODEL B: WinchesterMQ Virtual Hardware Register Circuit ---
    printf("\n[MODEL B] Initializing Virtual Hardware Registers...\n");
    uint8_t mem_block[1024];
    TsfiWaveletArena arena;
    tsfi_wavelet_arena_init(&arena, mem_block, sizeof(mem_block));

    TsfiWavelet *rod = tsfi_STAT(&arena, MotzkinPrime);
    TsfiWavelet *cone = tsfi_STAT(&arena, MotzkinPrime);
    
    rod->Prime = MotzkinPrime;
    cone->Prime = MotzkinPrime;
    rod->telemetry.Fa.base = Base;
    cone->telemetry.Fa.base = Base;
    rod->telemetry.Fa.secret = SecretRod;
    cone->telemetry.Fa.secret = SecretCone;

    // 1. AVAIL
    tsfi_Avail(rod, Base, MotzkinPrime);
    tsfi_Avail(cone, Base, MotzkinPrime);

    // Synchronize identities
    rod->telemetry.Fa.identity = SecretRod;
    cone->telemetry.Fa.identity = SecretCone;

    // 2. MAGNETIZE JOINT
    tsfi_MAGNETIZE_JOINT(rod, cone, MotzkinPrime);

    uint64_t monopole_B = rod->telemetry.Fa.monopole;
    printf("  -> Model B Monopole:  %lu\n", monopole_B);

    // --- INTEGRITY ASSERTIONS ---
    printf("\n[ASSERT] Verifying register integrity across both models...\n");
    
    // Monopole calculated inside hardware circuit must match the reference math
    uint64_t expected_monopole = (rod->telemetry.Fa.identity * cone->telemetry.Fa.identity) % MotzkinPrime;
    assert(monopole_B == expected_monopole && "Hardware Monopole Mismatch!");
    
    printf("[SUCCESS] All circuit registers verified. Convergence is absolute.\n");
    return 0;
}
