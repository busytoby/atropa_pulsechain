#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <immintrin.h>
#include "lau_memory.h"
#include "tsfi_types.h"
#include "tsfi_opt_zmm.h"
#include "tsfi_time.h"
#include "tsfi_k0rn_ops.h"

int main(void) {
    printf("=== TSFi Architecture Benchmark: DeepSeek DNA Vector GEMV (Dense Layer Simulation) ===\n");
    
    const char *dna_path = "assets/dna/deepseek_coder_v2/deepseek_coder_v2_moe_block.dna";
    FILE *f = fopen(dna_path, "rb");
    if (!f) {
        printf("[FRACTURE] Missing DNA file: %s. You must run the transpiler first.\n", dna_path);
        return 1;
    }
    
    fseek(f, 0, SEEK_END);
    off_t size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    void *ptr = lau_memalign_wired(512, size);
    if (!ptr || fread(ptr, 1, size, f) != (size_t)size) {
        printf("[FRACTURE] Failed to load or map physical DNA.\n");
        if (ptr) lau_free(ptr);
        fclose(f);
        return 1;
    }
    fclose(f);
    
    K0RnStream *dna = (K0RnStream*)ptr;
    
    __m512 token_embed = _mm512_set1_ps(0.015f);
    
    printf("  [IO] Mapped DNA Tensor Footprint: %lu bytes (%u K0Rn Operations)\n", (unsigned long)size, dna->op_count);
    printf("  [BENCHMARK] Executing physical FP32 Dense Layer Scan against %u geometries...\n", dna->op_count);
    
    int passes = 10000;
    unsigned long long t_start = get_time_ns();
    
    __m512 accum = _mm512_setzero_ps();
    
    for (int p = 0; p < passes; p++) {
        // Resolve physical geometry offset directly from the packed structure
        K0RnOp *ops = (K0RnOp*)((uint8_t*)ptr + sizeof(K0RnStream));
        uint32_t count = dna->op_count;
        
        for (uint32_t i = 0; i < count; i++) {
            __m512 weight_vec = _mm512_loadu_ps((const float *)&ops[i]);
            accum = _mm512_fmadd_ps(token_embed, weight_vec, accum);
        }
    }
    
    unsigned long long t_end = get_time_ns();
    
    double duration_ms = (t_end - t_start) / 1000000.0;
    double duration_s = duration_ms / 1000.0;
    
    double total_gflops = ((double)passes * (double)dna->op_count * 32.0) / 1000000000.0;
    double gflops_per_sec = total_gflops / duration_s;
    
    double gb_scanned = ((double)passes * (double)dna->op_count * 64.0) / 1000000000.0;
    double bandwidth_gb_s = gb_scanned / duration_s;
    
    float result = _mm512_reduce_add_ps(accum);
    
    printf("\n  [BENCHMARK] Total Token Passes: %d", passes);
    printf("\n  [BENCHMARK] Total Math: %f GFLOPs computed.", total_gflops);
    printf("\n  [BENCHMARK] Total RAM Streamed: %.2f GB", gb_scanned);
    printf("\n  [BENCHMARK] Execution Time: %.3f ms", duration_ms);
    printf("\n  [BENCHMARK] Final Tensor Density: %f", result);
    printf("\n  [BENCHMARK] Computational Velocity: %.2f GFLOPS/sec", gflops_per_sec);
    printf("\n  [BENCHMARK] Memory Wall Bandwidth: %.2f GB/sec\n", bandwidth_gb_s);
    
    lau_free(ptr);
    
    extern void lau_free_all_active(void);
    lau_free_all_active();
    
    printf("\n=== Physical DNA Matrix Scan Complete ===\n");
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    
    return 0;
}
