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
    
    printf("[TEST] Verifying baseline opinions start at 0.5...\n");
    assert(alpha_rec->SelfOpinion == 0.5);
    assert(alpha_rec->PeerOpinions[1] == 0.5);
    
    printf("[TEST] Simulating dynamic self-opinion drift on disharmonious feedback...\n");
    tsfi_cho_update_opinion(alpha_rec, -1, false);
    printf("  -> Updated SelfOpinion: %.4f\n", alpha_rec->SelfOpinion);
    assert(fabs(alpha_rec->SelfOpinion - 0.425) < 0.001);
    
    printf("[TEST] Simulating dynamic peer-opinion drift on harmonious feedback...\n");
    tsfi_cho_update_opinion(alpha_rec, 1, true);
    printf("  -> Updated PeerOpinions[1]: %.4f\n", alpha_rec->PeerOpinions[1]);
    assert(fabs(alpha_rec->PeerOpinions[1] - 0.575) < 0.001);
    
    printf("[SUCCESS] Swearing-in session, rule learning, and opinion drift verified successfully!\n");
    return 0;
}
