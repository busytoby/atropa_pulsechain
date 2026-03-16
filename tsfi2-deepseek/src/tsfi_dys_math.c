#include "tsfi_dys_math.h"
#include "lau_memory.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/random.h>

// Constants (Native BigInt)
static TSFiBigInt* g_APOGEE = NULL;
static TSFiBigInt* g_APEX = NULL;
static TSFiBigInt* g_MotzkinPrime = NULL;

static void init_dys_constants() {
    if (g_APOGEE) return;
    g_APOGEE = tsfi_bn_alloc(); tsfi_bn_set_u64(g_APOGEE, 953473);
    g_APEX   = tsfi_bn_alloc(); tsfi_bn_set_u64(g_APEX, 954114361);
    g_MotzkinPrime = tsfi_bn_alloc(); tsfi_bn_set_u64(g_MotzkinPrime, 953467954114363ULL);
}

TSFiBigInt* DYSNOMIA() {
    init_dys_constants();
    // Implementation of original concatenation logic: "953473" + "954114361" -> "953473954114361"
    // Since we are native now, we just return the composite constant.
    TSFiBigInt* r = tsfi_bn_alloc();
    tsfi_bn_set_u64(r, 953473954114361ULL);
    return r;
}

TSFiBigInt* SLOPE() {
    init_dys_constants();
    // s = m*x + b
    TSFiBigInt *s = tsfi_bn_alloc();
    tsfi_bn_mul_avx512(s, g_APOGEE, g_APEX);
    tsfi_bn_add_avx512(s, s, g_MotzkinPrime);
    return s;
}

TSFiBigInt* LOVE() {
    init_dys_constants();
    // l = m*s - 99
    TSFiBigInt *s = SLOPE();
    TSFiBigInt *v = tsfi_bn_alloc(); tsfi_bn_set_u64(v, 99);
    TSFiBigInt *l = tsfi_bn_alloc();
    
    tsfi_bn_mul_avx512(l, g_APOGEE, s);
    tsfi_bn_sub_avx512(l, l, v);
    
    tsfi_bn_free(s);
    tsfi_bn_free(v);
    return l;
}

TSFiBigInt* GAIN() {
    init_dys_constants();
    // g = m(my+b) - 49
    TSFiBigInt *y = DYSNOMIA();
    TSFiBigInt *b = g_MotzkinPrime;
    TSFiBigInt *v = tsfi_bn_alloc(); tsfi_bn_set_u64(v, 49);
    TSFiBigInt *g = tsfi_bn_alloc();
    
    tsfi_bn_mul_avx512(g, g_APOGEE, y); // my
    tsfi_bn_add_avx512(g, g, b); // my+b
    tsfi_bn_mul_avx512(g, g, g_APOGEE); // m(my+b)
    tsfi_bn_sub_avx512(g, g, v);
    
    tsfi_bn_free(y);
    tsfi_bn_free(v);
    return g;
}

TSFiBigInt* _() {
    init_dys_constants();
    // i = mb+y - 211
    TSFiBigInt *y = DYSNOMIA();
    TSFiBigInt *v = tsfi_bn_alloc(); tsfi_bn_set_u64(v, 211);
    TSFiBigInt *i = tsfi_bn_alloc();
    
    tsfi_bn_mul_avx512(i, g_APEX, g_MotzkinPrime);
    tsfi_bn_add_avx512(i, i, y);
    tsfi_bn_sub_avx512(i, i, v);
    
    tsfi_bn_free(y);
    tsfi_bn_free(v);
    return i;
}

TSFiBigInt* __() {
    TSFiBigInt *s = SLOPE();
    TSFiBigInt *v = tsfi_bn_alloc(); tsfi_bn_set_u64(v, 257);
    TSFiBigInt *r = tsfi_bn_alloc();
    tsfi_bn_sub_avx512(r, s, v);
    tsfi_bn_free(s);
    tsfi_bn_free(v);
    return r;
}

