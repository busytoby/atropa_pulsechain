#ifndef TSFI_WAVE512_H
#define TSFI_WAVE512_H

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <immintrin.h>

// --- GCC Vector Extensions ---
typedef float v16sf __attribute__((vector_size(64)));
typedef int   v16si __attribute__((vector_size(64)));
typedef long long v8di  __attribute__((vector_size(64)));
typedef uint8_t v64qi __attribute__((vector_size(64)));

// --- Wave512 Fundamental Types ---
// Alignment removed per Project Mariner WAVE15 Migration Protocol.
// Capacity-based alignment is now handled by the container/allocator.
// Re-adding explicit alignment for data member safety.
typedef struct { v16sf z[8] __attribute__((aligned(64))); } wave512;
typedef struct { v16si z[8] __attribute__((aligned(64))); } wave512_i;
typedef struct { v64qi z[8] __attribute__((aligned(64))); } wave512_u;

// --- Architectural Aliases ---
typedef wave512 VE512;
typedef wave512_i VX512;

// --- Accessors ---
#define WAVE512_CAST_TO_I(w) ({ union { wave512 f; wave512_i i; } u; u.f = w; u.i; })
#define WAVE512_CAST_TO_F(w) ({ union { wave512_i i; wave512 f; } u; u.i = w; u.f; })

// --- Pure Assembly Macros ---
#define _W8_OP1(op, r, a) \
    for(int i=0; i<8; i++) __asm__ volatile (op " %1, %0" : "=v"(r.z[i]) : "v"(a.z[i]));

#define _W8_OP2(op, r, a, b) \
    for(int i=0; i<8; i++) __asm__ volatile (op " %2, %1, %0" : "=v"(r.z[i]) : "v"(a.z[i]), "v"(b.z[i]));

#define _W8_OP3(op, r, a, b, c) \
    for(int i=0; i<8; i++) { \
        r.z[i] = a.z[i]; \
        __asm__ volatile (op " %2, %1, %0" : "+v"(r.z[i]) : "v"(b.z[i]), "v"(c.z[i])); \
    }

// --- Memory (Unaligned by Default) ---
static inline wave512 VMOVUPS(const float *ptr) {
    wave512 r; const v16sf *vp = (const v16sf*)ptr;
    for(int i=0; i<8; i++) __asm__ volatile ("vmovups %1, %0" : "=v"(r.z[i]) : "m"(vp[i]));
    return r;
}
// Alias VMOVAPS to VMOVUPS for safety during migration
#define VMOVAPS VMOVUPS

static inline void VSTOREUPS(void *ptr, wave512 val) {
    v16sf *vp = (v16sf*)ptr;
    for(int i=0; i<8; i++) __asm__ volatile ("vmovups %1, %0" : "=m"(vp[i]) : "v"(val.z[i]));
}
// Alias VSTREAMPS to VSTOREUPS to avoid alignment faults
#define VSTREAMPS VSTOREUPS

static inline wave512_i VMOVDQU64(const void *ptr) {
    wave512_i r; const v16si *vp = (const v16si*)ptr;
    for(int i=0; i<8; i++) __asm__ volatile ("vmovdqu64 %1, %0" : "=v"(r.z[i]) : "m"(vp[i]));
    return r;
}
#define VMOVDQA64 VMOVDQU64

static inline wave512_u VMOVDQU_U8(const void *ptr) {
    wave512_u r; const v64qi *vp = (const v64qi*)ptr;
    for(int i=0; i<8; i++) __asm__ volatile ("vmovdqu64 %1, %0" : "=v"(r.z[i]) : "m"(vp[i]));
    return r;
}
#define VMOVDQA_U8 VMOVDQU_U8

// --- Broadcast ---
static inline wave512 wave512_set1(float f) {
    wave512 r; v16sf z; __asm__ volatile("vbroadcastss %1, %0" : "=v"(z) : "m"(f));
    for(int i=0; i<8; i++) r.z[i] = z; 
    return r;
}
#define VBROADCASTSS(ptr) wave512_set1(*(float*)(ptr))
static inline wave512_i VBROADCASTSD(const uint64_t *ptr) {
    wave512_i r; v16si z; __asm__ volatile("vbroadcastsd %1, %0" : "=v"(z) : "m"(*ptr));
    for(int i=0; i<8; i++) r.z[i] = z; 
    return r;
}

// --- Bitwise Logic ---
#define VPANDQ(a, b) ({ wave512_i _r; _W8_OP2("vpandq", _r, a, b); _r; })
#define VPORQ(a, b)  ({ wave512_i _r; _W8_OP2("vporq", _r, a, b); _r; })
#define VPXORQ(a, b) ({ wave512_i _r; _W8_OP2("vpxorq", _r, a, b); _r; })
#define VPANDNQ(a, b) ({ wave512_i _r; _W8_OP2("vpandnq", _r, a, b); _r; })

