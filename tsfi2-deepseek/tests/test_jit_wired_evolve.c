#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "tsfi_wavelet_arena.h"

int main() {
    printf("=== TSFi JIT-Wired Method Test (W->Evolve) ===\n");
    
    uint64_t P = 953473ULL; // LITTLE_PRIME
    uint8_t *mem = malloc(1024 * 1024);
    TsfiWaveletArena arena;
    tsfi_wavelet_arena_init(&arena, mem, 1024 * 1024);
    
    // 1. Establish Basis (SEAL0-8)
    TsfiWavelet *W = tsfi_STAT(&arena, P);
    tsfi_Avail(W, 4052, P);
    W->telemetry.Monopole = 777; 
    W->telemetry.Fa.channel = 2026;
    W->telemetry.ReciprocalChannel = 1234;
    W->state = WAVELET_STATE_MAGNETIZE; // SEAL8
    
    // 2. Advance to YI (SEAL9)
    W->state = WAVELET_STATE_CREATE_YI;
    
    // 3. Attach Metadata and SEAL10 (Header)
    strncpy(W->owner, "JIT_Architect", 31);
    W->quota = 9999;
    tsfi_SealHeader(W, P);
    
    // 4. Activate YANG (SEAL11/Dim 6)
    tsfi_ActivateYang(W, 2026, P);
    
    // 5. JIT-Wired Evolution using the W->Evolve(selection) syntax
    printf("\n[C] Executing JIT-wired evolution (W->Evolve)...\n");
    W->Evolve(0); // Ichidai Pulse
    W->Evolve(1); // Daiichi Pulse
    W->Evolve(0); // Ichidai Pulse
    
    tsfi_wavelet_print(W, "JIT_WIRED_WAVELET");
    
    // Cleanup: In a production system, we'd munmap the thunk
    free(mem);
    return 0;
}
