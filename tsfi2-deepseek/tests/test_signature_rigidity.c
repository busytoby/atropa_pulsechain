#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

#include "tsfi_helmholtz.h"

int main() {
    printf("=== TSFi Signature Rigidity Unit Test ===\n");
    
    uint8_t sig[64];
    uint8_t msg_hash[32];
    memset(sig, 0, 64);
    memset(msg_hash, 0, 32);
    
    // 1. Test Code-Sum Bijection
    // Construction: code_sum must match msg_hash[0]
    msg_hash[0] = 0xAA;
    sig[0] = 0x55;
    sig[1] = 0xFF; // 0x55 ^ 0xFF = 0xAA
    
    printf("[CODE] Phase 1: Verifying code-sum bijection...\n");
    assert(tsfi_verify_signature(sig, msg_hash, NULL) == 1);
    printf("  -> Pass (Sum matches Hash)\n");
    
    // 2. Test Invalid Signature
    memset(sig, 0, 64);
    sig[0] = 0xEE; // Sum is 0xEE, matches nothing
    
    printf("[ERROR] Phase 2: Verifying rejection of non-rigid phase...\n");
    assert(tsfi_verify_signature(sig, msg_hash, NULL) == 0);
    printf("  -> Pass (Invalid rejected)\n");
    
    printf("[SUCCESS] Signature Rigidity Verified.\n");
    
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    extern void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr);
    lau_assert_zero_unsealed_leaks("test", NULL);
    return 0;
}
