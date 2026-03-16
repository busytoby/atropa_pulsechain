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
    printf("=== TSFi FUSE Complete State Verification ===\n");
    
    TSFiBigInt* Prime = tsfi_bn_alloc();
    tsfi_bn_set_u64(Prime, 953473ULL); 
    
    struct YI* shoot_yi = tsfi_reaction_shoot(Prime);
    
    // Extract Private FUSE Parameters
    TSFiBigInt* Xi = tsfi_bn_alloc(); tsfi_bn_copy(Xi, shoot_yi->Xi);
    TSFiBigInt* Alpha = tsfi_bn_alloc(); tsfi_bn_copy(Alpha, shoot_yi->Psi->Rho->Rod->Mu->Identity);
    TSFiBigInt* Beta = tsfi_bn_alloc(); tsfi_bn_copy(Beta, shoot_yi->Psi->Rho->Cone->Mu->Identity);
    
    struct SHAO* orig_rho = allocSHAO();
    orig_rho->Rod = allocSHA(); orig_rho->Rod->Mu = allocFa();
    orig_rho->Cone = allocSHA(); orig_rho->Cone->Mu = allocFa();
    
    orig_rho->Rod->Mu->Base = tsfi_bn_alloc(); orig_rho->Rod->Mu->Secret = tsfi_bn_alloc(); orig_rho->Rod->Mu->Signal = tsfi_bn_alloc();
    tsfi_bn_copy(orig_rho->Rod->Mu->Base, shoot_yi->Psi->Rho->Rod->Mu->Base);
    tsfi_bn_copy(orig_rho->Rod->Mu->Secret, shoot_yi->Psi->Rho->Rod->Mu->Secret);
    tsfi_bn_copy(orig_rho->Rod->Mu->Signal, shoot_yi->Psi->Rho->Rod->Mu->Signal);
    
    orig_rho->Cone->Mu->Base = tsfi_bn_alloc(); orig_rho->Cone->Mu->Secret = tsfi_bn_alloc(); orig_rho->Cone->Mu->Signal = tsfi_bn_alloc();
    tsfi_bn_copy(orig_rho->Cone->Mu->Base, shoot_yi->Psi->Rho->Cone->Mu->Base);
    tsfi_bn_copy(orig_rho->Cone->Mu->Secret, shoot_yi->Psi->Rho->Cone->Mu->Secret);
    tsfi_bn_copy(orig_rho->Cone->Mu->Signal, shoot_yi->Psi->Rho->Cone->Mu->Signal);

    // FUSE it back
    struct YI* fuse_yi = tsfi_reaction_fuse(Xi, Beta, Alpha, orig_rho, Prime);
    
    struct Fa* rod_s = shoot_yi->Psi->Rho->Rod->Mu;
    struct Fa* cone_s = shoot_yi->Psi->Rho->Cone->Mu;
    
    struct Fa* rod_f = fuse_yi->Psi->Rho->Rod->Mu;
    struct Fa* cone_f = fuse_yi->Psi->Rho->Cone->Mu;
    
    int all_match = 1;
    
    if (!verify_bn(rod_s->Secret, rod_f->Secret)) all_match = 0;
    if (!verify_bn(rod_s->Signal, rod_f->Signal)) all_match = 0;
    if (!verify_bn(rod_s->Base, rod_f->Base)) all_match = 0;
    if (!verify_bn(rod_s->Contour, rod_f->Contour)) all_match = 0;
    if (!verify_bn(rod_s->Pole, rod_f->Pole)) all_match = 0;
    if (!verify_bn(rod_s->Coordinate, rod_f->Coordinate)) all_match = 0;
    if (!verify_bn(rod_s->Identity, rod_f->Identity)) all_match = 0;
    if (!verify_bn(rod_s->Foundation, rod_f->Foundation)) all_match = 0;
    if (!verify_bn(rod_s->Channel, rod_f->Channel)) all_match = 0;
    if (!verify_bn(rod_s->Charge, rod_f->Charge)) all_match = 0;
    if (!verify_bn(rod_s->Limit, rod_f->Limit)) all_match = 0;
    if (!verify_bn(rod_s->Element, rod_f->Element)) all_match = 0;
    if (!verify_bn(rod_s->Monopole, rod_f->Monopole)) all_match = 0;
    if (!verify_bn(shoot_yi->Psi->Rho->Rod->Dynamo, fuse_yi->Psi->Rho->Rod->Dynamo)) all_match = 0;
    
    if (!verify_bn(cone_s->Secret, cone_f->Secret)) all_match = 0;
    if (!verify_bn(cone_s->Signal, cone_f->Signal)) all_match = 0;
    if (!verify_bn(cone_s->Base, cone_f->Base)) all_match = 0;
    if (!verify_bn(cone_s->Contour, cone_f->Contour)) all_match = 0;
    if (!verify_bn(cone_s->Pole, cone_f->Pole)) all_match = 0;
    if (!verify_bn(cone_s->Coordinate, cone_f->Coordinate)) all_match = 0;
    if (!verify_bn(cone_s->Identity, cone_f->Identity)) all_match = 0;
    if (!verify_bn(cone_s->Foundation, cone_f->Foundation)) all_match = 0;
    if (!verify_bn(cone_s->Channel, cone_f->Channel)) all_match = 0;
    if (!verify_bn(cone_s->Charge, cone_f->Charge)) all_match = 0;
    if (!verify_bn(cone_s->Limit, cone_f->Limit)) all_match = 0;
    if (!verify_bn(cone_s->Element, cone_f->Element)) all_match = 0;
    if (!verify_bn(cone_s->Monopole, cone_f->Monopole)) all_match = 0;
    if (!verify_bn(shoot_yi->Psi->Rho->Cone->Dynamo, fuse_yi->Psi->Rho->Cone->Dynamo)) all_match = 0;
    
    if (!verify_bn(shoot_yi->Psi->Manifold, fuse_yi->Psi->Manifold)) all_match = 0;
    if (!verify_bn(shoot_yi->Ring, fuse_yi->Ring)) all_match = 0;
    if (!verify_bn(shoot_yi->Psi->Rho->Barn, fuse_yi->Psi->Rho->Barn)) all_match = 0;
    
    if (all_match) {
        printf("[PASS] FUSE strictly reconstructs every single integer across all 14 members of both the Rod and the Cone, as well as the Manifold, Ring, and Barn.\n");
    } else {
        printf("[FAIL] Mismatch detected.\n");
        return 1;
    }
    
    return 0;
}
