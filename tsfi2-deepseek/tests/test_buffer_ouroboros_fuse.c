#include "tsfi_reaction.h"
#include "tsfi_io.h"
#include "tsfi_dys_math.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tsfi_time.h"

#define MAX_STEPS 1000000

int main() {
    TSFiBigInt* Prime = tsfi_bn_alloc();
    tsfi_bn_set_u64(Prime, 953467954114363ULL); 

    tsfi_io_printf(stdout, "=== TSFi Encrypted Buffer Security: Sequential FUSE Ouroboros Test ===\n");
    tsfi_io_printf(stdout, "Executing Floyd's Cycle-Finding Algorithm for %d iterations (10s max)...\n", MAX_STEPS);

    struct YI* root_yi = tsfi_reaction_shoot(Prime);

    TSFiBigInt* tortoise_Xi = tsfi_bn_alloc();
    TSFiBigInt* tortoise_Rho = tsfi_bn_alloc();
    TSFiBigInt* tortoise_Ohm = tsfi_bn_alloc();

    TSFiBigInt* hare_Xi = tsfi_bn_alloc();
    TSFiBigInt* hare_Rho = tsfi_bn_alloc();
    TSFiBigInt* hare_Ohm = tsfi_bn_alloc();

    TSFiBigInt* initial_Pi = root_yi->Ring ? root_yi->Ring : root_yi->Xi;
    struct Dai* root_dai = tsfi_reaction_compute_reciprocity(root_yi->Psi, initial_Pi, NULL);

    tsfi_bn_copy(tortoise_Xi, initial_Pi);
    tsfi_bn_copy(tortoise_Rho, root_dai->Daiichi);
    tsfi_bn_copy(tortoise_Ohm, root_dai->Ichidai);

    tsfi_bn_copy(hare_Xi, initial_Pi);
    tsfi_bn_copy(hare_Rho, root_dai->Daiichi);
    tsfi_bn_copy(hare_Ohm, root_dai->Ichidai);

    freeDAI(root_dai);

    int cycle_detected = 0;
    int steps = 0;
    uint64_t start_time = get_time_ns() / 1000000;

    for (int i = 0; i < MAX_STEPS; i++) {
        // --- Tortoise steps 1 ---
        struct YI* t_yi = tsfi_reaction_fuse(tortoise_Xi, tortoise_Rho, tortoise_Ohm, root_yi->Psi->Rho, Prime);
        TSFiBigInt* t_Pi = t_yi->Ring ? t_yi->Ring : t_yi->Xi;
        struct Dai* t_dai = tsfi_reaction_compute_reciprocity(t_yi->Psi, t_Pi, NULL);
        
        tsfi_bn_copy(tortoise_Xi, t_Pi);
        tsfi_bn_copy(tortoise_Rho, t_dai->Daiichi);
        tsfi_bn_copy(tortoise_Ohm, t_dai->Ichidai);
        
        freeDAI(t_dai);
        freeYI(t_yi);

        // --- Hare steps 2 ---
        struct YI* h_yi_1 = tsfi_reaction_fuse(hare_Xi, hare_Rho, hare_Ohm, root_yi->Psi->Rho, Prime);
        TSFiBigInt* h_Pi_1 = h_yi_1->Ring ? h_yi_1->Ring : h_yi_1->Xi;
        struct Dai* h_dai_1 = tsfi_reaction_compute_reciprocity(h_yi_1->Psi, h_Pi_1, NULL);
        
        tsfi_bn_copy(hare_Xi, h_Pi_1);
        tsfi_bn_copy(hare_Rho, h_dai_1->Daiichi);
        tsfi_bn_copy(hare_Ohm, h_dai_1->Ichidai);
        
        freeDAI(h_dai_1);
        freeYI(h_yi_1);

        struct YI* h_yi_2 = tsfi_reaction_fuse(hare_Xi, hare_Rho, hare_Ohm, root_yi->Psi->Rho, Prime);
        TSFiBigInt* h_Pi_2 = h_yi_2->Ring ? h_yi_2->Ring : h_yi_2->Xi;
        struct Dai* h_dai_2 = tsfi_reaction_compute_reciprocity(h_yi_2->Psi, h_Pi_2, NULL);
        
        tsfi_bn_copy(hare_Xi, h_Pi_2);
        tsfi_bn_copy(hare_Rho, h_dai_2->Daiichi);
        tsfi_bn_copy(hare_Ohm, h_dai_2->Ichidai);

        freeDAI(h_dai_2);
        freeYI(h_yi_2);

        steps++;
        if (tsfi_bn_cmp_avx512(tortoise_Xi, hare_Xi) == 0 &&
            tsfi_bn_cmp_avx512(tortoise_Rho, hare_Rho) == 0 &&
            tsfi_bn_cmp_avx512(tortoise_Ohm, hare_Ohm) == 0) {
            cycle_detected = 1;
            break;
        }

        if (steps % 100 == 0) {
            if ((get_time_ns() / 1000000) - start_time >= 10000) {
                tsfi_io_printf(stdout, "[TIMEOUT] 10 second maximum reached.\n");
                break;
            }
        }
    }

    if (cycle_detected) {
        tsfi_io_printf(stderr, "[FAIL] Cryptographic Periodicity Failure! Cycle detected at step %d.\n", steps);
        return 1;
    } else {
        tsfi_io_printf(stdout, "[SUCCESS] No topological loops detected. Completed %d unique FUSE chaining iterations before timeout.\n", steps);
    }

    tsfi_bn_free(tortoise_Xi); tsfi_bn_free(tortoise_Rho); tsfi_bn_free(tortoise_Ohm);
    tsfi_bn_free(hare_Xi); tsfi_bn_free(hare_Rho); tsfi_bn_free(hare_Ohm);
    freeYI(root_yi);
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}