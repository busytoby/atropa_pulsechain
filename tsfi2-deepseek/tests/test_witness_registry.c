#include <stdio.h>
#include <assert.h>
#include "tsfi_witness_registry.h"

#define MotzkinPrime 953467954114363ULL

int main() {
    printf("[TEST] Initializing CHO Witness Registry for Rule Learning and Swearing-in...\n");
    
    tsfi_ChoRegistry registry;
    tsfi_cho_registry_init(&registry);
    
    const char* bear_alpha = "0xAD4e198623A5E2723e19E4D4a6ECF72B1D19FE4B";
    const char* bear_beta  = "0xD07B9f3DF4E9634EbAa0CBF079816925b2C474Ce";
    
    bool alpha_ok = tsfi_cho_swear_in(&registry, bear_alpha, 153023, 440, MotzkinPrime);
    bool beta_ok = tsfi_cho_swear_in(&registry, bear_beta, 112000, 12000, MotzkinPrime);
    assert(alpha_ok && beta_ok);
    
    tsfi_DelegateRecord* alpha_rec = &registry.Delegates[0];
    tsfi_DelegateRecord* beta_rec = &registry.Delegates[1];
    
    printf("[TEST] Logging sequential emotional state attestations for Bear Beta...\n");
    // Feed: NORMATIVE -> EXCITED -> NORMATIVE (REASSURING pattern)
    tsfi_cho_record_attestation(beta_rec, STATE_NORMATIVE);
    tsfi_cho_record_attestation(beta_rec, STATE_EXCITED);
    tsfi_cho_record_attestation(beta_rec, STATE_NORMATIVE);
    
    printf("[TEST] Classifying sequential attestation patterns of Bear Beta...\n");
    tsfi_AttestationPattern pattern = tsfi_cho_classify_attestation_pattern(beta_rec);
    
    printf("  -> Classified Pattern: %s\n", pattern == PATTERN_REASSURING ? "REASSURING (Pattern match success)" : "OTHER");
    assert(pattern == PATTERN_REASSURING);
    
    printf("[SUCCESS] Swearing-in session, rule learning, and sequential attestation pattern classification verified successfully!\n");
    return 0;
}
