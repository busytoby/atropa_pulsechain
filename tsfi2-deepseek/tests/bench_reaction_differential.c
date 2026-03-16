#include "tsfi_reaction.h"
#include "tsfi_io.h"
#include "tsfi_dys_math.h"
#include "tsfi_time.h"
#include "lau_memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BATCH_COUNT 12500

int main() {
    TSFiBigInt* Prime = tsfi_bn_alloc();
    tsfi_bn_set_u64(Prime, 953467954114363ULL);

    int total_shots = BATCH_COUNT * 8;
    tsfi_io_printf(stdout, "=== TSFi Reaction Differential Benchmark (%d Iterations) ===\n", total_shots);
    
    struct YI* yi_batch[8] = {0};
    
    uint64_t start_shoot = get_time_ns() / 1000000;
    for (int b = 0; b < BATCH_COUNT; b++) {
        tsfi_reaction_shoot_batch8_fused(yi_batch, Prime);
    }
    uint64_t end_shoot = get_time_ns() / 1000000;
    
    TSFiBigInt* Xis[8] = {0};
    TSFiBigInt* Ohms[8] = {0};
    TSFiBigInt* Rhos[8] = {0};
    struct SHAO* rho_ptrs[8] = {0};
    for(int i=0; i<8; i++) {
        Xis[i] = yi_batch[i]->Xi;
        Ohms[i] = yi_batch[i]->Psi->Rho->Rod->Mu->Identity;
        Rhos[i] = yi_batch[i]->Psi->Rho->Cone->Mu->Identity;
        rho_ptrs[i] = yi_batch[i]->Psi->Rho;
    }
    
    struct YI* fuse_batch[8] = {0};
    
    uint64_t start_fuse = get_time_ns() / 1000000;
    for (int b = 0; b < BATCH_COUNT; b++) {
        tsfi_reaction_fuse_batch8_fused(fuse_batch, Xis, Rhos, Ohms, rho_ptrs, Prime);
    }
    uint64_t end_fuse = get_time_ns() / 1000000;
    
    struct Dai* dai_batch[8] = {0};

    uint64_t start_react = get_time_ns() / 1000000;
    for (int b = 0; b < BATCH_COUNT; b++) {
        for(int i=0; i<8; i++) {
            TSFiBigInt* Pi = fuse_batch[i]->Ring ? fuse_batch[i]->Ring : fuse_batch[i]->Xi;
            dai_batch[i] = tsfi_reaction_compute_reciprocity(fuse_batch[i]->Psi, Pi, NULL);
            freeDAI(dai_batch[i]);
        }
    }
    uint64_t end_react = get_time_ns() / 1000000;

    double d_shoot = (double)(end_shoot - start_shoot);
    if (d_shoot == 0) d_shoot = 1;
    double d_fuse = (double)(end_fuse - start_fuse);
    if (d_fuse == 0) d_fuse = 1;
    double d_react = (double)(end_react - start_react);
    if (d_react == 0) d_react = 1;

    tsfi_io_printf(stdout, "\n[RESULTS]\n");
    tsfi_io_printf(stdout, "1. SHOOT (Generative)    : %8.2f ms | %9.2f Ops/sec\n", d_shoot, (double)total_shots / (d_shoot / 1000.0));
    tsfi_io_printf(stdout, "2. FUSE  (Reconstructive): %8.2f ms | %9.2f Ops/sec\n", d_fuse, (double)total_shots / (d_fuse / 1000.0));
    tsfi_io_printf(stdout, "3. REACT (Observation)   : %8.2f ms | %9.2f Ops/sec\n", d_react, (double)total_shots / (d_react / 1000.0));
    
    for(int i=0; i<8; i++) {
        if (!fuse_batch[i] || !fuse_batch[i]->Psi || !fuse_batch[i]->Ring) {
            tsfi_io_printf(stderr, "[ERROR] FUSE final batch structural integrity compromised.\n");
            return 1;
        }
    }

    tsfi_io_printf(stdout, "\n[SUCCESS] Differential benchmark complete.\n");
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}