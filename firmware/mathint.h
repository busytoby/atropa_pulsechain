#define _POSIX_C_SOURCE 202405L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <immintrin.h>
#include "gemalloc.h"

typedef struct { uint64_t *v; int n; } MathInt;

static inline void mi_norm(MathInt *a) {
    while (a->n > 1 && a->v[a->n - 1] == 0) a->n--;
}

void mifree(MathInt i) { if (i.v) gemfree(i.v); }

// Hardware-accelerated comparison
static inline int micmp(MathInt a, MathInt b) {
    if (a.n != b.n) return a.n > b.n ? 1 : -1;
    for (int i = a.n - 1; i >= 0; i--) {
        if (a.v[i] != b.v[i]) return a.v[i] > b.v[i] ? 1 : -1;
    }
    return 0;
}

// Hardware-accelerated subtraction (SBB chain)
static inline unsigned char misub_inplace(MathInt *a, MathInt b) {
    unsigned char brw = 0;
    for (int i = 0; i < a->n; i++) {
        uint64_t val = (i < b.n) ? b.v[i] : 0;
        brw = _subborrow_u64(brw, a->v[i], val, (unsigned long long*)&a->v[i]);
    }
    mi_norm(a);
    return brw;
}

// Full product multiplication
MathInt mimul(MathInt a, MathInt b) {
    MathInt r = { jgemalloc((a.n + b.n) * 8, 0), a.n + b.n };
    memset(r.v, 0, r.n * 8);
    for (int i = 0; i < a.n; i++) {
        if (a.v[i] == 0) continue;
        uint64_t carry = 0;
        for (int j = 0; j < b.n; j++) {
            unsigned __int128 cur = (unsigned __int128)a.v[i] * b.v[j] + r.v[i+j] + carry;
            r.v[i+j] = (uint64_t)cur;
            carry = (uint64_t)(cur >> 64);
        }
        r.v[i + b.n] = carry;
    }
    mi_norm(&r);
    return r;
}

// --- Montgomery Reduction Logic ---

// Montgomery Step: res = (T + (T*m' mod R)*m) / R
// This replaces mimod during exponentiation.
MathInt montgomery_reduce(MathInt T, MathInt m, uint64_t m_inv) {
    int n = m.n;
    // T is roughly 2n limbs. We reduce it back to n limbs.
    for (int i = 0; i < n; i++) {
        uint64_t u = T.v[i] * m_inv;
        uint64_t carry = 0;
        for (int j = 0; j < n; j++) {
            unsigned __int128 cur = (unsigned __int128)u * m.v[j] + T.v[i + j] + carry;
            T.v[i + j] = (uint64_t)cur;
            carry = (uint64_t)(cur >> 64);
        }
        // Propagate carry through the rest of T
        for (int j = i + n; j < T.n && carry; j++) {
            unsigned char c_out = _addcarry_u64(0, T.v[j], carry, (unsigned long long*)&T.v[j]);
            carry = c_out;
        }
    }
    
    // Final result is the upper half of T
    MathInt res = { jgemalloc((n + 1) * 8, 0), n };
    memcpy(res.v, T.v + n, n * 8);
    mi_norm(&res);

    if (micmp(res, m) >= 0) misub_inplace(&res, m);
    return res;
}

// Modular Inverse for Montgomery: (m * m_inv) mod 2^64 == -1 mod 2^64
uint64_t compute_m_inv(uint64_t n0) {
    uint64_t inv = n0;
    for (int i = 0; i < 5; i++) inv *= 2 - n0 * inv;
    return -inv;
}

// --- Original Function Signatures ---

MathInt str2mi(const char* s) {
    MathInt b = { jgemalloc(8, 0), 1 }; b.v[0] = 0;
    for (int i = 0; s[i]; i++) {
        uint64_t carry = s[i] - '0';
        for (int j = 0; j < b.n || carry; j++) {
            if (j == b.n) { b.v = gemrealloc(b.v, (b.n + 1) * 8); b.v[b.n++] = 0; }
            unsigned __int128 cur = (unsigned __int128)b.v[j] * 10 + carry;
            b.v[j] = (uint64_t)cur;
            carry = (uint64_t)(cur >> 64);
        }
    }
    return b;
}

