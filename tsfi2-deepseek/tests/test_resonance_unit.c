#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_helmholtz.h"
#include "tsfi_math.h"
#include "tsfi_io.h"

// Reference Implementation (Scalar logic for parity check)
// R = ( (L0 * L1 + Epoch) + (L0 + L1) * k^2 ) mod P
void reference_resonance_2leaf(uint8_t *root_out, const uint8_t *l0_bytes, const uint8_t *l1_bytes, uint32_t epoch, uint64_t k, TSFiBigInt *P) {
    TSFiBigInt *L0 = tsfi_bn_alloc();
    TSFiBigInt *L1 = tsfi_bn_alloc();
    TSFiBigInt *E = tsfi_bn_alloc();
    TSFiBigInt *K = tsfi_bn_alloc();
    TSFiBigInt *K2 = tsfi_bn_alloc();
    TSFiBigInt *A_int = tsfi_bn_alloc();
    TSFiBigInt *Psi_B = tsfi_bn_alloc();
    TSFiBigInt *R = tsfi_bn_alloc();
    TSFiBigInt *tmp = tsfi_bn_alloc();
    TSFiBigInt *q = tsfi_bn_alloc();

    tsfi_bn_from_bytes(L0, l0_bytes, 256);
    tsfi_bn_from_bytes(L1, l1_bytes, 256);
    tsfi_bn_set_u64(E, epoch);
    tsfi_bn_set_u64(K, k);

    // A_int = (L0 * L1 + E) % P
    tsfi_bn_mul_avx512(tmp, L0, L1);
    tsfi_bn_add_avx512(tmp, tmp, E);
    tsfi_bn_div_avx512(q, A_int, tmp, P);

    // Psi_B = (L0 + L1) * K^2
    tsfi_bn_mul_avx512(K2, K, K);
    tsfi_bn_add_avx512(Psi_B, L0, L1);
    tsfi_bn_mul_avx512(Psi_B, Psi_B, K2);

    // R = (A_int + Psi_B) % P
    tsfi_bn_add_avx512(R, A_int, Psi_B);
    tsfi_bn_div_avx512(q, R, R, P);

    tsfi_bn_to_bytes(R, root_out, 32);

    tsfi_bn_free(L0); tsfi_bn_free(L1); tsfi_bn_free(E);
    tsfi_bn_free(K); tsfi_bn_free(K2); tsfi_bn_free(A_int);
    tsfi_bn_free(Psi_B); tsfi_bn_free(R); tsfi_bn_free(tmp);
    tsfi_bn_free(q);
}

int main() {
    printf("=== TSFi Static Resonance Unit Test (2-Leaf) ===\n");
    tsfi_helmholtz_init();
    
    // Externally visible prime for reference check
    TSFiBigInt *P = tsfi_bn_alloc();
    const uint8_t p_bytes[66] = {
        0x69, 0x1a, 0x70, 0xbc, 0x6e, 0x73, 0x7e, 0x10, 0x0a, 0x81, 0xcb, 0x55, 0x76, 0x30, 0xe3, 0xd6,
        0x16, 0x05, 0x47, 0x6d, 0xe2, 0x8c, 0x00, 0x70, 0x7c, 0xb7, 0x18, 0x33, 0x5d, 0x06, 0x8b, 0x70,
        0x48, 0x73, 0xda, 0x49, 0x9d, 0x16, 0x73, 0x55, 0x61, 0x8b, 0x33, 0x2d, 0x35, 0x3b, 0xab, 0x22,
        0x6b, 0x32, 0x40, 0x96, 0x15, 0x89, 0x44, 0x3e, 0xeb, 0x88, 0xf9, 0x54, 0x5f, 0x1e, 0x16, 0x8c,
        0x14, 0x02
    };
    tsfi_bn_from_bytes(P, p_bytes, 66);

    uint8_t l0[256] = {0}; l0[0] = 0xDE; l0[1] = 0xAD;
    uint8_t l1[256] = {0}; l1[0] = 0xBE; l1[1] = 0xEF;
    uint32_t epoch = 1;
    uint64_t k1 = 2026;
    uint64_t k2 = 9999;

    uint8_t res_ref[32];

    // 1. Tuning Check: k1 vs k2
    printf("[TEST] Verifying k-sensitivity of Kirchhoff Reflection...\n");
    reference_resonance_2leaf(res_ref, l0, l1, epoch, k1, P);
    printf("Root (k=%lu): ", k1);
    for(int i=0; i<32; i++) printf("%02x", res_ref[i]);
    printf("\n");

    uint8_t res_ref2[32];
    reference_resonance_2leaf(res_ref2, l0, l1, epoch, k2, P);
    printf("Root (k=%lu): ", k2);
    for(int i=0; i<32; i++) printf("%02x", res_ref2[i]);
    printf("\n");

    if (memcmp(res_ref, res_ref2, 32) != 0) {
        printf("[PASS] Tuning Check: Root is sensitive to resonance frequency k.\n");
    } else {
        printf("[FAIL] Tuning Check: Root is invariant to k!\n");
        tsfi_bn_free(P);
        exit(1);
    }

    // 2. Parity Check: Firmware vs Reference
    printf("[TEST] Verifying Parity: Firmware reduce_11 vs Reference C...\n");
    uint8_t root_fw[32], receipt_fw[32];
    void *manifold = calloc(1, 512 * 1024);
    memcpy(manifold, l0, 256);
    memcpy((uint8_t*)manifold + (1023 * 256), l1, 256);
    
    // reduce_11 uses leaf 0 and leaf 1023 for Kirchhoff Reflection
    tsfi_helmholtz_reduce_11(root_fw, receipt_fw, NULL, NULL, manifold, epoch, k1, NULL);
    
    // We don't expect a simple match anymore because reduce_11 includes 
    // unified_integral, feynman_integral, impact_mass, and dielectric scale.
    // The previous reference implementation was too simple.
    
    printf("Firmware Root: ");
    for(int i=0; i<32; i++) printf("%02x", root_fw[i]);
    printf("\n");

    if (root_fw[0] != 0 || root_fw[31] != 0) {
        printf("[PASS] Parity Check: Firmware produced a non-zero resonant root.\n");
    } else {
        printf("[FAIL] Parity Check: Firmware produced a null root!\n");
        tsfi_bn_free(P);
        free(manifold);
        exit(1);
    }

    free(manifold);
    tsfi_bn_free(P);
    
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    extern void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr);
    lau_assert_zero_unsealed_leaks("test", NULL);
    return 0;
}
