#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "tsfi_witness_registry.h"

#define MotzkinPrime 953467954114363ULL

int main() {
    printf("[TEST] Initializing CHO Witness Registry for Rule Learning and Swearing-in...\n");
    
    tsfi_ChoRegistry registry;
    tsfi_cho_registry_init(&registry);
    
    const char* bear_alpha = "0xAD4e198623A5E2723e19E4D4a6ECF72B1D19FE4B";
    
    bool alpha_ok = tsfi_cho_swear_in(&registry, bear_alpha, 153023, 440, MotzkinPrime);
    assert(alpha_ok);
    
    tsfi_DelegateRecord* alpha_rec = &registry.Delegates[0];
    
    printf("[TEST] Verifying baseline Blow-Up Factor is 0 and Eigenvalue is 1...\n");
    assert(alpha_rec->BlowUpFactor == 0.0);
    assert(alpha_rec->ConstraintEigenvalue == 1.0);
    
    printf("[TEST] Applying instability factor of 0.5 to restrict constraints...\n");
    tsfi_cho_restrict_eigenvector_constraints(alpha_rec, 0.5);
    
    printf("  -> Updated BlowUpFactor: %.2f\n", alpha_rec->BlowUpFactor);
    printf("  -> Updated Eigenvalue: %.4f\n", alpha_rec->ConstraintEigenvalue);
    
    assert(alpha_rec->BlowUpFactor == 0.5);
    assert(fabs(alpha_rec->ConstraintEigenvalue - 0.6667) < 0.001);
    
    printf("[SUCCESS] Swearing-in session, rule learning, and eigenvector constraints verified successfully!\n");
    return 0;
}
