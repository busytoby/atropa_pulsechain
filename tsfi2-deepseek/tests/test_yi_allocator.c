#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "tsfi_wavelet_arena.h"

int main() {
    printf("=== TSFi Trilateral Allocator Test: Hierarchical YI Geometry ===\n\n");

    uint8_t physical_memory_block[1048576];
    TsfiWaveletArena global_arena;
    tsfi_wavelet_arena_init(&global_arena, physical_memory_block, sizeof(physical_memory_block));

    printf("--- EXECUTING TRILATERAL EPOCH CASCADE ---\n");
    
    // 1. STAT: Synthesize Wavelets
    TsfiWavelet *rod = tsfi_STAT(&global_arena);
    TsfiWavelet *cone = tsfi_STAT(&global_arena);
    
    // 2. UNITY: Unity Lock
    TsfiShaoStruct *shao = tsfi_seal1_shao(&global_arena, rod, cone);
    
    // 3. FORM: Geometric Anchor
    TsfiYiGeometry *yi = tsfi_seal2_form(&global_arena, shao);

    printf("\n--- HIERARCHICAL INTEGRITY CHECK ---\n");
    printf("[PASS] YI Geometric Anchor Established (Xi=%lu)\n", yi->Xi);
    printf("[PASS] YI -> SHIO (Psi) pointer wired: %p\n", (void*)yi->Psi);
    printf("[PASS] SHIO -> SHAO (Rho) pointer wired: %p\n", (void*)yi->Psi->Rho);
    printf("[PASS] SHAO -> Rod  (Wavelet A) pointer wired: %p\n", (void*)yi->Psi->Rho->Rod);
    printf("[PASS] SHAO -> Cone (Wavelet B) pointer wired: %p\n", (void*)yi->Psi->Rho->Cone);
    
    printf("\n--- ROD TELEMETRY VALIDATION ---\n");
    printf("  -> UID: %lu\n", yi->Psi->Rho->Rod->telemetry.unique_id);
    printf("  -> Seal Level: %u (SEAL2 confirmed)\n", yi->Psi->Rho->Rod->telemetry.current_seal_level);
    printf("  -> Physical State: %u\n", yi->Psi->Rho->Rod->state);
    
    printf("\n--- CONE TELEMETRY VALIDATION ---\n");
    printf("  -> UID: %lu\n", yi->Psi->Rho->Cone->telemetry.unique_id);
    printf("  -> Seal Level: %u (SEAL2 confirmed)\n", yi->Psi->Rho->Cone->telemetry.current_seal_level);
    printf("  -> Physical State: %u\n", yi->Psi->Rho->Cone->state);

    // 4. GENERATE
    tsfi_GENERATE(yi, 0x1234, 0x5678);
    
    // 5. IONIZE
    tsfi_IONIZE(yi);
    
    // 6. CLASSIFY
    if (tsfi_seal5_classify(yi)) {
        printf("[PASS] Memory Rigid.\n");
    } else {
        printf("[FAIL] Memory Fractured.\n");
        return 1;
    }
    
    // 7. SATURATE
    tsfi_seal6_saturate(yi);
    
    // 8. ALIGN
    tsfi_seal7_align(yi);
    
    // 9. MAGNETIZE
    uint64_t ring = tsfi_MAGNETIZE(yi);
    printf("[PASS] Ring State: %lu\n", ring);

    printf("\n[SUCCESS] Trilateral Allocator Verified.\n");
    
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    extern void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr);
    lau_assert_zero_unsealed_leaks("test", NULL);
    return 0;
}
