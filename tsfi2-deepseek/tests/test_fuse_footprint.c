#include "tsfi_reaction.h"
#include "tsfi_io.h"
#include "tsfi_dys_math.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_fa(const char* prefix, struct Fa* F) {
    if (!F) return;
    tsfi_io_printf(stdout, "%s.Base: %lu\n", prefix, F->Base ? F->Base->limbs[0] : 0);
    tsfi_io_printf(stdout, "%s.Secret: %lu\n", prefix, F->Secret ? F->Secret->limbs[0] : 0);
    tsfi_io_printf(stdout, "%s.Signal: %lu\n", prefix, F->Signal ? F->Signal->limbs[0] : 0);
    tsfi_io_printf(stdout, "%s.Channel: %lu\n", prefix, F->Channel ? F->Channel->limbs[0] : 0);
    tsfi_io_printf(stdout, "%s.Coordinate: %lu\n", prefix, F->Coordinate ? F->Coordinate->limbs[0] : 0);
    tsfi_io_printf(stdout, "%s.Contour: %lu\n", prefix, F->Contour ? F->Contour->limbs[0] : 0);
    tsfi_io_printf(stdout, "%s.Pole: %lu\n", prefix, F->Pole ? F->Pole->limbs[0] : 0);
    tsfi_io_printf(stdout, "%s.Identity: %lu\n", prefix, F->Identity ? F->Identity->limbs[0] : 0);
    tsfi_io_printf(stdout, "%s.Foundation: %lu\n", prefix, F->Foundation ? F->Foundation->limbs[0] : 0);
    tsfi_io_printf(stdout, "%s.Charge: %lu\n", prefix, F->Charge ? F->Charge->limbs[0] : 0);
    tsfi_io_printf(stdout, "%s.Limit: %lu\n", prefix, F->Limit ? F->Limit->limbs[0] : 0);
    tsfi_io_printf(stdout, "%s.Element: %lu\n", prefix, F->Element ? F->Element->limbs[0] : 0);
    tsfi_io_printf(stdout, "%s.Monopole: %lu\n", prefix, F->Monopole ? F->Monopole->limbs[0] : 0);
}

static void print_sha(const char* prefix, struct SHA* A) {
    if (!A) return;
    tsfi_io_printf(stdout, "%s.Dynamo: %lu\n", prefix, A->Dynamo ? A->Dynamo->limbs[0] : 0);
    char buf[128]; snprintf(buf, sizeof(buf), "%s.Mu", prefix);
    print_fa(buf, A->Mu);
}

static void print_yi(const char* name, struct YI* Y) {
    tsfi_io_printf(stdout, "\n--- YI DATA MEMBERS: %s ---\n", name);
    tsfi_io_printf(stdout, "Xi: %lu\n", Y->Xi ? Y->Xi->limbs[0] : 0);
    tsfi_io_printf(stdout, "Ring: %lu\n", Y->Ring ? Y->Ring->limbs[0] : 0);
    if (Y->Psi) {
        tsfi_io_printf(stdout, "Psi.Manifold: %lu\n", Y->Psi->Manifold ? Y->Psi->Manifold->limbs[0] : 0);
        tsfi_io_printf(stdout, "Psi.Monopole: %lu\n", Y->Psi->Monopole ? Y->Psi->Monopole->limbs[0] : 0);
        if (Y->Psi->Rho) {
            tsfi_io_printf(stdout, "Psi.Rho.Barn: %lu\n", Y->Psi->Rho->Barn ? Y->Psi->Rho->Barn->limbs[0] : 0);
            print_sha("Psi.Rho.Rod", Y->Psi->Rho->Rod);
            print_sha("Psi.Rho.Cone", Y->Psi->Rho->Cone);
        }
    }
    tsfi_io_printf(stdout, "----------------------------\n\n");
}

int main() {
    TSFiBigInt* Prime = tsfi_bn_alloc();
    tsfi_bn_set_u64(Prime, 953467954114363ULL);

    tsfi_io_printf(stdout, "[FUSE] Verifying minimal footprint consistency...\n");

    struct YI* full_yi = tsfi_reaction_shoot(Prime);
    TSFiBigInt* Pi = full_yi->Ring ? full_yi->Ring : full_yi->Xi;
    struct Dai* orig_dai = tsfi_reaction_compute_reciprocity(full_yi->Psi, Pi, NULL);

    tsfi_io_printf(stdout, "[FUSE] Extracted original random variables: Xi, zD (Ohm), zI (Rho)...\n");
    TSFiBigInt* orig_Xi = full_yi->Xi;
    TSFiBigInt* orig_zD = full_yi->Psi->Rho->Rod->Mu->Identity;
    TSFiBigInt* orig_zI = full_yi->Psi->Rho->Cone->Mu->Identity;

    tsfi_io_printf(stdout, "[FUSE] Fusing new YI with original values and STAT state natively...\n");
    struct YI* fused_yi = tsfi_reaction_fuse(orig_Xi, orig_zI, orig_zD, full_yi->Psi->Rho, Prime);

    print_yi("ORIGINAL_YI", full_yi);
    print_yi("FUSED_YI", fused_yi);

    // Call ReactSHIO on the fused_yi with the SAME Pi (which is full_yi->Ring)
    struct Dai* fused_dai = tsfi_reaction_compute_reciprocity(fused_yi->Psi, Pi, NULL);

    tsfi_io_printf(stdout, "[FUSE] Original Dai: Ichidai %lu, Daiichi %lu\n", orig_dai->Ichidai->limbs[0], orig_dai->Daiichi->limbs[0]);
    tsfi_io_printf(stdout, "[FUSE] Fused Dai: Ichidai %lu, Daiichi %lu\n", fused_dai->Ichidai->limbs[0], fused_dai->Daiichi->limbs[0]);

    if (tsfi_bn_cmp_avx512(orig_dai->Ichidai, fused_dai->Ichidai) != 0 ||
        tsfi_bn_cmp_avx512(orig_dai->Daiichi, fused_dai->Daiichi) != 0) {
        tsfi_io_printf(stderr, "[FAIL] FUSE did not produce identical Dai results!\n");
        return 1;
    }

    tsfi_io_printf(stdout, "[SUCCESS] FUSE property perfectly verified.\n");
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}