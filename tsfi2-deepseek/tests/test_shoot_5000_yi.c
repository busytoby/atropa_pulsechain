#include "tsfi_reaction.h"
#include "tsfi_io.h"
#include "tsfi_dys_math.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main() {
    int total_shots = 5000;
    tsfi_io_printf(stdout, "[SHOOT-5000-YI] Initiating 5000 Advanced Reaction Shots...\n");

    TSFiBigInt *p_bn = tsfi_bn_alloc();
    tsfi_bn_set_u64(p_bn, 953467954114363ULL);

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    int success_count = 0;
    for (int i = 0; i < total_shots; i++) {
        struct YI* yi = tsfi_reaction_shoot(p_bn);
        if (!yi) {
            tsfi_io_printf(stderr, "\n[FAIL] Shot %d: Reaction failed.\n", i);
            return 1;
        }

        if (i > 0 && i % 1000 == 0) {
            tsfi_io_printf(stdout, "Progress: %d shots completed...\n", i);
            tsfi_io_flush(stdout);
        }

        TSFiBigInt *pi_bn = (yi->Ring && yi->Ring->active_limbs > 0) ? yi->Ring : yi->Xi;
        struct Dai* dai = tsfi_reaction_compute_reciprocity(yi->Psi, pi_bn, NULL);

        if (dai) {
            if (dai->Ichidai->active_limbs > 0 && dai->Daiichi->active_limbs > 0) {
                success_count++;
            }
            freeDAI(dai);
        } else {
            tsfi_io_printf(stderr, "\n[FAIL] Shot %d: Reciprocity failed.\n", i);
            return 1;
        }
        
        freeYI(yi);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    tsfi_io_printf(stdout, "\n[BENCHMARK RESULTS]\n");
    tsfi_io_printf(stdout, "Total Shots:   %d\n", total_shots);
    tsfi_io_printf(stdout, "Total Success: %d\n", success_count);
    tsfi_io_printf(stdout, "Total Time:    %.4f s\n", elapsed);
    tsfi_io_printf(stdout, "Throughput:    %.2f shots/sec\n", (double)success_count / elapsed);

    if (success_count == total_shots) {
        tsfi_io_printf(stdout, "[SUCCESS] 5000 YI Reactions verified with 100%% integrity.\n");
    } else {
        tsfi_io_printf(stderr, "[FAIL] Integrity gaps detected.\n");
        return 1;
    }

    tsfi_bn_free(p_bn);
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
