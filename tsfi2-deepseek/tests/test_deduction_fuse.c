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
    printf("=== TSFi Key Deduction Simulation (With FUSE logic) ===\n");
    TSFiBigInt* Prime = tsfi_bn_alloc();
    tsfi_bn_set_u64(Prime, 953473ULL); 
    
    struct YI* yi = tsfi_reaction_shoot(Prime);
    struct Fa* rod = yi->Psi->Rho->Rod->Mu;
    struct Fa* cone = yi->Psi->Rho->Cone->Mu;
    
    // PUBLIC KEY BOUNDARY
    TSFiBigInt* Ohm = tsfi_bn_alloc(); tsfi_bn_copy(Ohm, rod->Identity); // Alpha
    TSFiBigInt* Cone_Foundation = cone->Foundation;
    TSFiBigInt* Cone_Channel = cone->Channel;
    
    // IF WE HAVE THE FUSE SCALARS: Xi, Alpha, Beta, Prime
    TSFiBigInt* Xi = yi->Xi;
    TSFiBigInt* Alpha = rod->Identity; // Ohm
    TSFiBigInt* Beta = cone->Identity; // FUSE provides this!
    
    printf("\n--- WE NOW HAVE BETA FROM FUSE ---\n");
    print_bn("Alpha (Ohm)", Alpha);
    print_bn("Beta (Cone Identity)", Beta);
    
    // The Saturate function calculates intermediate variables:
    // Cone_Beta = modPow(Rod_Foundation, Cone_Identity)
    // Cone_Rho = modPow(Rod_Channel, Cone_Identity)
    
    // We deduced Rod_Beta from Alpha earlier:
    // Rod_Beta = modPow(Cone_Foundation, Alpha)
    TSFiBigInt* Rod_Beta = tsfi_bn_alloc();
    tsfi_bn_modpow_avx512(Rod_Beta, Cone_Foundation, Alpha, Prime);
    // And mathematically Rod_Beta == Cone_Beta
    
    // Can we deduce Cone_Rho now that we have Beta?
    // Cone_Rho = modPow(Rod_Channel, Beta)
    // We still DO NOT have Rod_Channel.
    printf("\nWe have Beta, but we still lack Rod_Channel to compute Cone_Rho.\n");
    printf("Rod_Channel requires Rod_Base ^ Rod_Signal.\n");
    
    // Can we deduce Rod_Eta?
    // Rod_Eta = modPow(Cone_Foundation, Rod_Signal). We lack Rod_Signal.
    
    // Even with Alpha, Beta, Xi, and Prime...
    // The core Secret and Signal for both Rod and Cone are STILL missing.
    // The FUSE struct orig_rho contains Secret and Signal. 
    // If you only mean the SCALARS Xi, Alpha, Beta, the structural break still holds.
    
    return 0;
}
