#include "tsfi_reaction.h"
#include "tsfi_dysnomia.h"
#include "lau_memory.h"
#include "lau_audit.h"
#include "tsfi_io.h"
#include "tsfi_dys_math.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_yi_for_prime(uint64_t prime_val) {
    TSFiBigInt *p_bn = tsfi_bn_alloc();
    tsfi_bn_set_u64(p_bn, prime_val);

    struct YI* yi = tsfi_reaction_shoot(p_bn);
    if (!yi) {
        printf("[FAIL] Reaction failed for prime %lu\n", prime_val);
        return;
    }

    printf("PRIME: %lu\n", prime_val);
    printf("  Manifold: %lu\n", yi->Psi->Manifold->limbs[0]);
    printf("  Monopole: %lu\n", yi->Psi->Monopole->limbs[0]);
    if (yi->Ring) {
        printf("  Ring: %lu\n", yi->Ring->limbs[0]);
    } else {
        printf("  Ring: 0\n");
    }
    if (yi->Psi->Rho) {
        if (yi->Psi->Rho->Rod) {
            printf("  Rod_Dynamo: %lu\n", yi->Psi->Rho->Rod->Dynamo->limbs[0]);
            if (yi->Psi->Rho->Rod->Mu) {
                printf("  Rod_Identity: %lu\n", yi->Psi->Rho->Rod->Mu->Identity->limbs[0]);
                printf("  Rod_Pole: %lu\n", yi->Psi->Rho->Rod->Mu->Pole->limbs[0]);
                printf("  Rod_Base: %lu\n", yi->Psi->Rho->Rod->Mu->Base->limbs[0]);
            }
        }
        if (yi->Psi->Rho->Cone) {
            printf("  Cone_Dynamo: %lu\n", yi->Psi->Rho->Cone->Dynamo->limbs[0]);
            if (yi->Psi->Rho->Cone->Mu) {
                printf("  Cone_Identity: %lu\n", yi->Psi->Rho->Cone->Mu->Identity->limbs[0]);
                printf("  Cone_Pole: %lu\n", yi->Psi->Rho->Cone->Mu->Pole->limbs[0]);
                printf("  Cone_Base: %lu\n", yi->Psi->Rho->Cone->Mu->Base->limbs[0]);
            }
        }
    }
    printf("\n");

    freeYI(yi);
    tsfi_bn_free(p_bn);
}

int main() {
    print_yi_for_prime(953467954114363ULL);
    print_yi_for_prime(1000000000000037ULL);
    print_yi_for_prime(1000000000000091ULL);
    print_yi_for_prime(1000000000000159ULL);
    return 0;
}
