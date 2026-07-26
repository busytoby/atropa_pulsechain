#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "../tsfi2-deepseek/inc/tsfi_delegate_sig.h"

int main(void) {
    printf("=== RUNNING AUNCIENT DELEGATE HYPOTROCHOID SIGNATURE TESTS ===\n");

    TSFiDelegateSig sig;
    tsfi_delegate_sig_init(&sig);

    // Initial state
    assert(sig.fx == 1.0f);
    assert(sig.R_hyp == 5.0f);

    // Resolve signature for token contract address
    const char *usdc_addr = "0x9585b5f087750eb39c4d9ca8e8828ef3a44268c6";
    printf("1. Resolving signature for address: %s\n", usdc_addr);
    tsfi_delegate_sig_resolve(&sig, usdc_addr);

    // Verify mapping completed
    assert(sig.fx > 1.0f);
    assert(sig.fy > 1.0f);
    assert(sig.fz > 1.0f);
    assert(sig.R_hyp > 5.0f);
    assert(sig.r_hyp > 1.0f);
    assert(sig.d_hyp > 2.0f);
    printf("   ✓ Parameters mapped: fx=%f, fy=%f, R_hyp=%f, r_hyp=%f, d_hyp=%f\n", 
           sig.fx, sig.fy, sig.R_hyp, sig.r_hyp, sig.d_hyp);

    // Evaluate coordinates
    float x = 0.0f;
    float y = 0.0f;
    tsfi_delegate_sig_evaluate(&sig, 0.5f, &x, &y);
    printf("   Evaluated position at t=0.5: X=%f, Y=%f\n", x, y);
    assert(fabs(x) > 1e-5f || fabs(y) > 1e-5f);

    printf("   ✓ Hypotrochoid coordinate evaluations verified successfully.\n");
    printf("=== AUNCIENT DELEGATE HYPOTROCHOID SIGNATURE TESTS COMPLETE (PASS) ===\n");
    return 0;
}
