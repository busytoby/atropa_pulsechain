#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tsfi_reaction.h"
#include "tsfi_dysnomia.h"
#include "tsfi_math.h"
#include "lau_memory.h"

int main() {
    printf("=== TSFi FUSE Regeneration Verification (Standard Implementation) ===\n");
    
    // 1. Setup
    TSFiBigInt* Prime = tsfi_bn_alloc();
    tsfi_bn_set_u64(Prime, 953473ULL); // Our little prime
    
    // 2. SHOOT (Initial Natural Synthesis)
    printf("[1/3] Executing Natural SHOOT...\n");
    struct YI* shoot_yi = tsfi_reaction_shoot(Prime);
    if (!shoot_yi) { printf("SHOOT Failed.\n"); return 1; }
    
    // 3. CAPTURE (Inputs for FUSE)
    printf("[2/3] Capturing FUSE Parameters...\n");
    TSFiBigInt* Xi = tsfi_bn_alloc(); tsfi_bn_copy(Xi, shoot_yi->Xi);
    TSFiBigInt* Alpha = tsfi_bn_alloc(); tsfi_bn_copy(Alpha, shoot_yi->Psi->Rho->Rod->Mu->Identity);
    TSFiBigInt* Beta = tsfi_bn_alloc(); tsfi_bn_copy(Beta, shoot_yi->Psi->Rho->Cone->Mu->Identity);
    
    // Store the starting secrets (DNA)
    struct SHAO* orig_rho = allocSHAO();
    orig_rho->Rod = allocSHA(); orig_rho->Rod->Mu = allocFa();
    orig_rho->Cone = allocSHA(); orig_rho->Cone->Mu = allocFa();
    
    tsfi_bn_copy(orig_rho->Rod->Mu->Base, shoot_yi->Psi->Rho->Rod->Mu->Base);
    tsfi_bn_copy(orig_rho->Rod->Mu->Secret, shoot_yi->Psi->Rho->Rod->Mu->Secret);
    tsfi_bn_copy(orig_rho->Rod->Mu->Signal, shoot_yi->Psi->Rho->Rod->Mu->Signal);
    
    tsfi_bn_copy(orig_rho->Cone->Mu->Base, shoot_yi->Psi->Rho->Cone->Mu->Base);
    tsfi_bn_copy(orig_rho->Cone->Mu->Secret, shoot_yi->Psi->Rho->Cone->Mu->Secret);
    tsfi_bn_copy(orig_rho->Cone->Mu->Signal, shoot_yi->Psi->Rho->Cone->Mu->Signal);

    // 4. FUSE (Regeneration)
    printf("[3/3] Executing Deterministic FUSE...\n");
    // Signature: tsfi_reaction_fuse(Xi, Beta, Alpha, orig_rho, Prime)
    struct YI* fuse_yi = tsfi_reaction_fuse(Xi, Beta, Alpha, orig_rho, Prime);
    
    // 5. VERIFY
    uint64_t s_ring = 0, f_ring = 0;
    tsfi_bn_get_u64(shoot_yi->Ring, &s_ring);
    tsfi_bn_get_u64(fuse_yi->Ring, &f_ring);
    
    printf("\nResults:\n");
    printf("  SHOOT Ring: %lu\n", s_ring);
    printf("  FUSE  Ring: %lu\n", f_ring);
    
    if (s_ring == f_ring && s_ring != 0) {
        printf("\n[PASS] FUSE successfully regenerated the same trilateral anchor!\n");
    } else {
        printf("\n[FAIL] FUSE Divergence detected.\n");
        // Detailed check
        return 1;
    }
    
    // Teardown
    tsfi_bn_free(Prime); tsfi_bn_free(Xi); tsfi_bn_free(Alpha); tsfi_bn_free(Beta);
    // freeYI calls freeSHIO which tries to free the members
    // but shoot_yi->Psi->Rho->Rod is actually shared or was allocated?
    // In src/tsfi_reaction.c STAT allocates new Fa.
    return 0;
}
