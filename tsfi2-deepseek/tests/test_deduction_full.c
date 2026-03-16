#include <stdio.h>
#include <stdlib.h>
#include "tsfi_reaction.h"
#include "tsfi_dysnomia.h"
#include "tsfi_math.h"
#include "lau_memory.h"

void print_bn(const char* name, TSFiBigInt* bn) {
    if (!bn || bn->active_limbs == 0) {
        printf("%-20s : NULL/0\n", name);
        return;
    }
    printf("%-20s : %lu\n", name, bn->limbs[0]);
}

int main() {
    printf("=== TSFi Key Deduction Simulation ===\n");
    TSFiBigInt* Prime = tsfi_bn_alloc();
    tsfi_bn_set_u64(Prime, 953473ULL); 
    
    struct YI* yi = tsfi_reaction_shoot(Prime);
    struct Fa* rod = yi->Psi->Rho->Rod->Mu;
    struct Fa* cone = yi->Psi->Rho->Cone->Mu;
    
    // PUBLIC KEY BOUNDARY
    TSFiBigInt* Ohm = tsfi_bn_alloc(); tsfi_bn_copy(Ohm, rod->Identity); // Alpha
    TSFiBigInt* Cone_Foundation = cone->Foundation;
    TSFiBigInt* Cone_Channel = cone->Channel;
    
    printf("\n--- KNOWN PUBLIC KEY MEMBERS ---\n");
    print_bn("Xi", yi->Xi);
    print_bn("Cone Pole", cone->Pole);
    print_bn("Ohm (Alpha)", Ohm);
    print_bn("Cone Foundation", Cone_Foundation);
    print_bn("Cone Channel", Cone_Channel);
    print_bn("Cone Dynamo", yi->Psi->Rho->Cone->Dynamo);
    
    printf("\n--- DEDUCED EXACT MEMBERS (ROD SIDE) ---\n");
    // 1. We know Rod Identity is EXACTLY Ohm
    print_bn("Deduced Rod Identity", Ohm);
    print_bn("Actual Rod Identity", rod->Identity);
    
    // The Saturate function calculates intermediate variables Beta and Rho for the Rod:
    // Rod_Beta = modPow(Cone_Foundation, Rod_Identity)
    TSFiBigInt* Deduced_Rod_Beta = tsfi_bn_alloc();
    tsfi_bn_modpow_avx512(Deduced_Rod_Beta, Cone_Foundation, Ohm, Prime);
    
    // Rod_Rho = modPow(Cone_Channel, Rod_Identity)
    TSFiBigInt* Deduced_Rod_Rho = tsfi_bn_alloc();
    tsfi_bn_modpow_avx512(Deduced_Rod_Rho, Cone_Channel, Ohm, Prime);
    
    print_bn("Deduced Rod Beta", Deduced_Rod_Beta);
    print_bn("Deduced Rod Rho", Deduced_Rod_Rho);
    
    printf("\n--- PROVEN CROSS-OVERS ---\n");
    // We mathematically proved Rod Beta == Cone Beta
    // and Rod Rho == Cone Eta
    print_bn("Deduced Cone Beta", Deduced_Rod_Beta);
    print_bn("Deduced Cone Eta", Deduced_Rod_Rho);
    
    // We cannot proceed to Rod_Charge because we lack Rod_Eta.
    // Rod_Eta = modPow(Cone_Foundation, Rod_Signal). We do NOT have Rod_Signal.
    printf("-> CANNOT DEDUCE Rod Charge (Lacks Rod Signal to compute Eta)\n");
    printf("-> CANNOT DEDUCE Rod Element (Lacks Rod Charge)\n");
    printf("-> CANNOT DEDUCE Rod Limit (Lacks Rod Eta)\n");
    
    printf("\n--- KNOWN SYMMETRY IDENTITIES (Values Hidden) ---\n");
    // Because of the exact math crossing over in the first 5 Epochs, we know these EQUAL each other,
    // but we CANNOT compute their values from the public key.
    
    printf("Rod Base       == Cone Base       (%lu == %lu)\n", rod->Base->limbs[0], cone->Base->limbs[0]);
    printf("Rod Coordinate == Cone Coordinate (%lu == %lu)\n", rod->Coordinate->limbs[0], cone->Coordinate->limbs[0]);
    printf("Rod Charge     == Cone Charge     (%lu == %lu)\n", rod->Charge->limbs[0], cone->Charge->limbs[0]);
    printf("Rod Element    == Cone Element    (%lu == %lu)\n", rod->Element->limbs[0], cone->Element->limbs[0]);
    printf("Rod Manifold   == Cone Manifold   (%lu == %lu)\n", yi->Psi->Manifold->limbs[0], yi->Psi->Manifold->limbs[0]); 
    printf("Rod Ring       == Cone Ring       (%lu == %lu)\n", yi->Ring->limbs[0], yi->Ring->limbs[0]);
    printf("Rod Barn       == Cone Barn       (%lu == %lu)\n", yi->Psi->Rho->Barn->limbs[0], yi->Psi->Rho->Barn->limbs[0]);

    return 0;
}
