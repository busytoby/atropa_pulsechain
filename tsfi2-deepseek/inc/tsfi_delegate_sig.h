#ifndef TSFI_DELEGATE_SIG_H
#define TSFI_DELEGATE_SIG_H

#include <stdint.h>
#include <stdbool.h>

// Render Delegate Hypotrochoid Signature parameters
typedef struct {
    float fx;
    float fy;
    float fz;
    float R_hyp;
    float r_hyp;
    float d_hyp;
} TSFiDelegateSig;

// Initialize the delegate signature to default states
void tsfi_delegate_sig_init(TSFiDelegateSig *sig);

// Resolves signature parameters by hashing the registered LAU token contract address
void tsfi_delegate_sig_resolve(TSFiDelegateSig *sig, const char *token_address);

// Evaluates the hypotrochoid curve coordinates at a given angle t
void tsfi_delegate_sig_evaluate(const TSFiDelegateSig *sig, float t, float *out_x, float *out_y);

#endif // TSFI_DELEGATE_SIG_H
