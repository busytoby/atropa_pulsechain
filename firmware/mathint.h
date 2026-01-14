#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "gemalloc.h"

typedef struct { uint64_t *v; int n; } MathInt;

void mifree(MathInt i) { gemfree(i.v); }

// High-speed string to MathInt (Base-10 to Base-2^64)
MathInt str2mi(const char* s) {
    MathInt b = { jgemalloc(sizeof(uint64_t), 0), 1 };
    for (int i = 0; s[i]; i++) {
        uint64_t carry = s[i] - '0';
        for (int j = 0; j < b.n || carry; j++) {
            if (j == b.n) {
                b.v = gemrealloc(b.v, (b.n + 1) * sizeof(uint64_t));
                b.v[b.n++] = 0;
            }   
            unsigned __int128 cur = (unsigned __int128)b.v[j] * 10 + carry;
            b.v[j] = (uint64_t)cur;
            carry = (uint64_t)(cur >> 64);
        }   
    }   
    return b;
}   

// Comparison (1: a>b, -1: a<b, 0: a==b)
int micmp(MathInt a, MathInt b) {
    if (a.n != b.n) return a.n > b.n ? 1 : -1;
    for (int i = a.n - 1; i >= 0; i--) if (a.v[i] != b.v[i]) return a.v[i] > b.v[i] ? 1 : -1;
    return 0;
}

// Subtraction: a = a - b (assumes a >= b)
void misub(MathInt *a, MathInt b) {
    uint64_t borrow = 0;
    for (int i = 0; i < a->n; i++) {
        uint64_t val = (i < b.n) ? b.v[i] : 0;
        uint64_t next_borrow = (a->v[i] < (unsigned __int128)val + borrow);
        a->v[i] = a->v[i] - val - borrow;
        borrow = next_borrow;
    }
    while (a->n > 1 && a->v[a->n - 1] == 0) a->n--;
}

// Fast Modulo using bit-shifts (a = a % m)
void mimod(MathInt *a, MathInt m) {
    if (micmp(*a, m) < 0) return;
    MathInt t = { jgemalloc(a->n * sizeof(uint64_t), 0), m.n };
    memcpy(t.v, m.v, m.n * 8);
    int s = 0;
    while (micmp(*a, t) >= 0) {
        uint64_t carry = t.v[t.n - 1] >> 63;
        for (int i = t.n - 1; i >= 0; i--) t.v[i] = (t.v[i] << 1) | (i > 0 ? t.v[i - 1] >> 63 : 0);
        if (carry) t.v[t.n++] = 1;
        s++;
    }
    for (int i = 0; i <= s; i++) {
        if (micmp(*a, t) >= 0) misub(a, t);
        for (int j = 0; j < t.n; j++) t.v[j] = (t.v[j] >> 1) | (j < t.n - 1 ? t.v[j + 1] << 63 : 0);
        if (t.n > 1 && t.v[t.n - 1] == 0) t.n--;
    }
    mifree(t);
}

// Multiplication: r = a * b
MathInt mimul(MathInt a, MathInt b) {
    MathInt r = { jgemalloc((a.n + b.n) * sizeof(uint64_t), 0), a.n + b.n };
    for (int i = 0; i < a.n; i++) {
        uint64_t carry = 0;
        for (int j = 0; j < b.n || carry; j++) {
            unsigned __int128 cur = (unsigned __int128)a.v[i] * (j < b.n ? b.v[j] : 0) + r.v[i + j] + carry;
            r.v[i + j] = (uint64_t)cur;
            carry = (uint64_t)(cur >> 64);
        }
    }
    while (r.n > 1 && r.v[r.n - 1] == 0) r.n--;
    return r;
}

// Convert MathInt back to String
char* mi2str(MathInt b) {
    char* s = jgemalloc(b.n * 20 + 2, 0);
    int p = 0;
    MathInt t = { jgemalloc(b.n * sizeof(uint64_t), 0), b.n };
    memcpy(t.v, b.v, b.n * 8);
    while (t.n > 1 || t.v[0] > 0) {
        uint64_t rem = 0;
        for (int i = t.n - 1; i >= 0; i--) {
            unsigned __int128 cur = t.v[i] + ((unsigned __int128)rem << 64);
            t.v[i] = (uint64_t)(cur / 10);
            rem = (uint64_t)(cur % 10);
        }
        s[p++] = rem + '0';
        while (t.n > 1 && t.v[t.n - 1] == 0) t.n--;
    }
    mifree(t);
    if (p == 0) s[p++] = '0';
    for (int i = 0; i < p / 2; i++) { char tmp = s[i]; s[i] = s[p - 1 - i]; s[p - 1 - i] = tmp; }
    return s;
}

MathInt modPow(const char* sb, const char* se, const char* sm) {
    MathInt b = str2mi(sb), e = str2mi(se), m = str2mi(sm), res = str2mi("1");
    mimod(&b, m);
    while (e.n > 1 || e.v[0] > 0) {
        if (e.v[0] & 1) {
            MathInt tmp = mimul(res, b); mifree(res); mimod(&tmp, m); res = tmp;
        }
        MathInt tmp = mimul(b, b); mifree(b); mimod(&tmp, m); b = tmp;
        // e >>= 1
        uint64_t rem = 0;
        for (int i = e.n - 1; i >= 0; i--) {
            uint64_t next_rem = e.v[i] & 1;
            e.v[i] = (e.v[i] >> 1) | (rem << 63);
            rem = next_rem;
        }
        if (e.n > 1 && e.v[e.n - 1] == 0) e.n--;
    }
    //char* rs = mi2str(res);
    mifree(b); mifree(e); mifree(m);
    return res;
}

char* rmi(size_t length) {
  MathInt r = { gemalloc(length * sizeof(uint64_t)), length };
  char* res = mi2str(r);
  gemfree(r.v);
  return res;
}
