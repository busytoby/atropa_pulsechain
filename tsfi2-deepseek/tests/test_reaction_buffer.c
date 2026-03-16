#include "tsfi_reaction.h"
#include "tsfi_io.h"
#include "tsfi_dys_math.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// Helper: Compute Hamming distance between two 64-bit integers
static int hamming_distance_u64(uint64_t a, uint64_t b) {
    uint64_t xor_val = a ^ b;
    int dist = 0;
    while (xor_val) {
        dist += xor_val & 1;
        xor_val >>= 1;
    }
    return dist;
}

// Helper: Compute Hamming weight (number of set bits)
static int hamming_weight_u64(uint64_t a) {
    return hamming_distance_u64(a, 0);
}

int main() {
    TSFiBigInt* Prime = tsfi_bn_alloc();
    tsfi_bn_set_u64(Prime, 953467954114363ULL); // Our standard 50-bit prime

    struct YI* yi = tsfi_reaction_shoot(Prime);
    TSFiBigInt* Pi = tsfi_bn_alloc();
    tsfi_bn_copy(Pi, yi->Ring ? yi->Ring : yi->Xi);

    // ReactSHIO gives us 1 Dai (2 TSFiBigInts: Ichidai, Daiichi)
    struct Dai* dai = tsfi_reaction_compute_reciprocity(yi->Psi, Pi, NULL);

    tsfi_io_printf(stdout, "1 Dai = 2 members (Ichidai, Daiichi)\n");
    tsfi_io_printf(stdout, "Theoretical Entropy per member: ~50 bits (based on prime)\n");
    tsfi_io_printf(stdout, "Memory allocation per member: %lu bytes\n", sizeof(uint64_t) * 8);

    // --- 1. The Strict Avalanche Criterion (SAC) Test ---
    tsfi_io_printf(stdout, "\n--- 1. Strict Avalanche Criterion (SAC) ---\n");
    TSFiBigInt* Pi_mutated = tsfi_bn_alloc();
    tsfi_bn_copy(Pi_mutated, Pi);
    Pi_mutated->limbs[0] ^= 1; // Flip exactly one bit
    struct Dai* dai_mutated = tsfi_reaction_compute_reciprocity(yi->Psi, Pi_mutated, NULL);
    
    int sac_ichidai = hamming_distance_u64(dai->Ichidai->limbs[0], dai_mutated->Ichidai->limbs[0]);
    int sac_daiichi = hamming_distance_u64(dai->Daiichi->limbs[0], dai_mutated->Daiichi->limbs[0]);
    tsfi_io_printf(stdout, "Single bit flip in Pi resulted in:\n");
    tsfi_io_printf(stdout, "  Ichidai flip count: %d bits\n", sac_ichidai);
    tsfi_io_printf(stdout, "  Daiichi flip count: %d bits\n", sac_daiichi);
    
    // Expecting roughly half of the ~50 active bits to flip (~25)
    assert(sac_ichidai > 10 && sac_ichidai < 40);
    assert(sac_daiichi > 10 && sac_daiichi < 40);
    
    freeDAI(dai_mutated);
    tsfi_bn_free(Pi_mutated);

    // --- 2. Shannon Entropy / Monobit Uniformity Test ---
    tsfi_io_printf(stdout, "\n--- 2. Monobit Uniformity Test ---\n");
    TSFiBigInt* run_pi = tsfi_bn_alloc();
    tsfi_bn_copy(run_pi, Pi);
    
    long total_bits_checked = 0;
    long total_bits_set = 0;
    for (int i = 0; i < 1000; i++) {
        struct Dai* d = tsfi_reaction_compute_reciprocity(yi->Psi, run_pi, NULL);
        // We only check the lowest 50 bits to match the Prime barrier
        total_bits_set += hamming_weight_u64(d->Ichidai->limbs[0] & 0x3FFFFFFFFFFFFULL);
        total_bits_set += hamming_weight_u64(d->Daiichi->limbs[0] & 0x3FFFFFFFFFFFFULL);
        total_bits_checked += 100; // 50 bits * 2 members
        
        tsfi_bn_copy(run_pi, d->Daiichi);
        freeDAI(d);
    }
    double ratio = (double)total_bits_set / total_bits_checked;
    tsfi_io_printf(stdout, "Ratio of 1s to 0s across 100,000 active bits: %f\n", ratio);
    // Expecting ratio very close to 0.5
    assert(ratio > 0.45 && ratio < 0.55);
    tsfi_bn_free(run_pi);

    // --- 3. Orthogonal Decorrelation ---
    tsfi_io_printf(stdout, "\n--- 3. Orthogonal Decorrelation ---\n");
    int orth_dist = hamming_distance_u64(dai->Ichidai->limbs[0], dai->Daiichi->limbs[0]);
    tsfi_io_printf(stdout, "Distance between paired Ichidai and Daiichi: %d bits\n", orth_dist);
    assert(orth_dist > 10 && orth_dist < 40);

    // --- 4. Floyd's Cycle Detection ---
    tsfi_io_printf(stdout, "\n--- 4. Cycle Detection (Tortoise and Hare) ---\n");
    TSFiBigInt* tortoise = tsfi_bn_alloc();
    TSFiBigInt* hare = tsfi_bn_alloc();
    tsfi_bn_copy(tortoise, Pi);
    tsfi_bn_copy(hare, Pi);
    
    int cycle_detected = 0;
    for (int step = 0; step < 1000; step++) {
        struct Dai* t_dai = tsfi_reaction_compute_reciprocity(yi->Psi, tortoise, NULL);
        tsfi_bn_copy(tortoise, t_dai->Daiichi);
        freeDAI(t_dai);
        
        struct Dai* h_dai1 = tsfi_reaction_compute_reciprocity(yi->Psi, hare, NULL);
        tsfi_bn_copy(hare, h_dai1->Daiichi);
        freeDAI(h_dai1);
        
        struct Dai* h_dai2 = tsfi_reaction_compute_reciprocity(yi->Psi, hare, NULL);
        tsfi_bn_copy(hare, h_dai2->Daiichi);
        freeDAI(h_dai2);
        
        if (tortoise->limbs[0] == hare->limbs[0]) {
            cycle_detected = 1;
            break;
        }
    }
    
    tsfi_io_printf(stdout, "Short-loop degenerate cycle detected: %s\n", cycle_detected ? "YES" : "NO");
    assert(cycle_detected == 0);
    
    tsfi_bn_free(tortoise);
    tsfi_bn_free(hare);

    freeDAI(dai);
    freeYI(yi);
    tsfi_bn_free(Pi);
    tsfi_bn_free(Prime);
    
    extern void tsfi_bn_drain_pool(void);
    extern void tsfi_dysnomia_drain_pools(void);
    tsfi_bn_drain_pool();
    tsfi_dysnomia_drain_pools();
    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
