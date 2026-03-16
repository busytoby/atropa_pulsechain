#include "tsfi_reaction.h"
#include "tsfi_io.h"
#include "tsfi_dys_math.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define TSFI_LIMB_BITS 52
#define TSFI_LIMB_MASK 0xFFFFFFFFFFFFF

static inline uint64_t scalar_mont_mul(uint64_t a, uint64_t b, uint64_t m, uint64_t k0) {
    unsigned __int128 prod = (unsigned __int128)a * b;
    uint64_t q = (uint64_t)(prod * k0) & TSFI_LIMB_MASK;
    unsigned __int128 red = prod + (unsigned __int128)q * m;
    uint64_t res = (uint64_t)(red >> TSFI_LIMB_BITS);
    if (res >= m) res -= m;
    return res;
}

static uint64_t calc_k0(uint64_t m0) {
    uint64_t x = m0;
    for (int i = 0; i < 6; i++) {
        x *= (2ULL - m0 * x);
    }
    return -x;
}

int main() {
    TSFiBigInt* Prime = tsfi_bn_alloc();
    tsfi_bn_set_u64(Prime, 953467954114363ULL);

    tsfi_io_printf(stdout, "[INTEGRITY] Starting Step-by-Step Parity Check...\n");

    srand(42);

    struct YI* yi_scalar = tsfi_reaction_shoot(Prime);
    
    struct YI* yi_batch[8];
    for(int i=0; i<8; i++) {
        yi_batch[i] = allocYI();
        yi_batch[i]->Xi = tsfi_bn_alloc();
    }
    
    tsfi_bn_copy(yi_batch[0]->Xi, yi_scalar->Xi);

    tsfi_reaction_shoot_batch8_fused(yi_batch, Prime);

    tsfi_io_printf(stdout, "[INTEGRITY] Batch Fused Run Finished (Internal checks passed).\n");

    TSFiBigInt *b = tsfi_bn_alloc(), *e = tsfi_bn_alloc(), *m = tsfi_bn_alloc();
    TSFiBigInt *r_scalar = tsfi_bn_alloc(), *r_batch[8];
    for(int i=0; i<8; i++) r_batch[i] = tsfi_bn_alloc();

    tsfi_bn_set_u64(b, 1234567);
    tsfi_bn_set_u64(e, 7654321);
    tsfi_bn_set_u64(m, 953467954114363ULL);

    tsfi_bn_modpow_avx512(r_scalar, b, e, m);

    TSFiBigInt *bs[8], *es[8], *ms[8];
    for(int i=0; i<8; i++) { bs[i] = b; es[i] = e; ms[i] = m; }
    tsfi_bn_modpow_batch8(r_batch, bs, es, ms);

    if (tsfi_bn_cmp_avx512(r_scalar, r_batch[0]) != 0) {
        tsfi_io_printf(stderr, "[FAIL] ModPow mismatch: Scalar %lu != Batch %lu\n", 
            r_scalar->limbs[0], r_batch[0]->limbs[0]);
        return 1;
    }

    tsfi_io_printf(stdout, "[INTEGRITY] Validating Montgomery Multiplication Precision...\n");
    uint64_t ma = Prime->limbs[0];
    uint64_t k0 = calc_k0(ma);
    uint64_t a_val = 1404093681048849ULL;
    uint64_t b_val = 173442926184100ULL;
    uint64_t R = ((unsigned __int128)1 << TSFI_LIMB_BITS) % ma;
    uint64_t RR = ((unsigned __int128)R * R) % ma;
    uint64_t a_m = scalar_mont_mul(a_val % ma, RR, ma, k0);
    uint64_t b_m = scalar_mont_mul(b_val % ma, RR, ma, k0);
    uint64_t res_m = scalar_mont_mul(a_m, b_m, ma, k0);
    uint64_t actual = scalar_mont_mul(res_m, 1, ma, k0);
    uint64_t expected = ((unsigned __int128)(a_val % ma) * (b_val % ma)) % ma;
    if (expected != actual) {
        tsfi_io_printf(stderr, "[FAIL] Montgomery Precision mismatch: Expected %lu != Actual %lu\n", expected, actual);
        return 1;
    }

    tsfi_io_printf(stdout, "[SUCCESS] Math Integrity / Parity verified.\n");

    // Clean up
    tsfi_bn_free(b);
    tsfi_bn_free(e);
    tsfi_bn_free(m);
    tsfi_bn_free(r_scalar);
    for(int i=0; i<8; i++) {
        tsfi_bn_free(r_batch[i]);
        freeYI(yi_batch[i]);
    }
    freeYI(yi_scalar);
    tsfi_bn_free(Prime);

    extern void tsfi_bn_drain_pool(void);
    extern void tsfi_dysnomia_drain_pools(void);
    tsfi_bn_drain_pool();
    tsfi_dysnomia_drain_pools();

    extern void lau_registry_teardown(void);
    lau_registry_teardown();

    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}