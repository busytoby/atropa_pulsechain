#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include "tsfi_wavelet_arena.h"

void run_thorough_audit(uint64_t P, const char* domain_name) {
    printf("\n=== Auditing Domain: %s (%llu) ===\n", domain_name, (unsigned long long)P);
    
    uint8_t *mem = malloc(1024 * 1024 * 4);
    TsfiWaveletArena arena;
    tsfi_wavelet_arena_init(&arena, mem, 1024 * 1024 * 4);
    
    // 1. Synthesize multiple unique wavelets
    TsfiWavelet *W1 = tsfi_STAT(&arena, P);
    TsfiWavelet *W2 = tsfi_STAT(&arena, P);
    
    // Manually advance to SEAL11 with Large-Scale anchors
    // Using values that scale with the Motzkin Prime (approx 15 digits)
    W1->telemetry.Monopole = P / 7; 
    W1->telemetry.Fa.channel = P - 101; 
    W1->telemetry.ReciprocalChannel = P - 202;
    W1->telemetry.Ring = 0xABCDE12345ULL % P;

    W2->telemetry.Monopole = P / 3; 
    W2->telemetry.Fa.channel = P - 303; 
    W2->telemetry.ReciprocalChannel = P - 404;
    W2->telemetry.Ring = 0xFEDCB67890ULL % P;
    
    W1->state = WAVELET_STATE_CREATE_YI;
    W2->state = WAVELET_STATE_CREATE_YI;
    
    tsfi_SealHeader(W1, P);
    tsfi_SealHeader(W2, P);
    
    tsfi_ActivateYang(W1, 2026, P);
    tsfi_ActivateYang(W2, 2026, P);
    
    printf(" [1/3] Initial States Established. Wiring unique JIT thunks...\n");
    
    // 2. Stress Test: 1000 Recursive Pulses per Wavelet
    printf(" [2/3] Executing 1000-pulse sequence per instance...\n");
    for (int i = 0; i < 1000; i++) {
        W1->Evolve(i % 2); 
        W2->Evolve((i + 1) % 2);
        
        if (i % 250 == 0) {
            if (W1->ichidai == 0 || W1->daiichi == 0) {
                printf("[FRACTURE] Entropy collapse at pulse %d\n", i);
                exit(1);
            }
        }
    }
    
    printf(" [3/3] Sequence Complete. SEAL Levels: W1=%d, W2=%d\n", (int)W1->state - 1, (int)W2->state - 1);
    assert(W1->state > 1000);
    assert(W2->state > 1000);
    
    tsfi_wavelet_print(W1, "AUDIT_W1");
    tsfi_wavelet_print(W2, "AUDIT_W2");
    
    free(mem);
    printf("=== %s Audit Passed ===\n", domain_name);
}

int main() {
    printf("=== TSFi Thorough JIT-Wired Evolution Audit ===\n");
    
    run_thorough_audit(953473ULL, "LITTLE_PRIME");
    run_thorough_audit(953467954114363ULL, "MOTZKIN_PRIME");
    
    printf("\n[SUCCESS] All domains verified rigid. JIT-Wired method W->Evolve() is stable.\n");
    return 0;
}
