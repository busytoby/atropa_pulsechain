#ifndef TSFI_MATH_H
#define TSFI_MATH_H

#include <stdint.h>
#include <stddef.h>
#include <immintrin.h>
#include "lau_memory.h"
#include "tsfi_types.h"
#include "tsfi_wiring.h"

// --- Configuration ---
#define TSFI_MATH_MAX_BYTES 8192
#define TSFI_MATH_MAX_BITS  (TSFI_MATH_MAX_BYTES * 8)

#define TSFI_LIMB_BITS 52
#define TSFI_LIMB_MASK 0xFFFFFFFFFFFFF
#define TSFI_NUM_LIMBS_RAW ((TSFI_MATH_MAX_BITS + TSFI_LIMB_BITS - 1) / TSFI_LIMB_BITS)
#define TSFI_NUM_LIMBS ((TSFI_NUM_LIMBS_RAW + 7) / 8 * 8)

// --- Mapped Structure Definition ---
// Includes 'limbs' aligned to 64 bytes for AVX512 usage.
// The MAPPED_COMMON_FIELDS (80 bytes) precede this.
// Compiler should pad to 128 bytes to satisfy align(64).
DEFINE_MAPPED_STRUCT(TSFiBigInt,
    uint64_t limbs[TSFI_NUM_LIMBS] __attribute__((aligned(64)));
    size_t active_limbs;
)

// --- Lifecycle ---
TSFiBigInt* tsfi_bn_alloc(void);
void tsfi_bn_free(TSFiBigInt *bn);

// --- Utilities ---
void tsfi_bn_randomize(TSFiBigInt *bn);
void tsfi_bn_from_bytes(TSFiBigInt *bn, const uint8_t *bytes, size_t len);
void tsfi_bn_to_bytes(const TSFiBigInt *bn, uint8_t *bytes, size_t len);
void tsfi_bn_set_u64(TSFiBigInt *bn, uint64_t val); // New

// --- Arithmetic ---
void tsfi_bn_modpow_scalar(TSFiBigInt *result, const TSFiBigInt *base, const TSFiBigInt *exp, const TSFiBigInt *mod);
void tsfi_bn_modpow_avx512(TSFiBigInt *result, const TSFiBigInt *base, const TSFiBigInt *exp, const TSFiBigInt *mod);
void tsfi_bn_mul_avx512(TSFiBigInt *r, const TSFiBigInt *a, const TSFiBigInt *b);
int tsfi_bn_cmp_avx512(const TSFiBigInt *a, const TSFiBigInt *b); // New
void tsfi_bn_add_avx512(TSFiBigInt *r, const TSFiBigInt *a, const TSFiBigInt *b); // New
void tsfi_bn_sub_avx512(TSFiBigInt *r, const TSFiBigInt *a, const TSFiBigInt *b); // New
void tsfi_bn_lshift_avx512(TSFiBigInt *r, const TSFiBigInt *a, int bits); // New
void tsfi_bn_rshift_avx512(TSFiBigInt *r, const TSFiBigInt *a, int bits); // New
void tsfi_bn_div_avx512(TSFiBigInt *q, TSFiBigInt *r, const TSFiBigInt *a, const TSFiBigInt *b); // New

#endif // TSFI_MATH_H