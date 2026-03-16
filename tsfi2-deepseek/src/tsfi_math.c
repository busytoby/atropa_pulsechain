#include "tsfi_math.h"
#include "tsfi_wave512.h"
#include "tsfi_io.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdalign.h>
#include <time.h>
#include <immintrin.h>

// --- Math System Logic ---

static inline void tsfi_bn_trim(TSFiBigInt *bn) {
    if (!bn) return;
    int i = (int)TSFI_NUM_LIMBS - 1;
    while (i >= 0 && bn->limbs[i] == 0) i--;
    bn->active_limbs = (size_t)(i + 1);
}

static inline void tsfi_bn_clean_upper(TSFiBigInt *bn) {
    tsfi_bn_trim(bn);
    for (size_t i = bn->active_limbs; i < TSFI_NUM_LIMBS; i++) {
        bn->limbs[i] = 0;
    }
}

static void math_logic_scramble(void *ws_ptr) {
    TSFiBigInt *bn = (TSFiBigInt*)ws_ptr;
    lau_mem_scramble(bn->limbs, sizeof(bn->limbs), LAU_SCRAMBLE_MODE_RANDOM);
    for (int i = 0; i < TSFI_NUM_LIMBS; i++) {
        bn->limbs[i] &= TSFI_LIMB_MASK;
    }
    tsfi_bn_trim(bn);
}

static void math_logic_epoch(int *ver) { (*ver)++; }
static bool math_logic_state(void *obj) { (void)obj; return true; }
static void math_logic_directive(int *cnt, char *dir) { (void)cnt; (void)dir; }
static void math_logic_provenance(void *ws) { (void)ws; }

static const TSFiLogicTable kMathLogic __attribute__((unused)) = {
    .logic_epoch = math_logic_epoch,
    .logic_state = math_logic_state,
    .logic_directive = math_logic_directive,
    .logic_scramble = math_logic_scramble,
    .logic_provenance = math_logic_provenance
};

static WaveSystem* g_math_prototype __attribute__((unused)) = NULL;

#define TSFI_BN_POOL_SIZE 4096
static TSFiBigInt* g_bn_pool[TSFI_BN_POOL_SIZE];
static int g_bn_pool_count = 0;

TSFiBigInt* tsfi_bn_alloc(void) {
    if (g_bn_pool_count > 0) {
        TSFiBigInt* bn = g_bn_pool[--g_bn_pool_count];
        memset(bn->limbs, 0, sizeof(bn->limbs));
        bn->active_limbs = 0;
        return bn;
    }
    TSFiBigInt* bn = (TSFiBigInt*)lau_malloc_wired(sizeof(TSFiBigInt));
    if (!bn) return NULL;
    memset(bn->limbs, 0, sizeof(bn->limbs));
    bn->active_limbs = 0;
    return bn;
}

TSFiBigInt* tsfi_bn_realloc(TSFiBigInt *bn) {
    if (!bn) return tsfi_bn_alloc();
    memset(bn->limbs, 0, sizeof(bn->limbs));
    bn->active_limbs = 0;
    return bn;
}

void tsfi_bn_pool_cleanup(void) {
    while (g_bn_pool_count > 0) {
        lau_free(g_bn_pool[--g_bn_pool_count]);
    }
}
void tsfi_bn_free(TSFiBigInt *bn) {
    if (!bn) return;
    for (int i = 0; i < g_bn_pool_count; i++) {
        if (g_bn_pool[i] == bn) return;
    }
    if (g_bn_pool_count < TSFI_BN_POOL_SIZE) {
        g_bn_pool[g_bn_pool_count++] = bn;
    } else {
        lau_free(bn);
    }
}

void tsfi_bn_randomize(TSFiBigInt *bn) {
    for (int i = 0; i < TSFI_NUM_LIMBS; i++) {
        bn->limbs[i] = (uint64_t)rand() & TSFI_LIMB_MASK;
    }
    bn->active_limbs = TSFI_NUM_LIMBS;
}

