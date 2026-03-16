#include "tsfi_reaction.h"
#include "tsfi_dysnomia.h"
#include "lau_memory.h"
#include "lau_audit.h"
#include "tsfi_io.h"
#include "tsfi_dys_math.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void test_yi_reaction_shoot() {
    tsfi_io_printf(stdout, "\n=== TSFi YI Reaction Shoot Test ===\n");
    tsfi_io_printf(stdout, "[SHOOT-YI] Initiating Advanced Reaction (YI) Shot...\n");

    TSFiBigInt *p_bn = tsfi_bn_alloc();
    tsfi_bn_set_u64(p_bn, 953467954114363ULL);

    struct YI* yi = tsfi_reaction_shoot(p_bn);

    if (!yi) {
        tsfi_io_printf(stderr, "[FAIL] Reaction failed to produce YI.\n");
        exit(1);
    }

    tsfi_io_printf(stdout, "[YI] Manifold: %lu\n", yi->Psi->Manifold->limbs[0]);
    
    if (yi->Ring) {
        tsfi_io_printf(stdout, "[YI] Ring:     %lu\n", yi->Ring->limbs[0]);
    } else {
        tsfi_io_printf(stdout, "[YI] Ring:     (NULL - Using Xi as fallback)\n");
    }

    if (yi->Psi->Monopole) {
        tsfi_io_printf(stdout, "[YI] Monopole: %lu\n", yi->Psi->Monopole->limbs[0]);
    }

    TSFiBigInt *pi_bn = (yi->Ring && yi->Ring->active_limbs > 0) ? yi->Ring : yi->Xi;
    struct Dai* dai = tsfi_reaction_compute_reciprocity(yi->Psi, pi_bn, NULL);

    if (dai) {
        tsfi_io_printf(stdout, "[DAI] Ichidai: %lu\n", dai->Ichidai->limbs[0]);
        tsfi_io_printf(stdout, "[DAI] Daiichi: %lu\n", dai->Daiichi->limbs[0]);
        tsfi_io_printf(stdout, "[SUCCESS] YI Reaction confirmed valid.\n");
    } else {
        tsfi_io_printf(stderr, "[FAIL] Reciprocity verification failed.\n");
        exit(1);
    }
    
    freeDAI(dai);
    freeYI(yi);
    tsfi_bn_free(p_bn);
}

int main() {
    // The legacy "manual construction" using string fields was rendered obsolete by the new BigInt representation.
    // The test will now solely verify the modern dynamic reaction construction logic from test_shoot_yi.c
    test_yi_reaction_shoot();
    
    extern void tsfi_bn_drain_pool(void);
    extern void tsfi_dysnomia_drain_pools(void);
    tsfi_bn_drain_pool();
    tsfi_dysnomia_drain_pools();
    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
