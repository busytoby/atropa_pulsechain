#include <stdio.h>
#include <stdlib.h>
#include "tsfi_reaction.h"
#include "tsfi_dysnomia.h"
#include "tsfi_math.h"
#include "lau_memory.h"

void print_bn(const char* name, TSFiBigInt* bn) {
    if (!bn || bn->active_limbs == 0) {
        printf("%-18s : NULL/0\n", name);
        return;
    }
    printf("%-18s : %lu\n", name, bn->limbs[0]);
}

int main() {
    printf("=== TSFi Trilateral Symmetry Test ===\n");
    TSFiBigInt* Prime = tsfi_bn_alloc();
    tsfi_bn_set_u64(Prime, 953473ULL); 
    
    struct YI* yi = tsfi_reaction_shoot(Prime);
    struct Fa* rod = yi->Psi->Rho->Rod->Mu;
    struct Fa* cone = yi->Psi->Rho->Cone->Mu;
    
    printf("\n--- ROD STATE ---\n");
    print_bn("Secret", rod->Secret);
    print_bn("Signal", rod->Signal);
    print_bn("Base", rod->Base);
    print_bn("Contour", rod->Contour);
    print_bn("Pole", rod->Pole);
    print_bn("Coordinate", rod->Coordinate);
    print_bn("Identity", rod->Identity);
    print_bn("Foundation", rod->Foundation);
    print_bn("Channel", rod->Channel);
    print_bn("Charge", rod->Charge);
    print_bn("Limit", rod->Limit);
    print_bn("Element", rod->Element);
    print_bn("Monopole", rod->Monopole);
    print_bn("Dynamo", yi->Psi->Rho->Rod->Dynamo);
    
    printf("\n--- CONE STATE ---\n");
    print_bn("Secret", cone->Secret);
    print_bn("Signal", cone->Signal);
    print_bn("Base", cone->Base);
    print_bn("Contour", cone->Contour);
    print_bn("Pole", cone->Pole);
    print_bn("Coordinate", cone->Coordinate);
    print_bn("Identity", cone->Identity);
    print_bn("Foundation", cone->Foundation);
    print_bn("Channel", cone->Channel);
    print_bn("Charge", cone->Charge);
    print_bn("Limit", cone->Limit);
    print_bn("Element", cone->Element);
    print_bn("Monopole", cone->Monopole);
    print_bn("Dynamo", yi->Psi->Rho->Cone->Dynamo);
    
    return 0;
}
