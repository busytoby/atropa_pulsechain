#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "lau_memory.h"
#include "tsfi_types.h"
#include "tsfi_opt_zmm.h"
#include "tsfi_time.h"

// Define external execution
extern void tsfi_kernel_deepseek_mla(void *regs, ZmmSynapse *syn);

int main(void) {
    printf("=== TSFi Architecture Benchmark: DeepSeek-Coder-V2 MLA/MoE Kernels ===\n");
    
    // Allocate the 2KB contiguous register file aligned to Nef-511
    void *regs = lau_memalign_wired(512, 2048);
    ZmmSynapse syn;
    memset(&syn, 0, sizeof(ZmmSynapse));
    
    // Generate massive random AVX-512 input vectors to prevent compiler cache/optimizations
    // Simulating deep network latent states.
    tsfi_scramble_wave512(regs, 2048);
    
    // Warmup the CPU caches and vector units
    for (int i = 0; i < 1000; i++) {
        tsfi_kernel_deepseek_mla(regs, &syn);
    }
    
    printf("  [IO] Executing Physical AVX-512 Tensor Multiplication...\n");
    
    const uint64_t iterations = 10000000; // 10 Million forward passes
    
    unsigned long long t_start = get_time_ns();
    
    for (uint64_t i = 0; i < iterations; i++) {
        tsfi_kernel_deepseek_mla(regs, &syn);
    }
    
    unsigned long long t_end = get_time_ns();
    
    double duration_ms = (t_end - t_start) / 1000000.0;
    double duration_s = duration_ms / 1000.0;
    
    // In our specific kernel:
    // 2x _mm512_fmadd_ps (32 FLOPs each)
    // 1x _mm512_mul_ps (16 FLOPs)
    // 1x _mm512_cmp_ps_mask (16 FLOPs equivalent logically)
    // 1x _mm512_mask_blend_ps (16 FLOPs equivalent logically)
    // 1x _mm512_reduce_add_ps (15 FLOPs)
    // Total approximate mathematical FLOPs per iteration = ~127 FLOPs
    
    double total_gflops = ((double)iterations * 127.0) / 1000000000.0;
    double gflops_per_sec = total_gflops / duration_s;
    double millions_per_sec = ((double)iterations / 1000000.0) / duration_s;
    
    printf("\n  [BENCHMARK] Total Iterations: %lu", iterations);
    printf("\n  [BENCHMARK] Execution Time: %.3f ms", duration_ms);
    printf("\n  [BENCHMARK] Synaptic Resonance State: %f", syn.mass_density);
    printf("\n  [BENCHMARK] Throughput: %.2f Million Passes/sec", millions_per_sec);
    printf("\n  [BENCHMARK] Compute Bandwidth: %.2f GFLOPS/sec\n", gflops_per_sec);
    
    lau_free(regs);
    
    extern void lau_free_all_active(void);
    lau_free_all_active();
    
    printf("\n=== DeepSeek MLA Benchmark Complete ===\n");
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    
    return 0;
}
