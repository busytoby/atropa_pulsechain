#include "tsfi_math.h"
#include <stdlib.h>
#include <string.h>

// Helper to reverse buffer (LE <-> BE)
static void reverse_bytes(uint8_t *buf, size_t len) {
    for (size_t i = 0; i < len / 2; i++) {
        uint8_t t = buf[i];
        buf[i] = buf[len - 1 - i];
        buf[len - 1 - i] = t;
    }
}

// Exported function for C# Interop
// Returns 0 on success
__attribute__((visibility("default")))
int Interop_ModPow(
    const uint8_t *base_bytes, size_t base_len,
    const uint8_t *exp_bytes, size_t exp_len,
    const uint8_t *mod_bytes, size_t mod_len,
    uint8_t *result_buf, size_t result_cap, size_t *result_written
) {
    if (!base_bytes || !exp_bytes || !mod_bytes || !result_buf || !result_written) return -1;

    // Allocate TSFiBigInts (managed by lau_memory)
    TSFiBigInt *b = tsfi_bn_alloc();
    TSFiBigInt *e = tsfi_bn_alloc();
    TSFiBigInt *m = tsfi_bn_alloc();
    TSFiBigInt *r = tsfi_bn_alloc();

    // Copy and Reverse (C# LE -> TSFi BE)
    uint8_t *tmp_b = lau_malloc(base_len); memcpy(tmp_b, base_bytes, base_len); reverse_bytes(tmp_b, base_len);
    uint8_t *tmp_e = lau_malloc(exp_len);  memcpy(tmp_e, exp_bytes, exp_len);  reverse_bytes(tmp_e, exp_len);
    uint8_t *tmp_m = lau_malloc(mod_len);  memcpy(tmp_m, mod_bytes, mod_len);  reverse_bytes(tmp_m, mod_len);

    tsfi_bn_from_bytes(b, tmp_b, base_len);
    tsfi_bn_from_bytes(e, tmp_e, exp_len);
    tsfi_bn_from_bytes(m, tmp_m, mod_len);

    lau_free(tmp_b); lau_free(tmp_e); lau_free(tmp_m);

    // Compute
    tsfi_bn_modpow_avx512(r, b, e, m);

    // Result size matches modulus size usually, but we output what we have.
    // For simplicity, we output 'mod_len' bytes, as result < m.
    size_t out_len = mod_len;
    if (result_cap < out_len) {
        tsfi_bn_free(b); tsfi_bn_free(e); tsfi_bn_free(m); tsfi_bn_free(r);
        return -2; // Buffer too small
    }

    uint8_t *tmp_r = lau_malloc(out_len);
    tsfi_bn_to_bytes(r, tmp_r, out_len); // Returns BE
    reverse_bytes(tmp_r, out_len); // Convert to LE

    memcpy(result_buf, tmp_r, out_len);
    *result_written = out_len;

    lau_free(tmp_r);
    tsfi_bn_free(b); tsfi_bn_free(e); tsfi_bn_free(m); tsfi_bn_free(r);
    return 0;
}
