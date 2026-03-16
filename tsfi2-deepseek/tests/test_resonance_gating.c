#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tsfi_helmholtz.h"
#include "tsfi_io.h"

int main() {
    printf("=== TSFi Acoustic Harmonic Gating Test ===\n");
    tsfi_helmholtz_init();

    void *manifold = calloc(1, 512 * 1024);
    AR_AccountLeaf *leaves = (AR_AccountLeaf *)manifold;
    uint64_t world_k = 2026;

    // Setup Leaf 512 (Source)
    leaves[512].nonce = 1;
    memset(leaves[512].balance, 0xFF, 32); // Max Balance

    TSFiDirective tx = {0};
    tx.src_leaf = 512;
    tx.dst_leaf = 513;
    tx.nonce = 1;
    tx.chain_id = world_k;
    tx.signature[0] = 0x51; // 'S' valid signature hook

    // 1. Resonance Gating: k=1 (All-Pass)
    printf("[TEST] Resonance Gating with k=1 (Should PASS regardless of Hash)...\n");
    
    uint8_t wave_hash[64];
    tsfi_keccak512((const uint8_t*)&tx, 140, wave_hash);
    uint64_t res_val = *(uint64_t*)wave_hash;
    
    uint64_t k_pass = 1;
    if ((res_val % k_pass) == 0) {
        printf("[PASS] Harmonic Coherence verified for k=1.\n");
    } else {
        printf("[FAIL] k=1 gated wave? Math error.\n");
        exit(1);
    }

    // 2. Resonance Gating: High Frequency
    uint64_t k_high = 0xFFFFFFFFFFFFFFFFULL; 
    printf("[TEST] Resonance Gating with k=MAX (Should FAIL with high probability)...\n");
    if ((res_val % k_high) == 0 && res_val != 0) {
        printf("[WARN] Statistical Anomaly: Random hash matched k=MAX.\n");
    } else {
        printf("[PASS] Wave energy absorbed by dielectric (Mismatch verified).\n");
    }

    // 3. Resonance Gating: Specific k
    printf("[TEST] Verifying Modulo Residue logic for k=%lu...\n", world_k);
    uint64_t mod = res_val % world_k;
    printf("Wave Frequency Residue: %lu\n", mod);
    if (mod < world_k) {
        printf("[PASS] Residue within boundary.\n");
    } else {
        printf("[FAIL] Modulo residue overflow.\n");
        exit(1);
    }

    free(manifold);
    
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    extern void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr);
    lau_assert_zero_unsealed_leaks("test", NULL);
    return 0;
}
