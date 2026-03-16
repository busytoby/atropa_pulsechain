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
    printf("=== TSFi Architecture Benchmark: DeepSeek MLA Context Window Scaling ===\n");
    
    // We need to simulate the latency of the Attention layer as context size grows.
    // DeepSeek-Coder-V2 supports up to 128k context, but we will benchmark the latency 
    // of the Q*K^T scaling for a standard local 8k context window (8192 tokens).
    
    const uint32_t context_size = 8192;
    const uint32_t head_dim = 128; // Standard dimension per attention head
    
    // Memory allocation for the Key/Value cache.
    // KV Cache Size = context_size * head_dim * sizeof(float)
    size_t kv_size = context_size * head_dim * sizeof(float);
    
    float *k_cache = (float*)lau_memalign_wired(512, kv_size);
    
    if (!k_cache) {
        printf("[FRACTURE] Failed to allocate KV cache block.\n");
        return 1;
    }
    
    // Populate with dummy values to prevent optimizing away
    for (size_t i = 0; i < (kv_size / sizeof(float)); i++) {
        k_cache[i] = (float)(i % 100) * 0.001f;
    }
    
    // The Query vector for the current generation token
    __m512 v_q[head_dim / 16]; // 128 floats = 8 AVX-512 vectors
    for (int i = 0; i < (head_dim / 16); i++) {
        v_q[i] = _mm512_set1_ps(0.05f);
    }
    
    printf("  [IO] Allocated %lu MB Key Cache for %u tokens.\n", kv_size / (1024 * 1024), context_size);
    printf("  [BENCHMARK] Executing physical AVX-512 Q*K^T attention scan...\n");
    
    int passes = 10000; 
    unsigned long long t_start = get_time_ns();
    
    __m512 total_attn = _mm512_setzero_ps();
    
    for (int p = 0; p < passes; p++) {
        // For a single token generation step, the Query must attend to ALL previous tokens in the context window.
        // This is a dot product of v_q with every single 128-dim vector in the K-cache.
        
        for (uint32_t tok = 0; tok < context_size; tok++) {
            float *k_vec = &k_cache[tok * head_dim];
            __m512 token_score = _mm512_setzero_ps();
            
            // Dot product of 128-dim query vs 128-dim key (8 AVX-512 iterations)
            for (int h = 0; h < (head_dim / 16); h++) {
                __m512 vk = _mm512_loadu_ps(&k_vec[h * 16]);
                token_score = _mm512_fmadd_ps(v_q[h], vk, token_score);
            }
            
            // Accumulate the horizontal sums into the global attention mass to prevent compiler stripping
            total_attn = _mm512_add_ps(total_attn, token_score);
        }
    }
    
    unsigned long long t_end = get_time_ns();
    
    double duration_ms = (t_end - t_start) / 1000000.0;
    double duration_s = duration_ms / 1000.0;
    
    // Math logic: (8 FMA instructions * 32 FLOPs = 256 FLOPs) per token.
    // 256 FLOPs * 8192 tokens = 2,097,152 FLOPs per pass.
    double total_gflops = ((double)passes * (double)context_size * 256.0) / 1000000000.0;
    double gflops_per_sec = total_gflops / duration_s;
    
    // Memory logic: 128 floats (512 bytes) read per token.
    // 512 bytes * 8192 tokens = 4,194,304 bytes per pass.
    double gb_scanned = ((double)passes * (double)context_size * 512.0) / 1000000000.0;
    double bandwidth_gb_s = gb_scanned / duration_s;
    
    float final_mass = _mm512_reduce_add_ps(total_attn);
    
    printf("\n  [BENCHMARK] Context Window: %u tokens", context_size);
    printf("\n  [BENCHMARK] Token Passes (Attention Scans): %d", passes);
    printf("\n  [BENCHMARK] Execution Time: %.3f ms", duration_ms);
    printf("\n  [BENCHMARK] Generation Latency (Per Token): %.3f ms", duration_ms / passes);
    printf("\n  [BENCHMARK] Final Attention Mass: %f", final_mass);
    printf("\n  [BENCHMARK] Computational Velocity: %.2f GFLOPS/sec", gflops_per_sec);
    printf("\n  [BENCHMARK] KV Cache Bandwidth: %.2f GB/sec\n", bandwidth_gb_s);
    
    lau_free(k_cache);
    
    extern void lau_free_all_active(void);
    lau_free_all_active();
    
    printf("\n=== Attention Window Benchmark Complete ===\n");
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    
    return 0;
}