// --- Comparison (Vector Mask Semantics) ---
#define VPCMPGTB(a, b) ({ \
    wave512_u _r; \
    for(int i=0; i<8; i++) { \
        __mmask64 _m = _mm512_cmpgt_epi8_mask((__m512i)a.z[i], (__m512i)b.z[i]); \
        _r.z[i] = (v64qi)_mm512_maskz_set1_epi8(_m, 0xFF); \
    } \
    _r; \
})

#define VPCMPGTQ(a, b) ({ \
    wave512_i _r; \
    for(int i=0; i<8; i++) { \
        __mmask8 _m = _mm512_cmpgt_epi64_mask((__m512i)a.z[i], (__m512i)b.z[i]); \
        _r.z[i] = (v16si)_mm512_maskz_set1_epi64(_m, -1LL); \
    } \
    _r; \
})

// --- Vector Selection (Safety Blending) ---
#define VSELECTQ(mask, t_val, f_val) ({ \
    wave512_i _r; \
    for(int i=0; i<8; i++) { \
        __mmask8 _m = _mm512_movepi64_mask((__m512i)mask.z[i]); \
        _r.z[i] = (v16si)_mm512_mask_blend_epi64(_m, (__m512i)f_val.z[i], (__m512i)t_val.z[i]); \
    } \
    _r; \
})

// --- Arithmetic ---
#define VADDPS(a, b) ({ wave512 _r; _W8_OP2("vaddps", _r, a, b); _r; })
#define VSUBPS(a, b) ({ wave512 _r; _W8_OP2("vsubps", _r, a, b); _r; })
#define VMULPS(a, b) ({ wave512 _r; _W8_OP2("vmulps", _r, a, b); _r; })
#define VDIVPS(a, b) ({ wave512 _r; _W8_OP2("vdivps", _r, a, b); _r; })
#define VMINPS(a, b) ({ wave512 _r; _W8_OP2("vminps", _r, a, b); _r; })
#define VMAXPS(a, b) ({ wave512 _r; _W8_OP2("vmaxps", _r, a, b); _r; })
#define VFMADD213PS(a, b, c) ({ wave512 _r; _W8_OP3("vfmadd213ps", _r, a, b, c); _r; })
#define VFMA_RELU_PS(a, b, c) ({ wave512 _r; _W8_OP3("vfmadd213ps", _r, a, b, c); wave512 _z = VZERO(); _r = VMAXPS(_r, _z); _r; })

// --- Math ---
#define VSQRTPS(a)  ({ wave512 _r; _W8_OP1("vsqrtps", _r, a); _r; })
#define VRSQRTPS(a) ({ wave512 _r; _W8_OP1("vrsqrt14ps", _r, a); _r; })

// --- Pattern Matching ---
#define VDBPSADBW(a, b, imm) ({ \
    wave512_i _r; \
    for(int i=0; i<8; i++) _r.z[i] = (v16si)_mm512_dbsad_epu8((__m512i)a.z[i], (__m512i)b.z[i], imm); \
    _r; \
})

// --- Integer Math ---
#define VPADDQ(a, b) ({ wave512_i _r; _W8_OP2("vpaddq", _r, a, b); _r; })
#define VPABSQ(a)    ({ wave512_i _r; _W8_OP1("vpabsq", _r, a); _r; })
#define VPMAXUB(a, b) ({ wave512_u _r; _W8_OP2("vpmaxub", _r, a, b); _r; })
#define VPMADD52LUQ(acc, src, mult) ({ wave512_i _r; _W8_OP3("vpmadd52luq", _r, acc, src, mult); _r; })
#define VPMADD52HUQ(acc, src, mult) ({ wave512_i _r; _W8_OP3("vpmadd52huq", _r, acc, src, mult); _r; })

// --- Zero ---
#define VZERO() ({ wave512 _r; v16sf _z = {0}; for(int i=0; i<8; i++) _r.z[i] = _z; _r; })
#define VZERO_I() ({ wave512_i _r; v16si _z = {0}; for(int i=0; i<8; i++) _r.z[i] = _z; _r; })

// --- Permute ---
#define VPERMPS_IMM(idx, a) ({ \
    wave512 _r; int _idx = (int)(idx); v16si _vidx; \
    __asm__ volatile("vpbroadcastd %1, %0" : "=v"(_vidx) : "m"(_idx)); \
    for(int i=0; i<8; i++) __asm__ volatile("vpermps %1, %2, %0" : "=v"(_r.z[i]) : "v"(a.z[i]), "v"(_vidx)); \
    _r; \
})

// --- Comparison ---
static inline bool VPCMPEQB(const wave512_u *a, const wave512_u *b) { return memcmp(a, b, 512) == 0; }
static inline bool VPCMPEQQ(const wave512_i *a, const wave512_i *b) { return memcmp(a, b, 512) == 0; }

#endif // TSFI_WAVE512_H
