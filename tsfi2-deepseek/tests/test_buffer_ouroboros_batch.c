#include "tsfi_reaction.h"
#include "tsfi_io.h"
#include "tsfi_dys_math.h"
#include "tsfi_time.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STEPS 1000000

int main() {
    TSFiBigInt* Prime = tsfi_bn_alloc();
    tsfi_bn_set_u64(Prime, 953467954114363ULL); 

    tsfi_io_printf(stdout, "=== TSFi Encrypted Buffer Security: AVX-512 FUSE Ouroboros Test ===\\n");
    tsfi_io_printf(stdout, "Executing 8 parallel Floyd's Cycle-Finding streams for up to %d iterations (10s max)...\\n", MAX_STEPS);

    struct YI* root_yi_batch[8] = {0};
    tsfi_reaction_shoot_batch8_fused(root_yi_batch, Prime);

    TSFiBigInt* t_Xis[8]; TSFiBigInt* t_Rhos[8]; TSFiBigInt* t_Ohms[8];
    TSFiBigInt* h_Xis[8]; TSFiBigInt* h_Rhos[8]; TSFiBigInt* h_Ohms[8];
    struct SHAO* orig_rho_ptrs[8];

    for (int i = 0; i < 8; i++) {
        t_Xis[i] = tsfi_bn_alloc(); t_Rhos[i] = tsfi_bn_alloc(); t_Ohms[i] = tsfi_bn_alloc();
        h_Xis[i] = tsfi_bn_alloc(); h_Rhos[i] = tsfi_bn_alloc(); h_Ohms[i] = tsfi_bn_alloc();
        
        TSFiBigInt* initial_Pi = root_yi_batch[i]->Ring ? root_yi_batch[i]->Ring : root_yi_batch[i]->Xi;
        
        // Slightly mutate initial Pi per stream so they explore entirely different topological sectors
        initial_Pi->limbs[0] ^= (i * 1000);

        struct Dai* root_dai = tsfi_reaction_compute_reciprocity(root_yi_batch[i]->Psi, initial_Pi, NULL);

        tsfi_bn_copy(t_Xis[i], initial_Pi);
        tsfi_bn_copy(t_Rhos[i], root_dai->Daiichi);
        tsfi_bn_copy(t_Ohms[i], root_dai->Ichidai);

        tsfi_bn_copy(h_Xis[i], initial_Pi);
        tsfi_bn_copy(h_Rhos[i], root_dai->Daiichi);
        tsfi_bn_copy(h_Ohms[i], root_dai->Ichidai);
        
        orig_rho_ptrs[i] = root_yi_batch[i]->Psi->Rho;
        freeDAI(root_dai);
    }

    struct YI* t_yi_batch[8] = {0};
    struct YI* h_yi_batch[8] = {0};

    int cycle_detected = 0;
    int cycle_stream_id = -1;
    int steps = 0;
    uint64_t start_time = get_time_ns() / 1000000;

    for (int step = 0; step < MAX_STEPS; step++) {
        // --- Tortoise Step 1 ---
        tsfi_reaction_fuse_batch8_fused(t_yi_batch, t_Xis, t_Rhos, t_Ohms, orig_rho_ptrs, Prime);
        for(int i=0; i<8; i++) {
            TSFiBigInt* Pi = t_yi_batch[i]->Ring ? t_yi_batch[i]->Ring : t_yi_batch[i]->Xi;
            struct Dai* t_dai = tsfi_reaction_compute_reciprocity(t_yi_batch[i]->Psi, Pi, NULL);
            tsfi_bn_copy(t_Xis[i], Pi);
            tsfi_bn_copy(t_Rhos[i], t_dai->Daiichi);
            tsfi_bn_copy(t_Ohms[i], t_dai->Ichidai);
            freeDAI(t_dai);
        }

        // --- Hare Step 1 ---
        tsfi_reaction_fuse_batch8_fused(h_yi_batch, h_Xis, h_Rhos, h_Ohms, orig_rho_ptrs, Prime);
        for(int i=0; i<8; i++) {
            TSFiBigInt* Pi = h_yi_batch[i]->Ring ? h_yi_batch[i]->Ring : h_yi_batch[i]->Xi;
            struct Dai* h_dai = tsfi_reaction_compute_reciprocity(h_yi_batch[i]->Psi, Pi, NULL);
            tsfi_bn_copy(h_Xis[i], Pi);
            tsfi_bn_copy(h_Rhos[i], h_dai->Daiichi);
            tsfi_bn_copy(h_Ohms[i], h_dai->Ichidai);
            freeDAI(h_dai);
        }

        // --- Hare Step 2 ---
        tsfi_reaction_fuse_batch8_fused(h_yi_batch, h_Xis, h_Rhos, h_Ohms, orig_rho_ptrs, Prime);
        for(int i=0; i<8; i++) {
            TSFiBigInt* Pi = h_yi_batch[i]->Ring ? h_yi_batch[i]->Ring : h_yi_batch[i]->Xi;
            struct Dai* h_dai = tsfi_reaction_compute_reciprocity(h_yi_batch[i]->Psi, Pi, NULL);
            tsfi_bn_copy(h_Xis[i], Pi);
            tsfi_bn_copy(h_Rhos[i], h_dai->Daiichi);
            tsfi_bn_copy(h_Ohms[i], h_dai->Ichidai);
            freeDAI(h_dai);
        }

        steps++;

        // Collision Check
        for(int i=0; i<8; i++) {
            if (tsfi_bn_cmp_avx512(t_Xis[i], h_Xis[i]) == 0 &&
                tsfi_bn_cmp_avx512(t_Rhos[i], h_Rhos[i]) == 0 &&
                tsfi_bn_cmp_avx512(t_Ohms[i], h_Ohms[i]) == 0) {
                cycle_detected = 1;
                cycle_stream_id = i;
                break;
            }
        }
        if (cycle_detected) break;

        // Time Check
        if (steps % 10 == 0) {
            if ((get_time_ns() / 1000000) - start_time >= 10000) {
                tsfi_io_printf(stdout, "[TIMEOUT] 10 second maximum reached.\\n");
                break;
            }
        }
    }

    if (cycle_detected) {
        tsfi_io_printf(stderr, "[FAIL] Cryptographic Periodicity Failure! Cycle detected on stream %d at step %d.\\n", cycle_stream_id, steps);
        return 1;
    } else {
        tsfi_io_printf(stdout, "[SUCCESS] No topological loops detected.\\n");
        tsfi_io_printf(stdout, "Completed %d parallel batch steps (Total paths explored: %d) before timeout.\\n", steps, steps * 8);
    }

        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}