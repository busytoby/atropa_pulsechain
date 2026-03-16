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
    printf("=== TSFi Architecture Benchmark: DeepSeek Sparse MoE Routing ===\n");
    
    // DeepSeek-Coder-V2 uses a massive sparse Mixture-of-Experts architecture.
    // E.g., 160 fine-grained experts, where only 6 are active per token.
    const int num_experts = 160;
    const int active_experts = 6;
    const int expert_dim = 4096; // Simulated hidden dimension of an expert
    
    // Total footprint for all experts
    size_t total_moe_size = num_experts * expert_dim * sizeof(float);
    printf("  [IO] Allocating Sparse MoE Matrix: %lu MB (%d experts, %d active)\n", total_moe_size / (1024 * 1024), num_experts, active_experts);
    
    float *moe_weights = (float*)lau_memalign_wired(512, total_moe_size);
    if (!moe_weights) {
        printf("[FRACTURE] System OOM.\n");
        return 1;
    }
    
    // Simulate token state input
    __m512 token_state[expert_dim / 16];
    for (int i = 0; i < (expert_dim / 16); i++) {
        token_state[i] = _mm512_set1_ps(0.01f);
    }
    
    // Simulate the Gate output (the network decides which 6 experts to trigger)
    int routed_experts[6] = {12, 45, 88, 102, 134, 159};
    
    printf("  [BENCHMARK] Executing physical AVX-512 sparse routing across massive parameter space...\n");
    
    int passes = 10000;
    unsigned long long t_start = get_time_ns();
    
    __m512 total_accum = _mm512_setzero_ps();
    
    for (int p = 0; p < passes; p++) {
        __m512 token_accum = _mm512_setzero_ps();
        
        // DeepSeek MoE executes strictly on the 6 active experts per token, completely skipping the 154 others.
        for (int e = 0; e < active_experts; e++) {
            int expert_idx = routed_experts[e];
            float *expert_block = &moe_weights[expert_idx * expert_dim];
            
            // FMA math across the specific expert's hidden dimension
            for (int i = 0; i < (expert_dim / 16); i++) {
                __m512 v_weight = _mm512_loadu_ps(&expert_block[i * 16]);
                token_accum = _mm512_fmadd_ps(token_state[i], v_weight, token_accum);
            }
        }
        
        // Add to global state to prevent compiler stripping
        total_accum = _mm512_add_ps(total_accum, token_accum);
    }
    
    unsigned long long t_end = get_time_ns();
    
    double duration_ms = (t_end - t_start) / 1000000.0;
    double duration_s = duration_ms / 1000.0;
    
    // FLOPs per expert: expert_dim * 2 (1 multiply + 1 add)
    // 4096 * 2 = 8192 FLOPs per expert
    // Total FLOPs per token pass: 8192 * 6 active experts = 49,152 FLOPs
    double total_gflops = ((double)passes * (double)active_experts * (double)expert_dim * 2.0) / 1000000000.0;
    double gflops_per_sec = total_gflops / duration_s;
    
    // RAM Streamed: We only stream the 6 active experts out of 160.
    // 6 experts * 4096 floats * 4 bytes = 98,304 bytes per token pass.
    double gb_scanned = ((double)passes * (double)active_experts * (double)expert_dim * 4.0) / 1000000000.0;
    double bandwidth_gb_s = gb_scanned / duration_s;
    
    float final_mass = _mm512_reduce_add_ps(total_accum);
    
    printf("\n  [BENCHMARK] Total Token Passes: %d", passes);
    printf("\n  [BENCHMARK] Execution Time: %.3f ms", duration_ms);
    printf("\n  [BENCHMARK] Active MoE Generation Latency (Per Token): %.3f ms", duration_ms / passes);
    printf("\n  [BENCHMARK] Sparse Computational Velocity: %.2f GFLOPS/sec", gflops_per_sec);
    printf("\n  [BENCHMARK] Sparse Memory Bandwidth: %.2f GB/sec\n", bandwidth_gb_s);
    printf("\n  [BENCHMARK] Final Routing Mass: %f\n", final_mass);
    
    lau_free(moe_weights);
    
    extern void lau_free_all_active(void);
    lau_free_all_active();
    
    printf("\n=== DeepSeek MoE Benchmark Complete ===\n");
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    
    return 0;
}
