#include "tsfi_delegate_sig.h"
#include <openssl/sha.h>
#include <math.h>
#include <string.h>

void tsfi_delegate_sig_init(TSFiDelegateSig *sig) {
    if (!sig) return;
    sig->fx = 1.0f;
    sig->fy = 1.0f;
    sig->fz = 1.0f;
    sig->R_hyp = 5.0f;
    sig->r_hyp = 3.0f;
    sig->d_hyp = 5.0f;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
void tsfi_delegate_sig_resolve(TSFiDelegateSig *sig, const char *token_address) {
    if (!sig || !token_address) return;
    uint8_t hash[32];
    SHA256((const uint8_t*)token_address, strlen(token_address), hash);
    
    // Map hash bytes to signature parameters
    sig->fx = 1.0f + (float)hash[0] / 32.0f;
    sig->fy = 1.0f + (float)hash[1] / 32.0f;
    sig->fz = 1.0f + (float)hash[2] / 32.0f;
    
    sig->R_hyp = 5.0f + (float)hash[3] / 10.0f;
    sig->r_hyp = 1.0f + (float)hash[4] / 20.0f;
    sig->d_hyp = 2.0f + (float)hash[5] / 10.0f;
}
#pragma GCC diagnostic pop

void tsfi_delegate_sig_evaluate(const TSFiDelegateSig *sig, float t, float *out_x, float *out_y) {
    if (!sig || !out_x || !out_y) return;
    
    // Hypotrochoid equations:
    // x(t) = (R - r) * cos(t) + d * cos((R - r) * t / r)
    // y(t) = (R - r) * sin(t) - d * sin((R - r) * t / r)
    float R_minus_r = sig->R_hyp - sig->r_hyp;
    float arg = (R_minus_r * t) / sig->r_hyp;
    
    *out_x = R_minus_r * cosf(t * sig->fx) + sig->d_hyp * cosf(arg * sig->fy);
    *out_y = R_minus_r * sinf(t * sig->fx) - sig->d_hyp * sinf(arg * sig->fy);
}
