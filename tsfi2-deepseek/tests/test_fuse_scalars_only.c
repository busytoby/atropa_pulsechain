#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tsfi_reaction.h"
#include "tsfi_dysnomia.h"
#include "tsfi_math.h"
#include "lau_memory.h"

int verify_bn(TSFiBigInt* a, TSFiBigInt* b) {
    if (!a && !b) return 1;
    if (!a || !b) return 0;
    if (a->active_limbs != b->active_limbs) return 0;
    for (int i = 0; i < a->active_limbs; i++) {
        if (a->limbs[i] != b->limbs[i]) return 0;
    }
    return 1;
}

int main() {
    printf("=== TSFi FUSE Scalar-Only Verification ===\n");
    
    TSFiBigInt* Prime = tsfi_bn_alloc();
    tsfi_bn_set_u64(Prime, 953473ULL); 
    
    // Original state
    struct YI* shoot_yi = tsfi_reaction_shoot(Prime);
    
    // Extract only the scalars. No orig_rho.
    TSFiBigInt* Xi = tsfi_bn_alloc(); tsfi_bn_copy(Xi, shoot_yi->Xi);
    TSFiBigInt* Alpha = tsfi_bn_alloc(); tsfi_bn_copy(Alpha, shoot_yi->Psi->Rho->Rod->Mu->Identity);
    TSFiBigInt* Beta = tsfi_bn_alloc(); tsfi_bn_copy(Beta, shoot_yi->Psi->Rho->Cone->Mu->Identity);
    
    // Fuse with NULL orig_rho
    struct YI* fuse_yi = tsfi_reaction_fuse(Xi, Beta, Alpha, NULL, Prime);
    
    if (verify_bn(shoot_yi->Ring, fuse_yi->Ring)) {
        printf("[PASS] FUSE with only Xi, Alpha, Beta, Prime successfully reconstructed the state.\n");
    } else {
        printf("[FAIL] FUSE without orig_rho generated a different state.\n");
        printf("Shoot Ring: %lu\n", shoot_yi->Ring->limbs[0]);
        printf("Fuse  Ring: %lu\n", fuse_yi->Ring->limbs[0]);
        return 1;
    }
    
    return 0;
}
