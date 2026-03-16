#include <unistd.h>
#define _POSIX_C_SOURCE 200809L
#include "tsfi_reaction.h"
#include "tsfi_io.h"
#include "tsfi_dys_math.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    alarm(5);
    TSFiBigInt* Prime = tsfi_bn_alloc();
    tsfi_bn_set_u64(Prime, 953467954114363ULL); 

    tsfi_io_printf(stdout, "--- FUSE VERIFICATION ---\n");

    for (int i = 1; i <= 3; i++) {
        // Generate base structure via SHOOT
        struct YI* yi = tsfi_reaction_shoot(Prime);
        
        TSFiBigInt* orig_Xi = yi->Xi;
        TSFiBigInt* orig_zD = yi->Psi->Rho->Rod->Mu->Identity;
        TSFiBigInt* orig_zI = yi->Psi->Rho->Cone->Mu->Identity;

        // Feed the random noise vectors back into FUSE to verify deterministic lock
        struct YI* fused_yi = tsfi_reaction_fuse(orig_Xi, orig_zI, orig_zD, yi->Psi->Rho, Prime);

        TSFiBigInt* orig_Pi = yi->Ring ? yi->Ring : yi->Xi;
        struct Dai* orig_dai = tsfi_reaction_compute_reciprocity(yi->Psi, orig_Pi, NULL);

        TSFiBigInt* fused_Pi = fused_yi->Ring ? fused_yi->Ring : fused_yi->Xi;
        struct Dai* fused_dai = tsfi_reaction_compute_reciprocity(fused_yi->Psi, fused_Pi, NULL);

        int match = 1;
        
        if (tsfi_bn_cmp_avx512(yi->Psi->Rho->Rod->Mu->Identity, fused_yi->Psi->Rho->Rod->Mu->Identity) != 0) match = 0;
        if (tsfi_bn_cmp_avx512(yi->Psi->Rho->Rod->Mu->Foundation, fused_yi->Psi->Rho->Rod->Mu->Foundation) != 0) match = 0;
        if (tsfi_bn_cmp_avx512(yi->Psi->Rho->Cone->Mu->Identity, fused_yi->Psi->Rho->Cone->Mu->Identity) != 0) match = 0;
        if (tsfi_bn_cmp_avx512(yi->Psi->Rho->Cone->Mu->Foundation, fused_yi->Psi->Rho->Cone->Mu->Foundation) != 0) match = 0;

        // Verify the React output matches
        if (tsfi_bn_cmp_avx512(orig_dai->Ichidai, fused_dai->Ichidai) != 0) match = 0;
        if (tsfi_bn_cmp_avx512(orig_dai->Daiichi, fused_dai->Daiichi) != 0) match = 0;

        if (match) {
            tsfi_io_printf(stdout, "[RECORD %d] FUSE Validation: PASS (Identities, Foundations, and Dai locked)\n", i);
            tsfi_io_printf(stdout, "  -> Orig  DAI.Ichidai: %lu\n", orig_dai->Ichidai->limbs[0]);
            tsfi_io_printf(stdout, "  -> Fused DAI.Ichidai: %lu\n", fused_dai->Ichidai->limbs[0]);
        } else {
            tsfi_io_printf(stdout, "[RECORD %d] FUSE Validation: FAIL (Structural deviation detected)\n", i);
        }

        freeDAI(orig_dai);
        freeDAI(fused_dai);
        freeYI(fused_yi);
        freeYI(yi);
    }

    tsfi_bn_free(Prime);
    
    extern void tsfi_bn_drain_pool(void);
    extern void tsfi_dysnomia_drain_pools(void);
    tsfi_bn_drain_pool();
    tsfi_dysnomia_drain_pools();
    
    extern void tsfi_dl_thunks_teardown(void);
    tsfi_dl_thunks_teardown();
    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    
    extern void lau_report_memory_metrics(void);
    extern void lau_free_all_active(void);
    lau_free_all_active();
    lau_report_memory_metrics();
    extern void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr);
    lau_assert_zero_unsealed_leaks("test", NULL);
    return 0;
}