void tsfi_bn_from_bytes(TSFiBigInt *bn, const uint8_t *bytes, size_t len) {
    memset(bn->limbs, 0, sizeof(bn->limbs));
    size_t limb_idx = 0;
    int bit_offset = 0;
    for (size_t i = 0; i < len; i++) {
        uint64_t val = bytes[len - 1 - i];
        if (bit_offset + 8 <= TSFI_LIMB_BITS) {
            bn->limbs[limb_idx] |= (val << bit_offset);
            bit_offset += 8;
        } else {
            int first_part = TSFI_LIMB_BITS - bit_offset;
            bn->limbs[limb_idx] |= ((val & ((1 << first_part) - 1)) << bit_offset);
            limb_idx++;
            bn->limbs[limb_idx] |= (val >> first_part);
            bit_offset = 8 - first_part;
        }
    }
    tsfi_bn_trim(bn);
}

void tsfi_bn_to_bytes(const TSFiBigInt *bn, uint8_t *bytes, size_t len) {
    if (!bn || !bytes || len == 0) return;
    memset(bytes, 0, len);
    size_t limb_idx = 0;
    int bit_offset = 0;
    for (size_t i = 0; i < len; i++) {
        uint64_t val = 0;
        if (bit_offset + 8 <= TSFI_LIMB_BITS) {
            val = (bn->limbs[limb_idx] >> bit_offset) & 0xFF;
            bit_offset += 8;
            if (bit_offset == TSFI_LIMB_BITS) { limb_idx++; bit_offset = 0; }
        } else {
            int first_part = TSFI_LIMB_BITS - bit_offset;
            val = (bn->limbs[limb_idx] >> bit_offset) & ((1 << first_part) - 1);
            limb_idx++;
            int second_part = 8 - first_part;
            val |= (bn->limbs[limb_idx] & ((1 << second_part) - 1)) << first_part;
            bit_offset = second_part;
        }
        bytes[len - 1 - i] = (uint8_t)val;
        if (limb_idx >= TSFI_NUM_LIMBS) break;
    }
}

void tsfi_bn_set_u64(TSFiBigInt *bn, uint64_t val) {
    memset(bn->limbs, 0, sizeof(bn->limbs));
    bn->limbs[0] = val & TSFI_LIMB_MASK;
    bn->limbs[1] = val >> TSFI_LIMB_BITS;
    bn->active_limbs = (bn->limbs[1]) ? 2 : (bn->limbs[0] ? 1 : 0);
}

static uint64_t calc_k0(uint64_t m0) {
    uint64_t x = m0;
    for (int i = 0; i < 6; i++) {
        x *= (2ULL - m0 * x);
    }
    return -x;
}

void tsfi_bn_copy(TSFiBigInt *dst, const TSFiBigInt *src) {
    memcpy(dst->limbs, src->limbs, sizeof(src->limbs));
    dst->active_limbs = src->active_limbs;
}

int tsfi_bn_cmp_avx512(const TSFiBigInt *a, const TSFiBigInt *b) {
    int n_a = (int)a->active_limbs;
    while (n_a > 0 && a->limbs[n_a - 1] == 0) n_a--;
    int n_b = (int)b->active_limbs;
    while (n_b > 0 && b->limbs[n_b - 1] == 0) n_b--;
    if (n_a > n_b) return 1;
    if (n_a < n_b) return -1;
    if (n_a == 0) return 0;
    for (int i = n_a - 1; i >= 0; i--) {
        if (a->limbs[i] > b->limbs[i]) return 1;
        if (a->limbs[i] < b->limbs[i]) return -1;
    }
    return 0;
}

static int bn_cmp(const TSFiBigInt *a, const TSFiBigInt *b) {
    return tsfi_bn_cmp_avx512(a, b);
}

