#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "tsfi_wavelet_arena.h"

int main() {
    printf("=== TSFi Wired C Method Test (W->Evolve) ===\n");
    
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
    strncpy(W->owner, "Wired_Developer", 31);
    W->quota = 7777;
    tsfi_SealHeader(W, P);
    
    // 4. Activate YANG (SEAL11/Dim 6)
    tsfi_ActivateYang(W, 2026, P);
    
    // 5. Wired Evolution using the W->Evolve syntax
    printf("\n[C] Executing wired evolution sequence (W->Evolve)...\n");
    W->Evolve(W, 0); // Evolve using Ichidai
    W->Evolve(W, 1); // Evolve using Daiichi
    W->Evolve(W, 0); // Evolve using Ichidai
    
    tsfi_wavelet_print(W, "WIRED_WAVELET");
    
    free(mem);
    return 0;
}