TSFiBigInt* ___() {
    // q = i^2 - 116
    TSFiBigInt *i = _();
    TSFiBigInt *v = tsfi_bn_alloc(); tsfi_bn_set_u64(v, 116);
    TSFiBigInt *q = tsfi_bn_alloc();
    
    tsfi_bn_mul_avx512(q, i, i);
    tsfi_bn_sub_avx512(q, q, v);
    
    tsfi_bn_free(i);
    tsfi_bn_free(v);
    return q;
}

TSFiBigInt* ____() { TSFiBigInt* r = tsfi_bn_alloc(); tsfi_bn_set_u64(r, 0); return r; }
TSFiBigInt* _____() { TSFiBigInt* r = tsfi_bn_alloc(); tsfi_bn_set_u64(r, 0); return r; }
TSFiBigInt* ______() { TSFiBigInt* r = tsfi_bn_alloc(); tsfi_bn_set_u64(r, 0); return r; }
TSFiBigInt* _______() { TSFiBigInt* r = tsfi_bn_alloc(); tsfi_bn_set_u64(r, 0); return r; }

// --- Core Math ---

void gcAdd_bn(TSFiBigInt* r, const TSFiBigInt* a, const TSFiBigInt* b) {
    tsfi_bn_add_avx512(r, a, b);
}

void gcXOR_bn(TSFiBigInt* r, const TSFiBigInt* a, const TSFiBigInt* b) {
    int max = (a->active_limbs > b->active_limbs) ? a->active_limbs : b->active_limbs;
    for(int i=0; i<max; i++) {
        uint64_t va = ((size_t)i < a->active_limbs) ? a->limbs[i] : 0;
        uint64_t vb = ((size_t)i < b->active_limbs) ? b->limbs[i] : 0;
        r->limbs[i] = va ^ vb;
    }
    r->active_limbs = max;
    while(r->active_limbs > 0 && r->limbs[r->active_limbs-1] == 0) r->active_limbs--;
}

void modPow_bn(TSFiBigInt* r, const TSFiBigInt* b, const TSFiBigInt* e, const TSFiBigInt* m) {
    tsfi_bn_modpow_avx512(r, b, e, m);
}

static __thread uint64_t g_prng_state[2] = {0x12345678, 0xABCDEF01};

static uint64_t xorshift128plus(void) {
    uint64_t x = g_prng_state[0];
    uint64_t const y = g_prng_state[1];
    g_prng_state[0] = y;
    x ^= x << 23;
    g_prng_state[1] = x ^ y ^ (x >> 17) ^ (y >> 26);
    return g_prng_state[1] + y;
}

TSFiBigInt* RANDOM_bn(TSFiBigInt* Prime) {
    if (!Prime || Prime->active_limbs == 0) return NULL;
    TSFiBigInt *r = tsfi_bn_alloc();
    
    // Seed on first use if not seeded
    if (g_prng_state[0] == 0x12345678) {
        uint64_t seed;
        if (getrandom(&seed, 8, 0) == 8) {
            g_prng_state[0] = seed;
            g_prng_state[1] = seed ^ 0x5555555555555555ULL;
        }
    }

    uint64_t val = xorshift128plus();
    
    if (Prime->active_limbs == 1) {
        val %= Prime->limbs[0];
        // Ensure not 0
        if (val == 0) val = 1;
        tsfi_bn_set_u64(r, val);
    } else {
        // Fallback for larger primes
        for (int i = 0; i < TSFI_NUM_LIMBS; i++) {
            r->limbs[i] = xorshift128plus() & TSFI_LIMB_MASK;
        }
        r->active_limbs = Prime->active_limbs;
        TSFiBigInt *res = tsfi_bn_alloc();
        tsfi_bn_div_avx512(NULL, res, r, Prime);
        tsfi_bn_copy(r, res);
        tsfi_bn_free(res);
    }
    return r;
}