static void bn_sub_safe(TSFiBigInt *r, const TSFiBigInt *a, const TSFiBigInt *b) {
    uint64_t borrow = 0;
    int n = (a->active_limbs > b->active_limbs) ? a->active_limbs : b->active_limbs;
    for(int i=0; i<n; i++) {
        uint64_t va = (i < (int)a->active_limbs) ? a->limbs[i] : 0;
        uint64_t vb = (i < (int)b->active_limbs) ? b->limbs[i] : 0;
        uint64_t sub = va - vb - borrow;
        if (va < vb + borrow) {
            r->limbs[i] = sub & TSFI_LIMB_MASK;
            borrow = 1;
        } else {
            r->limbs[i] = sub & TSFI_LIMB_MASK;
            borrow = 0;
        }
    }
    tsfi_bn_trim(r);
}

void tsfi_bn_add_avx512(TSFiBigInt *r, const TSFiBigInt *a, const TSFiBigInt *b) {
    uint64_t carry = 0;
    int n = (a->active_limbs > b->active_limbs) ? a->active_limbs : b->active_limbs;
    for(int i=0; i<n; i++) {
        uint64_t va = (i < (int)a->active_limbs) ? a->limbs[i] : 0;
        uint64_t vb = (i < (int)b->active_limbs) ? b->limbs[i] : 0;
        uint64_t sum = va + vb + carry;
        r->limbs[i] = sum & TSFI_LIMB_MASK;
        carry = sum >> TSFI_LIMB_BITS;
    }
    if (carry) r->limbs[n++] = carry;
    r->active_limbs = n;
}

void tsfi_bn_sub_avx512(TSFiBigInt *r, const TSFiBigInt *a, const TSFiBigInt *b) {
    bn_sub_safe(r, a, b);
}

void tsfi_bn_mul_avx512(TSFiBigInt *r, const TSFiBigInt *a, const TSFiBigInt *b) {
    TSFiBigInt tmp;
    memset(&tmp, 0, sizeof(tmp));
    for (size_t i = 0; i < a->active_limbs; i++) {
        uint64_t carry = 0;
        for (size_t j = 0; j < b->active_limbs; j++) {
            unsigned __int128 prod = (unsigned __int128)a->limbs[i] * b->limbs[j] + tmp.limbs[i+j] + carry;
            tmp.limbs[i+j] = (uint64_t)prod & TSFI_LIMB_MASK;
            carry = (uint64_t)(prod >> TSFI_LIMB_BITS);
        }
        tmp.limbs[i + b->active_limbs] += carry;
    }
    tsfi_bn_copy(r, &tmp);
    tsfi_bn_trim(r);
}

void tsfi_bn_div_avx512(TSFiBigInt *q, TSFiBigInt *r, const TSFiBigInt *a, const TSFiBigInt *b) {
    if (b->active_limbs == 0) return;
    if (b->active_limbs == 1 && b->limbs[0] == 0) return;
    if (bn_cmp(a, b) < 0) {
        if (q) tsfi_bn_set_u64(q, 0);
        if (r) tsfi_bn_copy(r, a);
        return;
    }
    TSFiBigInt rem, quo;
    memset(&rem, 0, sizeof(rem));
    memset(&quo, 0, sizeof(quo));
    
    int total_bits = a->active_limbs * TSFI_LIMB_BITS;
    for (int i = total_bits - 1; i >= 0; i--) {
        uint64_t carry = (a->limbs[i / TSFI_LIMB_BITS] >> (i % TSFI_LIMB_BITS)) & 1;
        uint64_t r_carry = carry;
        for(size_t j=0; j<rem.active_limbs || r_carry; j++) {
            uint64_t val = (rem.limbs[j] << 1) | r_carry;
            rem.limbs[j] = val & TSFI_LIMB_MASK;
            r_carry = val >> TSFI_LIMB_BITS;
            if (j >= rem.active_limbs) rem.active_limbs = j+1;
        }
        if (bn_cmp(&rem, b) >= 0) {
            bn_sub_safe(&rem, &rem, b);
            quo.limbs[i / TSFI_LIMB_BITS] |= (1ULL << (i % TSFI_LIMB_BITS));
        }
    }
    tsfi_bn_trim(&quo);
    if (q) tsfi_bn_copy(q, &quo);
    if (r) tsfi_bn_copy(r, &rem);
}

