#include "tsfi_reaction.h"
#include "tsfi_io.h"
#include "tsfi_dys_math.h"
#include "tsfi_time.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    TSFiBigInt* Prime = tsfi_bn_alloc();
    tsfi_bn_set_u64(Prime, 953467954114363ULL);

    tsfi_io_printf(stdout, "[BENCH-FUSE-BATCH] Benchmarking 5000 SHOOT -> FUSE -> VERIFY cycles using AVX-512 batches...\n");
    
    int batch_count = 5000 / 8; // 625 batches of 8 = 5000
    int total_shots = batch_count * 8;
    int success_count = 0;
    
    struct YI* orig_yi_batch[8] = {0};
    struct YI* fused_yi_batch[8] = {0};
    struct Dai* orig_dai_batch[8] = {0};
    struct Dai* fused_dai_batch[8] = {0};

    TSFiBigInt* orig_Xis[8] = {0};
    TSFiBigInt* orig_Ohms[8] = {0};
    TSFiBigInt* orig_Rhos[8] = {0};
    struct SHAO* orig_rho_ptrs[8] = {0};
    
    uint64_t start_time = get_time_ns() / 1000000;
    
    for (int b = 0; b < batch_count; b++) {
        // Shoot 8
        tsfi_reaction_shoot_batch8_fused(orig_yi_batch, Prime);
        
        // Extract parameters
        for(int i=0; i<8; i++) {
            TSFiBigInt* Pi = orig_yi_batch[i]->Ring ? orig_yi_batch[i]->Ring : orig_yi_batch[i]->Xi;
            orig_dai_batch[i] = tsfi_reaction_compute_reciprocity(orig_yi_batch[i]->Psi, Pi, NULL);
            orig_Xis[i] = orig_yi_batch[i]->Xi;
            orig_Ohms[i] = orig_yi_batch[i]->Psi->Rho->Rod->Mu->Identity;
            orig_Rhos[i] = orig_yi_batch[i]->Psi->Rho->Cone->Mu->Identity;
            orig_rho_ptrs[i] = orig_yi_batch[i]->Psi->Rho;
        }

        // Fuse 8
        tsfi_reaction_fuse_batch8_fused(fused_yi_batch, orig_Xis, orig_Rhos, orig_Ohms, orig_rho_ptrs, Prime);

        // Verify 8
        for(int i=0; i<8; i++) {
            TSFiBigInt* Pi = fused_yi_batch[i]->Ring ? fused_yi_batch[i]->Ring : fused_yi_batch[i]->Xi;
            fused_dai_batch[i] = tsfi_reaction_compute_reciprocity(fused_yi_batch[i]->Psi, Pi, NULL);
            
            if (tsfi_bn_cmp_avx512(orig_dai_batch[i]->Ichidai, fused_dai_batch[i]->Ichidai) == 0 &&
                tsfi_bn_cmp_avx512(orig_dai_batch[i]->Daiichi, fused_dai_batch[i]->Daiichi) == 0) {
                success_count++;
            }
            freeDAI(orig_dai_batch[i]);
            freeDAI(fused_dai_batch[i]);
            // Free the structs inside YI but keep the YI pointers for the next batch reuse.
            // Oh, we should probably let them leak for speed or clean them?
            // Since allocYI/allocSHIO might be reused, let's keep them and they will be reused in the next tsfi_reaction_shoot_batch8_fused.
        }
    }
    
    uint64_t end_time = get_time_ns() / 1000000;
    double duration = (double)(end_time - start_time);
    if (duration == 0) duration = 1;
    
    tsfi_io_printf(stdout, "Completed %d cycles in %.2f ms\n", total_shots, duration);
    tsfi_io_printf(stdout, "Cycle throughput: %.2f cycles/sec\n", (double)total_shots / (duration / 1000.0));
    tsfi_io_printf(stdout, "Successful footprint verifications: %d/%d\n", success_count, total_shots);

    if (success_count != total_shots) {
        tsfi_io_printf(stderr, "[FAIL] Not all footprint verifications succeeded!\n");
        return 1;
    }

    tsfi_io_printf(stdout, "[SUCCESS] All %d AVX-512 FUSE property checks passed.\n", total_shots);
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}