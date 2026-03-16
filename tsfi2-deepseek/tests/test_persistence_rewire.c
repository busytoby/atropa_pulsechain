#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "tsfi_wavelet_arena.h"

int main() {
    printf("=== TSFi Persistence and Re-Wiring Audit ===\n");
    
    uint64_t P = 953473ULL;
    uint8_t *mem1 = malloc(1024 * 1024);
    TsfiWaveletArena arena1;
    tsfi_wavelet_arena_init(&arena1, mem1, 1024 * 1024);
    
    // 1. Setup Original Wavelet
    TsfiWavelet *W_orig = tsfi_STAT(&arena1, P);
    W_orig->telemetry.Monopole = 777; 
    W_orig->telemetry.Fa.channel = 2026;
    W_orig->telemetry.ReciprocalChannel = 1234;
    W_orig->state = WAVELET_STATE_ACTIVATE_YANG; // Simulate YANG state
    W_orig->ichidai = 100;
    W_orig->daiichi = 200;
    
    printf("[PROCESS 1] Original Wavelet UID: %llu, State: %u\n", 
           (unsigned long long)W_orig->telemetry.unique_id, W_orig->state);
    
    // Evolve once to set a baseline
    W_orig->Evolve(0);
    uint64_t ichi_step1 = W_orig->ichidai;
    printf("[PROCESS 1] Post-Evolve Entropy: [%llu, %llu]\n", 
           (unsigned long long)W_orig->ichidai, (unsigned long long)W_orig->daiichi);

    // 2. Persist to "Disk" (Simulated)
    FILE *f = fopen("test_wavelet.bin", "wb");
    fwrite(W_orig, TSFI_WAVELET_LEAF_SIZE, 1, f);
    fclose(f);
    
    free(mem1); // Process 1 memory is gone
    printf("[PROCESS 1] Memory freed. Wavelet persisted.\n");

    // 3. Load into "Process 2" (New Memory Address)
    printf("\n--- Starting Process 2 Simulation ---\n");
    uint8_t *mem2 = malloc(1024 * 1024);
    // We offset the load to ensure the address is different
    TsfiWavelet *W_loaded = (TsfiWavelet*)(mem2 + 4096); 
    
    f = fopen("test_wavelet.bin", "rb");
    fread(W_loaded, TSFI_WAVELET_LEAF_SIZE, 1, f);
    fclose(f);
    
    printf("[PROCESS 2] Loaded Wavelet UID: %llu at Address: %p\n", 
           (unsigned long long)W_loaded->telemetry.unique_id, (void*)W_loaded);
    
    // 4. Re-Wire
    tsfi_RewireWavelet(W_loaded);
    
    // 5. Verify capabilities are restored
    printf("[PROCESS 2] Executing Evolve(0) on re-wired wavelet...\n");
    W_loaded->Evolve(0);
    
    printf("[PROCESS 2] Final Entropy: [%llu, %llu]\n", 
           (unsigned long long)W_loaded->ichidai, (unsigned long long)W_loaded->daiichi);
    
    if (W_loaded->ichidai != ichi_step1) {
        printf("[SUCCESS] Re-wired evolution sequence progressed from previous state.\n");
    } else {
        printf("[FAILURE] Entropy did not advance correctly.\n");
        return 1;
    }

    free(mem2);
    unlink("test_wavelet.bin");
    printf("=== Persistence Audit Passed ===\n");
    return 0;
}