static __attribute__((unused)) void bn_mod_mul(TSFiBigInt *r, const TSFiBigInt *a, const TSFiBigInt *b, const TSFiBigInt *m) {
    TSFiBigInt *tmp = tsfi_bn_alloc();
    tsfi_bn_mul_avx512(tmp, a, b);
    tsfi_bn_div_avx512(NULL, r, tmp, m);
    tsfi_bn_free(tmp);
}

// --- Batched AVX-512 IFMA Montgomery Logic (Single Limb) ---

static inline __m512i mont_mul_batch8(__m512i v_a, __m512i v_b, __m512i v_m, __m512i v_k0) {
    __m512i v_lo = _mm512_madd52lo_epu64(_mm512_setzero_si512(), v_a, v_b);
    __m512i v_hi = _mm512_madd52hi_epu64(_mm512_setzero_si512(), v_a, v_b);
    __m512i v_q  = _mm512_madd52lo_epu64(_mm512_setzero_si512(), v_lo, v_k0);
    v_q = _mm512_and_si512(v_q, _mm512_set1_epi64(TSFI_LIMB_MASK));
    __m512i v_red_hi = _mm512_madd52hi_epu64(v_hi, v_q, v_m);
    __m512i v_red_lo = _mm512_madd52lo_epu64(v_lo, v_q, v_m);
    __m512i v_carry = _mm512_srli_epi64(v_red_lo, TSFI_LIMB_BITS);
    __m512i v_res = _mm512_add_epi64(v_red_hi, v_carry);
    __mmask8 v_gt = _mm512_cmpge_epu64_mask(v_res, v_m);
    v_res = _mm512_mask_sub_epi64(v_res, v_gt, v_res, v_m);
    return v_res;
}

void tsfi_bn_mod_setup_batch8(TSFiModContextBatch8 *ctx, TSFiBigInt **mods) {
    memset(ctx, 0, sizeof(*ctx));
    ctx->even_mask = 0;
    for (int i = 0; i < 8; i++) {
        if (mods[i]->active_limbs > 1) {
            ctx->even_mask |= (1 << i);
            continue;
        }
        uint64_t m = mods[i]->limbs[0];
        if (m == 0) { ctx->even_mask |= (1 << i); continue; }
        ctx->m[i] = m;
        
        if ((m & 1) == 0) {
            ctx->even_mask |= (1 << i);
            continue;
        }
        ctx->k0[i] = calc_k0(m);
        ctx->R[i] = ((unsigned __int128)1 << TSFI_LIMB_BITS) % m;
        ctx->RR[i] = ((unsigned __int128)ctx->R[i] * ctx->R[i]) % m;
    }
}

static uint64_t scalar_mont_mul(uint64_t a, uint64_t b, uint64_t m, uint64_t k0) {
    unsigned __int128 prod = (unsigned __int128)a * b;
    uint64_t q = (uint64_t)(prod * k0) & TSFI_LIMB_MASK;
    unsigned __int128 red = prod + (unsigned __int128)q * m;
    uint64_t res = (uint64_t)(red >> TSFI_LIMB_BITS);
    if (res >= m) res -= m;
    return res;
}

