#include "tsfi_reaction.h"
#include "tsfi_io.h"
#include "tsfi_dys_math.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void extract_3_unique_dai(TSFiBigInt* Prime) {
    tsfi_io_printf(stdout, "\n=== TSFi Cryptographic Extraction: 3 Unique DAI ===\n");

    for (int i = 1; i <= 3; i++) {
        struct YI* yi = tsfi_reaction_shoot(Prime);
        
        TSFiBigInt* Pi = yi->Ring ? yi->Ring : yi->Xi;
        struct Dai* dai = tsfi_reaction_compute_reciprocity(yi->Psi, Pi, NULL);

        tsfi_io_printf(stdout, "\n--- DAI EXTRACTION RECORD %d ---\n", i);
        
        tsfi_io_printf(stdout, "DAI.Ichidai: %lu\n", dai->Ichidai->limbs[0]);
        tsfi_io_printf(stdout, "DAI.Daiichi: %lu\n", dai->Daiichi->limbs[0]);
        
        tsfi_io_printf(stdout, "Rod.Identity: %lu\n", yi->Psi->Rho->Rod->Mu->Identity->limbs[0]);
        tsfi_io_printf(stdout, "Rod.Foundation: %lu\n", yi->Psi->Rho->Rod->Mu->Foundation->limbs[0]);
        
        tsfi_io_printf(stdout, "Cone.Identity: %lu\n", yi->Psi->Rho->Cone->Mu->Identity->limbs[0]);
        tsfi_io_printf(stdout, "Cone.Foundation: %lu\n", yi->Psi->Rho->Cone->Mu->Foundation->limbs[0]);

        freeDAI(dai);
        freeYI(yi);
    }
}

int main() {
    TSFiBigInt* Prime = tsfi_bn_alloc();
    tsfi_bn_set_u64(Prime, 953467954114363ULL); 

    extract_3_unique_dai(Prime);

    tsfi_io_printf(stdout, "\n=== TSFi Cryptographic Extraction: 5 Sequential DAI ===\n");

    struct YI* yi = tsfi_reaction_shoot(Prime);

    // Initial Pi is a compatible random number modulo Prime
    TSFiBigInt* Pi = RANDOM_bn(Prime);

    for (int i = 1; i <= 5; i++) {
        struct Dai* dai = tsfi_reaction_compute_reciprocity(yi->Psi, Pi, NULL);

        tsfi_io_printf(stdout, "\n--- DAI EXTRACTION RECORD %d ---\n", i);
        tsfi_io_printf(stdout, "Pi Input: %lu\n", Pi->limbs[0]);
        tsfi_io_printf(stdout, "DAI.Ichidai: %lu\n", dai->Ichidai->limbs[0]);
        tsfi_io_printf(stdout, "DAI.Daiichi: %lu\n", dai->Daiichi->limbs[0]);
        
        // Feed Daiichi back into Pi for the next sequential iteration
        tsfi_bn_copy(Pi, dai->Daiichi);
        freeDAI(dai);
    }

    tsfi_io_printf(stdout, "\n=== Extraction Complete ===\n");

    tsfi_bn_free(Pi);
    freeYI(yi);
    tsfi_bn_free(Prime);
    
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