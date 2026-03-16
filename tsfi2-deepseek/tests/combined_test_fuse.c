#include "tsfi_reaction.c"
#include "tsfi_reaction.h"
#include "tsfi_io.h"
#include "tsfi_dys_math.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static struct YI* local_fuse(TSFiBigInt* Xi, TSFiBigInt* Rho, TSFiBigInt* Ohm, TSFiBigInt* Prime) {
    struct SHIO* B = allocSHIO(); struct SHAO* A = allocSHAO(); struct YI* R = allocYI();
    memset(B, 0, sizeof(struct SHIO)); memset(A, 0, sizeof(struct SHAO)); memset(R, 0, sizeof(struct YI));
    A->Rod = STAT(Prime); A->Cone = STAT(Prime); B->Rho = A; R->Psi = B;
    R->Xi = tsfi_bn_realloc(R->Xi); tsfi_bn_copy(R->Xi, Xi);
    
    // FUSE: Ohm is zD (Rod), Rho is zI (Cone)
    GENERATE(R->Psi, R->Xi, Ohm, Rho, Prime); 
    IONIZE(R->Psi);
    TSFiBigInt *t1 = tsfi_bn_alloc(), *t2 = tsfi_bn_alloc(), *t3 = tsfi_bn_alloc();
    R->Ring = MAGNETIZE_reused(R->Psi, Prime, t1, t2, t3);
    tsfi_bn_free(t1); tsfi_bn_free(t2); tsfi_bn_free(t3);
    return R;
}

int main() {
    TSFiBigInt* Prime = tsfi_bn_alloc();
    tsfi_bn_set_u64(Prime, 953467954114363ULL);

    tsfi_io_printf(stdout, "[FUSE] Testing Dysnomia FUSE properties...\n");

    struct YI* orig = tsfi_reaction_shoot(Prime);
    TSFiBigInt* Pi = orig->Xi;
    struct Dai* orig_dai = tsfi_reaction_compute_reciprocity(orig->Psi, Pi, NULL);

    struct YI* fused = local_fuse(Pi, orig_dai->Daiichi, orig_dai->Ichidai, Prime);
    TSFiBigInt* fused_Pi = fused->Xi;
    struct Dai* fused_dai = tsfi_reaction_compute_reciprocity(fused->Psi, fused_Pi, NULL);

    tsfi_io_printf(stdout, "Orig Dai Ichidai: %lu\n", orig_dai->Ichidai->limbs[0]);
    tsfi_io_printf(stdout, "Fused Dai Ichidai: %lu\n", fused_dai->Ichidai->limbs[0]);
    tsfi_io_printf(stdout, "Orig Dai Daiichi: %lu\n", orig_dai->Daiichi->limbs[0]);
    tsfi_io_printf(stdout, "Fused Dai Daiichi: %lu\n", fused_dai->Daiichi->limbs[0]);

    if (tsfi_bn_cmp_avx512(orig_dai->Ichidai, fused_dai->Ichidai) != 0 ||
        tsfi_bn_cmp_avx512(orig_dai->Daiichi, fused_dai->Daiichi) != 0) {
        tsfi_io_printf(stderr, "[FAIL] FUSE did not produce identical ReactSHIO results!\n");
        return 1;
    }

    tsfi_io_printf(stdout, "[SUCCESS] FUSE mathematical property verified.\n");
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