void tsfi_bn_modpow_avx512(TSFiBigInt *result, const TSFiBigInt *base, const TSFiBigInt *exp, const TSFiBigInt *mod) {
    if (mod->active_limbs == 0) { tsfi_bn_set_u64(result, 0); return; }
    if (mod->active_limbs == 1 && mod->limbs[0] <= 1) { tsfi_bn_set_u64(result, 0); return; }

    int n_base = (int)base->active_limbs;
    while(n_base > 0 && base->limbs[n_base-1] == 0) n_base--;
    if (n_base == 0) { tsfi_bn_set_u64(result, 0); return; }
    if (n_base == 1 && base->limbs[0] == 1) { tsfi_bn_set_u64(result, 1); return; }
    
    if (mod->active_limbs == 1 && (mod->limbs[0] & 1)) {
        uint64_t m = mod->limbs[0];
        uint64_t k0 = calc_k0(m);
        uint64_t R = ((unsigned __int128)1 << TSFI_LIMB_BITS) % m;
        uint64_t RR = ((unsigned __int128)R * R) % m;
        uint64_t b = (base->active_limbs > 0) ? (base->limbs[0] % m) : 0;
        uint64_t b_m = scalar_mont_mul(b, RR, m, k0);
        uint64_t res_m = R;

        int start_limb = (int)exp->active_limbs - 1;
        while(start_limb >= 0 && exp->limbs[start_limb] == 0) start_limb--;
        if (start_limb < 0) { tsfi_bn_set_u64(result, 1); return; }
        
        bool started = false;
        for (int i = start_limb; i >= 0; i--) {
            uint64_t e_limb = exp->limbs[i];
            for (int bit = 63; bit >= 0; bit--) {
                if (!started) {
                    if ((e_limb >> bit) & 1) started = true;
                    else continue;
                }
                res_m = scalar_mont_mul(res_m, res_m, m, k0);
                if ((e_limb >> bit) & 1) res_m = scalar_mont_mul(res_m, b_m, m, k0);
            }
        }
        uint64_t res = scalar_mont_mul(res_m, 1, m, k0);
        tsfi_bn_set_u64(result, res);
        return;
    }

    if (mod->active_limbs == 1) {
        uint64_t m = mod->limbs[0];
        TSFiBigInt rem; memset(&rem, 0, sizeof(rem));
        tsfi_bn_div_avx512(NULL, &rem, base, mod);
        uint64_t b = (rem.active_limbs > 0) ? rem.limbs[0] : 0;
        uint64_t r = 1;
        int start_limb = (int)exp->active_limbs - 1;
        while(start_limb >= 0 && exp->limbs[start_limb] == 0) start_limb--;
        if (start_limb < 0) { tsfi_bn_set_u64(result, 1); return; }
        bool started = false;
        for (int i = start_limb; i >= 0; i--) {
            uint64_t e_limb = exp->limbs[i];
            for (int bit = 63; bit >= 0; bit--) {
                if (!started) { if ((e_limb >> bit) & 1) started = true; else continue; }
                r = (uint64_t)(((unsigned __int128)r * r) % m);
                if ((e_limb >> bit) & 1) r = (uint64_t)(((unsigned __int128)r * b) % m);
            }
        }
        tsfi_bn_set_u64(result, r);
        return;
    }
    tsfi_bn_set_u64(result, 0);
}

