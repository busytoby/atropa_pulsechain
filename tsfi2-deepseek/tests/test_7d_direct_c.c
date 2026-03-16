#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "tsfi_wavelet_arena.h"

int main() {
    printf("=== TSFi Direct C API Test (Dimension 7 Evolution) ===\n");
    
    uint64_t P = 953473ULL; // LITTLE_PRIME
    uint8_t *mem = malloc(1024 * 1024);
    TsfiWaveletArena arena;
    tsfi_wavelet_arena_init(&arena, mem, 1024 * 1024);
    
    // 1. Establish Basis (SEAL0-8)
    TsfiWavelet *W = tsfi_STAT(&arena, P);
    tsfi_Avail(W, 4052, P);
    // Note: Manually set necessary trilateral anchors for state transition
    W->telemetry.Monopole = 777; 
    W->telemetry.Fa.channel = 2026;
    W->telemetry.ReciprocalChannel = 1234;
    W->state = WAVELET_STATE_MAGNETIZE; // SEAL8
    
    // 2. Advance to YI (SEAL9)
    W->state = WAVELET_STATE_CREATE_YI;
    
    // 3. Attach Metadata and SEAL10 (Header)
    strncpy(W->owner, "C_Programmer", 31);
    W->quota = 5000;
    tsfi_SealHeader(W, P);
    
    // 4. Activate YANG (SEAL11/Dim 6)
    tsfi_ActivateYang(W, 2026, P);
    
    // 5. Recursive Evolution (SEAL12+/Dim 7)
    printf("\n[C] Executing recursive entropy sequence directly in C...\n");
    for (int i = 0; i < 5; i++) {
        tsfi_EvolveDai(W, i % 2, P);
    }
    
    tsfi_wavelet_print(W, "DIRECT_C");
    
    free(mem);
    return 0;
}
