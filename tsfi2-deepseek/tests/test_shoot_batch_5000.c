#include "tsfi_reaction.h"
#include "tsfi_io.h"
#include "tsfi_dys_math.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main() {
    int total_shots = 5000;
    int batch_count = total_shots / 8;
    tsfi_io_printf(stdout, "[SHOOT-BATCH-5000] Initiating 5000 Fused Batch Shots...\n");

    TSFiBigInt *p_bn = tsfi_bn_alloc();
    tsfi_bn_set_u64(p_bn, 953467954114363ULL);

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    struct YI* yi_batch[8] = {NULL};
    int success_count = 0;

    for (int b = 0; b < batch_count; b++) {
        tsfi_reaction_shoot_batch8_fused(yi_batch, p_bn);
        
        for (int i = 0; i < 8; i++) {
            if (yi_batch[i] && yi_batch[i]->Psi->Manifold) {
                TSFiBigInt *pi_bn = (yi_batch[i]->Ring && yi_batch[i]->Ring->active_limbs > 0) ? yi_batch[i]->Ring : yi_batch[i]->Xi;
                struct Dai* dai = tsfi_reaction_compute_reciprocity(yi_batch[i]->Psi, pi_bn, NULL);
                if (dai) {
                    success_count++;
                    // Free the generated DAI memory immediately
                    freeDAI(dai);
                }
            }
        }

        if (b > 0 && b % 100 == 0) {
            tsfi_io_printf(stdout, "Progress: %d shots completed...\n", b * 8);
            tsfi_io_flush(stdout);
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    tsfi_io_printf(stdout, "\n[BATCH BENCHMARK RESULTS]\n");
    tsfi_io_printf(stdout, "Total Shots:   %d\n", batch_count * 8);
    tsfi_io_printf(stdout, "Total Success: %d\n", success_count);
    tsfi_io_printf(stdout, "Total Time:    %.4f s\n", elapsed);
    tsfi_io_printf(stdout, "Throughput:    %.2f shots/sec\n", (double)success_count / elapsed);

    if (success_count == batch_count * 8) {
        tsfi_io_printf(stdout, "[SUCCESS] 5000 Fused Batch shots verified.\n");
    }

    for(int i=0; i<8; i++) if(yi_batch[i]) freeYI(yi_batch[i]);
    tsfi_bn_free(p_bn);

        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