void tsfi_bn_modpow_batch8_ctx(TSFiBigInt **results, TSFiBigInt **bases, TSFiBigInt **exps, TSFiBigInt **mods, const TSFiModContextBatch8 *ctx) {
    alignas(64) uint64_t a_bases[8], a_exps[8];
    __mmask8 v_fallback_mask = ctx->even_mask;
    
    for (int i = 0; i < 8; i++) {
        if ((v_fallback_mask >> i) & 1) {
            tsfi_bn_modpow_avx512(results[i], bases[i], exps[i], mods[i]);
            continue;
        }
        if (exps[i]->active_limbs > 1 || bases[i]->active_limbs > 1) {
            tsfi_bn_modpow_avx512(results[i], bases[i], exps[i], mods[i]);
            v_fallback_mask |= (1 << i);
            continue;
        }
        a_bases[i] = (bases[i]->active_limbs > 0) ? (bases[i]->limbs[0] % ctx->m[i]) : 0;
        a_exps[i] = (exps[i]->active_limbs > 0) ? exps[i]->limbs[0] : 0;
    }

    if (v_fallback_mask == 0xFF) return;

    __m512i v_m = _mm512_loadu_si512(ctx->m);
    __m512i v_k0 = _mm512_loadu_si512(ctx->k0);
    __m512i v_R = _mm512_loadu_si512(ctx->R);
    __m512i v_RR = _mm512_loadu_si512(ctx->RR);
    __m512i v_b = _mm512_load_si512(a_bases);
    __m512i v_e = _mm512_load_si512(a_exps);

    __m512i v_bm = mont_mul_batch8(v_b, v_RR, v_m, v_k0);
    __m512i v_resm = v_R;

    for (int bit = 63; bit >= 0; bit--) {
        __m512i v_bit_mask = _mm512_set1_epi64(1ULL << bit);
        __mmask8 v_bits = _mm512_test_epi64_mask(v_e, v_bit_mask);
        v_resm = mont_mul_batch8(v_resm, v_resm, v_m, v_k0);
        __mmask8 v_up_mask = v_bits & (~v_fallback_mask);
        if (v_up_mask) v_resm = _mm512_mask_mov_epi64(v_resm, v_up_mask, mont_mul_batch8(v_resm, v_bm, v_m, v_k0));
    }

    __m512i v_final = mont_mul_batch8(v_resm, _mm512_set1_epi64(1), v_m, v_k0);
    alignas(64) uint64_t a_final[8];
    _mm512_store_si512(a_final, v_final);
    for (int i = 0; i < 8; i++) {
        if (!((v_fallback_mask >> i) & 1)) tsfi_bn_set_u64(results[i], a_final[i]);
    }
}

void tsfi_bn_modpow_batch8(TSFiBigInt **results, TSFiBigInt **bases, TSFiBigInt **exps, TSFiBigInt **mods) {
    TSFiModContextBatch8 ctx;
    tsfi_bn_mod_setup_batch8(&ctx, mods);
    tsfi_bn_modpow_batch8_ctx(results, bases, exps, mods, &ctx);
}

void tsfi_bn_drain_pool(void) {
    for (int i = 0; i < g_bn_pool_count; i++) {
        if (g_bn_pool[i]) { lau_free(g_bn_pool[i]); g_bn_pool[i] = NULL; }
    }
    g_bn_pool_count = 0;
}

void tsfi_bn_lshift_avx512(TSFiBigInt *result, const TSFiBigInt *src, int bits) {
    if (bits == 0) {
        tsfi_bn_copy(result, src);
        return;
    }
    int limbs_shift = bits / 64;
    int bit_shift = bits % 64;
    
    uint64_t carry = 0;
    for (int i = 0; i < 8; i++) {
        int dst_idx = i + limbs_shift;
        if (dst_idx < 8) {
            uint64_t val = src->limbs[i];
            result->limbs[dst_idx] = (val << bit_shift) | carry;
            carry = (bit_shift > 0) ? (val >> (64 - bit_shift)) : 0;
        }
    }
    for (int i = 0; i < limbs_shift && i < 8; i++) {
        result->limbs[i] = 0;
    }
}

void tsfi_bn_rshift_avx512(TSFiBigInt *result, const TSFiBigInt *src, int bits) {
    if (bits == 0) {
        tsfi_bn_copy(result, src);
        return;
    }
    int limbs_shift = bits / 64;
    int bit_shift = bits % 64;
    
    uint64_t carry = 0;
    for (int i = 7; i >= 0; i--) {
        int dst_idx = i - limbs_shift;
        if (dst_idx >= 0) {
            uint64_t val = src->limbs[i];
            result->limbs[dst_idx] = (val >> bit_shift) | carry;
            carry = (bit_shift > 0) ? (val << (64 - bit_shift)) : 0;
        }
    }
    for (int i = 8 - limbs_shift; i < 8 && i >= 0; i++) {
        result->limbs[i] = 0;
    }
}