char* mi2str(MathInt b) {
    char* s = jgemalloc(b.n * 20 + 2, 0);
    int p = 0;
    MathInt t = { jgemalloc(b.n * 8, 0), b.n };
    memcpy(t.v, b.v, b.n * 8);
    while (t.n > 1 || t.v[0] > 0) {
        uint64_t rem = 0;
        for (int i = t.n - 1; i >= 0; i--) {
            unsigned __int128 cur = t.v[i] + ((unsigned __int128)rem << 64);
            t.v[i] = (uint64_t)(cur / 10);
            rem = (uint64_t)(cur % 10);
        }
        s[p++] = rem + '0';
        mi_norm(&t);
    }
    mifree(t);
    if (p == 0) s[p++] = '0';
    s[p] = '\0';
    for (int i = 0; i < p / 2; i++) { char tmp = s[i]; s[i] = s[p-1-i]; s[p-1-i] = tmp; }
    return s;
}

char* rmi(size_t length) {
    MathInt r = { gemalloc(length * 8), (int)length };
    char* res = mi2str(r);
    gemfree(r.v);
    return res;
}

// Standard mimod (used for initial setup only)
void mimod(MathInt *a, MathInt m) {
    if (micmp(*a, m) < 0) return;
    MathInt t = { jgemalloc((a->n + 1) * 8, 0), m.n };
    memcpy(t.v, m.v, m.n * 8);
    int s = ((a->n - 1) * 64 + (64 - __builtin_clzll(a->v[a->n - 1]))) - 
            ((m.n - 1) * 64 + (64 - __builtin_clzll(m.v[m.n - 1])));
    if (s > 0) {
        for (int i = 0; i < s; i++) {
            uint64_t c = 0;
            for (int j = 0; j < t.n; j++) {
                uint64_t nc = t.v[j] >> 63;
                t.v[j] = (t.v[j] << 1) | c;
                c = nc;
            }
            if (c) t.v[t.n++] = c;
        }
    }
    for (int i = 0; i <= s; i++) {
        if (micmp(*a, t) >= 0) misub_inplace(a, t);
        uint64_t c = 0;
        for (int j = t.n - 1; j >= 0; j--) {
            uint64_t nc = (t.v[j] & 1) << 63;
            t.v[j] = (t.v[j] >> 1) | c;
            c = nc;
        }
        mi_norm(&t);
    }
    mifree(t);
}

MathInt modPow(const char* sb, const char* se, const char* sm) {
    MathInt b = str2mi(sb), e = str2mi(se), m = str2mi(sm);
    uint64_t m_inv = compute_m_inv(m.v[0]);

    // Precompute R^2 mod m for Montgomery entry
    MathInt r2 = { jgemalloc((m.n * 2 + 1) * 8, 0), m.n * 2 + 1 };
    memset(r2.v, 0, r2.n * 8);
    r2.v[m.n * 2] = 1;
    mimod(&r2, m);

    // Enter Montgomery domain: b_bar = b * R mod m
    MathInt b_prod = mimul(b, r2);
    MathInt b_bar = montgomery_reduce(b_prod, m, m_inv);
    mifree(b_prod);
    
    // res_bar = 1 * R mod m
    MathInt one = str2mi("1");
    MathInt res_prod = mimul(one, r2);
    MathInt res_bar = montgomery_reduce(res_prod, m, m_inv);
    mifree(one);
    mifree(res_prod);

    while (e.n > 1 || e.v[0] > 0) {
        if (e.v[0] & 1) {
            MathInt tmp_p = mimul(res_bar, b_bar);
            MathInt next_res = montgomery_reduce(tmp_p, m, m_inv);
            mifree(res_bar);
            mifree(tmp_p);
            res_bar = next_res;
        }

        MathInt b_sq = mimul(b_bar, b_bar);
        MathInt next_b = montgomery_reduce(b_sq, m, m_inv);
        mifree(b_bar);
        mifree(b_sq);
        b_bar = next_b;

        uint64_t c = 0;
        for (int i = e.n - 1; i >= 0; i--) {
            uint64_t nc = (e.v[i] & 1) << 63;
            e.v[i] = (e.v[i] >> 1) | c;
            c = nc;
        }
        mi_norm(&e);
    }

    // Exit Montgomery domain: res = res_bar * 1 / R mod m
    MathInt final_res = montgomery_reduce(res_bar, m, m_inv);
    
    mifree(b); mifree(e); mifree(m); mifree(r2); mifree(res_bar); mifree(b_bar);
    return final_res;
}
