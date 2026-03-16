#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tsfi_reaction.h"
#include "tsfi_dysnomia.h"
#include "tsfi_math.h"
#include "lau_memory.h"

int main() {
    printf("=== TSFi FUSE Taste Signature Verification ===\n");
    
    // 1. Setup
    TSFiBigInt* Prime = tsfi_bn_alloc();
    tsfi_bn_set_u64(Prime, 953473ULL); 
    
    // 2. SHOOT (Initial Natural Synthesis)
    printf("[1/4] Executing Natural SHOOT (Original Taste)...\n");
    struct YI* shoot_yi = tsfi_reaction_shoot(Prime);
    if (!shoot_yi) { printf("SHOOT Failed.\n"); return 1; }
    
    // 3. CAPTURE (Inputs for FUSE - The Private Key)
    printf("[2/4] Extracting Private FUSE Parameters...\n");
    TSFiBigInt* Xi = tsfi_bn_alloc(); tsfi_bn_copy(Xi, shoot_yi->Xi);
    TSFiBigInt* Alpha = tsfi_bn_alloc(); tsfi_bn_copy(Alpha, shoot_yi->Psi->Rho->Rod->Mu->Identity);
    TSFiBigInt* Beta = tsfi_bn_alloc(); tsfi_bn_copy(Beta, shoot_yi->Psi->Rho->Cone->Mu->Identity);
    
    struct SHAO* orig_rho = allocSHAO();
    orig_rho->Rod = allocSHA(); orig_rho->Rod->Mu = allocFa();
    orig_rho->Cone = allocSHA(); orig_rho->Cone->Mu = allocFa();
    
    orig_rho->Rod->Mu->Base = tsfi_bn_alloc();
    orig_rho->Rod->Mu->Secret = tsfi_bn_alloc();
    orig_rho->Rod->Mu->Signal = tsfi_bn_alloc();
    tsfi_bn_copy(orig_rho->Rod->Mu->Base, shoot_yi->Psi->Rho->Rod->Mu->Base);
    tsfi_bn_copy(orig_rho->Rod->Mu->Secret, shoot_yi->Psi->Rho->Rod->Mu->Secret);
    tsfi_bn_copy(orig_rho->Rod->Mu->Signal, shoot_yi->Psi->Rho->Rod->Mu->Signal);
    
    orig_rho->Cone->Mu->Base = tsfi_bn_alloc();
    orig_rho->Cone->Mu->Secret = tsfi_bn_alloc();
    orig_rho->Cone->Mu->Signal = tsfi_bn_alloc();
    tsfi_bn_copy(orig_rho->Cone->Mu->Base, shoot_yi->Psi->Rho->Cone->Mu->Base);
    tsfi_bn_copy(orig_rho->Cone->Mu->Secret, shoot_yi->Psi->Rho->Cone->Mu->Secret);
    tsfi_bn_copy(orig_rho->Cone->Mu->Signal, shoot_yi->Psi->Rho->Cone->Mu->Signal);

    // 4. FUSE (Regeneration / The Public Key Taste)
    printf("[3/4] Fusing the Public Taste Form...\n");
    struct YI* fuse_yi = tsfi_reaction_fuse(Xi, Beta, Alpha, orig_rho, Prime);
    
    // 5. SIGNATURE 
    printf("[4/4] Signing the Taste with a Payload...\n");
    TSFiBigInt* Payload = tsfi_bn_alloc();
    tsfi_bn_set_u64(Payload, 424242ULL); // A known payload signature
    
    struct Dai* shoot_dai = tsfi_reaction_compute_reciprocity(shoot_yi->Psi, Payload, NULL);
    struct Dai* fuse_dai = tsfi_reaction_compute_reciprocity(fuse_yi->Psi, Payload, NULL);
    
    // 6. VERIFY MIND AND BODY
    uint64_t s_mind = 0, s_body = 0;
    uint64_t f_mind = 0, f_body = 0;
    
    if (shoot_dai->Ichidai->active_limbs > 0) s_mind = shoot_dai->Ichidai->limbs[0];
    if (shoot_dai->Daiichi->active_limbs > 0) s_body = shoot_dai->Daiichi->limbs[0];
    
    if (fuse_dai->Ichidai->active_limbs > 0) f_mind = fuse_dai->Ichidai->limbs[0];
    if (fuse_dai->Daiichi->active_limbs > 0) f_body = fuse_dai->Daiichi->limbs[0];
    
    printf("\nSignature Results (Mind / Body):\n");
    printf("  SHOOT: %lu / %lu\n", s_mind, s_body);
    printf("  FUSE : %lu / %lu\n", f_mind, f_body);
    
    if (s_mind == f_mind && s_body == f_body && s_mind != 0 && s_body != 0) {
        printf("\n[PASS] FUSE successfully verified the signature at both the Mind and Body levels.\n");
    } else {
        printf("\n[FAIL] FUSE Divergence detected in signature verification.\n");
        return 1;
    }
    
    return 0;
}
