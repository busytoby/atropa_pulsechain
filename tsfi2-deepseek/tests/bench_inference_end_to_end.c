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

// Define external deepseek core function
extern void tsfi_kernel_deepseek_mla(void *regs, ZmmSynapse *syn);

int main(void) {
    printf("=== TSFi Architecture Benchmark: DeepSeek End-To-End Latent Inference ===\n");
    
    // Allocate the unified ZMM architectural execution block
    TsfiZmmManifest *manifest = (TsfiZmmManifest*)lau_memalign_wired(512, sizeof(TsfiZmmManifest));
    memset(manifest, 0, sizeof(TsfiZmmManifest));
    manifest->contiguous_rf = lau_memalign_wired(512, 2048);
    manifest->active_mask = 0xFFFFFFFF; // Fully saturate the 32 registers
    manifest->micro_kernel = tsfi_kernel_deepseek_mla;
    
    // Simulate initial vector entropy (the input prompt embedding)
    tsfi_scramble_wave512(manifest->contiguous_rf, 2048);
    
    printf("  [IO] ZMM Architecture initialized and securely mounted to AVX-512 boundaries.\n");
    printf("  [BENCHMARK] Executing full Transformer layer stack inference (32 layers)...\n");
    
    const int num_layers = 32;
    const int generated_tokens = 512; // Simulate a 512-token response
    
    unsigned long long t_start = get_time_ns();
    
    // The core generation loop
    for (int t = 0; t < generated_tokens; t++) {
        // Pass the token state through the entire model layer stack
        for (int l = 0; l < num_layers; l++) {
            tsfi_dispatch_zmm_dynamic(manifest);
        }
    }
    
    unsigned long long t_end = get_time_ns();
    
    double duration_ms = (t_end - t_start) / 1000000.0;
    double ms_per_token = duration_ms / (double)generated_tokens;
    double tokens_per_second = 1000.0 / ms_per_token;
    
    printf("\n  [BENCHMARK] Tokens Generated: %d", generated_tokens);
    printf("\n  [BENCHMARK] Layers Penetrated per Token: %d", num_layers);
    printf("\n  [BENCHMARK] Total Execution Time: %.3f ms", duration_ms);
    printf("\n  [BENCHMARK] Generation Latency (Time To Token): %.3f ms/token", ms_per_token);
    printf("\n  [BENCHMARK] Theoretical Inference Velocity: %.2f Tokens/sec\n", tokens_per_second);
    
    lau_free(manifest->contiguous_rf);
    lau_free(manifest);
    
    extern void lau_free_all_active(void);
    lau_free_all_active();
    
    printf("\n=== End-To-End Inference Benchmark Complete ===\n");
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    
    return 0;
}
