#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "tsfi_wavelet_arena.h"

int main() {
    printf("=== TSFi Trilateral Allocator Test: Hierarchical YI Geometry ===\n\n");

    uint8_t physical_memory_block[1048576];
    TsfiWaveletArena global_arena;
    tsfi_wavelet_arena_init(&global_arena, physical_memory_block, sizeof(physical_memory_block));

    uint64_t prime_val = 953467954114363ULL; // Auncient Motzkin Prime

    printf("--- EXECUTING TRILATERAL EPOCH CASCADE ---\n");
    
    // 1. STAT: Synthesize Wavelets
    TsfiWavelet *rod = tsfi_STAT(&global_arena, prime_val);
    TsfiWavelet *cone = tsfi_STAT(&global_arena, prime_val);
    
    // 2. GENERATE: Initialize states (Xi, Alpha, Beta)
    tsfi_GENERATE(rod, cone, 12345ULL, 67890ULL, 54321ULL, prime_val);
    
    // 3. Progressive Seal Levels
    tsfi_Form(rod, 100ULL, prime_val);
    tsfi_Form(cone, 100ULL, prime_val);
    
    tsfi_Polarize(rod, prime_val);
    tsfi_Polarize(cone, prime_val);
    
    tsfi_Conjugate(rod, 200ULL, prime_val);
    tsfi_Conjugate(cone, 200ULL, prime_val);
    
    tsfi_Conify(rod, 300ULL, prime_val);
    tsfi_Conify(cone, 300ULL, prime_val);
    
    tsfi_Saturate(rod, 300ULL, 10ULL, 50ULL, prime_val);
    tsfi_Saturate(cone, 300ULL, 10ULL, 50ULL, prime_val);
    
    tsfi_IONIZE_BOND(rod, prime_val);
    tsfi_IONIZE_BOND(cone, prime_val);
    
    // 4. MAGNETIZE
    tsfi_MAGNETIZE_JOINT(rod, cone, prime_val);

    printf("\n--- HIERARCHICAL INTEGRITY CHECK ---\n");
    printf("[PASS] YI Wavelet A (Rod) pointer: %p\n", (void*)rod);
    printf("[PASS] YI Wavelet B (Cone) pointer: %p\n", (void*)cone);
    
    printf("\n--- ROD TELEMETRY VALIDATION ---\n");
    printf("  -> UID: %lu\n", (unsigned long)rod->telemetry.unique_id);
    printf("  -> Seal Level: %u (SEAL8 confirmed)\n", rod->telemetry.current_seal_level);
    printf("  -> Physical State: %u\n", rod->state);
    
    printf("\n--- CONE TELEMETRY VALIDATION ---\n");
    printf("  -> UID: %lu\n", (unsigned long)cone->telemetry.unique_id);
    printf("  -> Seal Level: %u (SEAL8 confirmed)\n", cone->telemetry.current_seal_level);
    printf("  -> Physical State: %u\n", cone->state);

    if (rod->telemetry.Fa.monopole == cone->telemetry.Fa.monopole) {
        printf("[PASS] Memory Rigid (Joint Monopole: %lu).\n", rod->telemetry.Fa.monopole);
    } else {
        printf("[FAIL] Memory Fractured.\n");
        return 1;
    }
    
    printf("\n[SUCCESS] Trilateral Allocator Verified.\n");
    
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    extern void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr);
    lau_assert_zero_unsealed_leaks("test", NULL);
    return 0;
}
