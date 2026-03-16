#include "tsfi_reaction.h"
#include "tsfi_io.h"
#include "tsfi_dys_math.h"
#include "tsfi_time.h"
#include "lau_memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TARGET_MEGABYTES 130
#define BYTES_PER_DAI 128
#define TOTAL_OPS ((TARGET_MEGABYTES * 1024 * 1024) / BYTES_PER_DAI)
#define BATCH_COUNT (TOTAL_OPS / 8)

int main() {
    TSFiBigInt* Prime = tsfi_bn_alloc();
    tsfi_bn_set_u64(Prime, 953467954114363ULL);

    tsfi_io_printf(stdout, "=== TSFi Reaction Encryptable Buffer Benchmark ===\n");
    tsfi_io_printf(stdout, "Targeting: %d MB buffer via %d ReactSHIO calls.\n", TARGET_MEGABYTES, TOTAL_OPS);

    // Seed the foundational YI structures
    struct YI* root_yi_batch[8] = {0};
    tsfi_reaction_shoot_batch8_fused(root_yi_batch, Prime);

    // Extract the Pi Rings
    TSFiBigInt* Pi_batch[8];
    for (int i = 0; i < 8; i++) {
        Pi_batch[i] = root_yi_batch[i]->Ring ? root_yi_batch[i]->Ring : root_yi_batch[i]->Xi;
    }

    // Allocate the unified buffer space
    uint8_t* crypto_buffer = lau_malloc_wired(TARGET_MEGABYTES * 1024 * 1024);
    if (!crypto_buffer) {
        tsfi_io_printf(stderr, "[ERROR] Failed to allocate %d MB crypto buffer.\n", TARGET_MEGABYTES);
        return 1;
    }
    
    struct Dai* dai_batch[8] = {0};
    uint64_t bytes_written = 0;
    
    tsfi_io_printf(stdout, "[EXECUTION] Streaming %d Batches (AVX-512) into Buffer...\n", BATCH_COUNT);
    uint64_t start_time = get_time_ns() / 1000000;

    for (int b = 0; b < BATCH_COUNT; b++) {
        for (int i = 0; i < 8; i++) {
            // Generate the React state (the encrypted payload)
            dai_batch[i] = tsfi_reaction_compute_reciprocity(root_yi_batch[i]->Psi, Pi_batch[i], NULL);
            
            // Serialize Daiichi and Ichidai directly into the contiguous buffer
            // To form sequential state, we use the Dai output to advance Pi
            Pi_batch[i] = dai_batch[i]->Daiichi;
            
            memcpy(crypto_buffer + bytes_written, &dai_batch[i]->Ichidai->limbs[0], 64);
            bytes_written += 64;
            memcpy(crypto_buffer + bytes_written, &dai_batch[i]->Daiichi->limbs[0], 64);
            bytes_written += 64;
            
            freeDAI(dai_batch[i]);
        }
    }

    uint64_t end_time = get_time_ns() / 1000000;
    double duration_ms = (double)(end_time - start_time);
    if (duration_ms == 0) duration_ms = 1;
    
    double mb_per_sec = (TARGET_MEGABYTES / (duration_ms / 1000.0));

    tsfi_io_printf(stdout, "\n[RESULTS]\n");
    tsfi_io_printf(stdout, "Duration       : %.2f ms\n", duration_ms);
    tsfi_io_printf(stdout, "Buffer Written : %.2f MB\n", (double)bytes_written / (1024.0 * 1024.0));
    tsfi_io_printf(stdout, "Throughput     : %.2f MB/sec\n", mb_per_sec);

    if (mb_per_sec >= 100.0) {
        tsfi_io_printf(stdout, "[SUCCESS] Verified sustained sequential rate > 100 MB/sec for encryptable stream.\n");
    } else {
        tsfi_io_printf(stdout, "[WARNING] Throughput fell below threshold. Structural bottleneck detected.\n");
    }

    lau_free(crypto_buffer);
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
