#include <stdio.h>
#include <stdlib.h>
#include "tsfi_reaction.h"
#include "tsfi_dysnomia.h"
#include "tsfi_math.h"
#include "lau_memory.h"

int main() {
    printf("=== TSFi Known Value Deduction Test ===\n");
    TSFiBigInt* Prime = tsfi_bn_alloc();
    tsfi_bn_set_u64(Prime, 953473ULL); 
    
    struct YI* yi = tsfi_reaction_shoot(Prime);
    struct Fa* rod = yi->Psi->Rho->Rod->Mu;
    struct Fa* cone = yi->Psi->Rho->Cone->Mu;
    
    TSFiBigInt* Ohm = tsfi_bn_alloc();
    tsfi_bn_copy(Ohm, rod->Identity); // This is Alpha
    
    TSFiBigInt* Cone_Foundation = cone->Foundation;
    TSFiBigInt* Cone_Channel = cone->Channel;
    
    printf("--- PUBLIC KEY ---\n");
    printf("Ohm             : %lu\n", Ohm->limbs[0]);
    printf("Cone Foundation : %lu\n", Cone_Foundation->limbs[0]);
    printf("Cone Channel    : %lu\n", Cone_Channel->limbs[0]);
    printf("Cone Dynamo     : %lu\n", yi->Psi->Rho->Cone->Dynamo->limbs[0]);
    
    printf("\n--- DEDUCED VALUES ---\n");
    
    TSFiBigInt* Rod_Rho = tsfi_bn_alloc();
    tsfi_bn_modpow_avx512(Rod_Rho, Cone_Channel, Ohm, Prime);
    printf("Deduced Rod Rho : %lu\n", Rod_Rho->limbs[0]);
    
    TSFiBigInt* Rod_Beta = tsfi_bn_alloc();
    tsfi_bn_modpow_avx512(Rod_Beta, Cone_Foundation, Ohm, Prime);
    printf("Deduced Rod Beta: %lu\n", Rod_Beta->limbs[0]);
    
    printf("\n--- ACTUAL VALUES FOR VERIFICATION ---\n");
    printf("Rod Charge      : %lu\n", rod->Charge->limbs[0]);
    printf("Cone Charge     : %lu\n", cone->Charge->limbs[0]);
    printf("Rod Element     : %lu\n", rod->Element->limbs[0]);
    printf("Cone Element    : %lu\n", cone->Element->limbs[0]);
    
    return 0;
}
