#include "tsfi_c_math.h"
#include <math.h>
#include <immintrin.h>
#include <stdio.h>
#include "tsfi_hilbert.h"
#include "tsfi_types.h"
#include "lau_memory.h"
#include "tsfi_wave512.h"
#include <stddef.h>
#include <string.h>

// Helper: Init profile if missing
static void init_default_profile(TSFiMathProfile *p) {
    float pi = ((float)TSFI_SECRET_CORE);
    p->domain_scaler = 1.0f / (2.0f * pi);
    // Cosine: 1 - c2*x^2 + c4*x^4 - c6*x^6
    float pi2 = 2.0f * pi;
    float pi2_2 = pi2 * pi2;
    p->cos_coeffs[0] = 1.0f;
    p->cos_coeffs[1] = -0.5f * pi2_2;
    p->cos_coeffs[2] = (1.0f/24.0f) * pi2_2 * pi2_2;
    p->cos_coeffs[3] = -(1.0f/720.0f) * pi2_2 * pi2_2 * pi2_2;
    
    // Sine: 2pix - (2pix)^3/6 ...
    p->sin_coeffs[0] = pi2;
    p->sin_coeffs[1] = -(1.0f/6.0f) * pi2_2 * pi2;
    p->sin_coeffs[2] = (1.0f/120.0f) * pi2_2 * pi2_2 * pi2;
    p->sin_coeffs[3] = -(1.0f/5040.0f) * pi2_2 * pi2_2 * pi2_2 * pi2;
}

// AlphaFold Kernel: Fold Banach Sequence to Hilbert Coefficients
// Input: Sequence data (bytes)
// Output: TSFiHilbertGlyph (coeffs)
// Parallelism: Processes 16 floats (16 bytes input) per AVX-512 vector
void tsfi_hilbert_fold_sequence_avx512(TSFiHilbertGlyph *g, const char *seq, size_t len) {
    if (!g || !seq || len == 0) return;
    
    // Initialize Glyph (Clear State)
    memset(g, 0, sizeof(TSFiHilbertGlyph));
    
    size_t capacity = TSFI_HILBERT_DIM * TSFI_HILBERT_DIM * 2;
    size_t limit = len > capacity ? capacity : len; 
    
    // Use raw __m512 for standard AVX operations
    __m512 vNorm = _mm512_set1_ps(1.0f / 255.0f);
    
    size_t i = 0;
    // Process 16 bytes at a time -> 16 floats
    if (limit >= 16) {
        for (; i <= limit - 16; i += 16) {
            // Load 16 bytes (unaligned) into XMM register
            __m128i vBytes = _mm_loadu_si128((const __m128i*)&seq[i]);
            
            // Zero-extend 16 bytes to 16 32-bit integers in ZMM
            __m512i vInts = _mm512_cvtepu8_epi32(vBytes);
            
            // Convert integers to float
            __m512 vFloats = _mm512_cvtepi32_ps(vInts);
            
            // Normalize [0..255] -> [0.0..1.0]
            vFloats = _mm512_mul_ps(vFloats, vNorm);
            
            // Store 16 floats directly into the matrix linear buffer
            // (TSFiHilbertGlyph coeffs is contiguous float array)
            float *dest = (float*)((char*)g->coeffs + i * sizeof(float)); 
            
            _mm512_storeu_ps(dest, vFloats);
        }
    }
    
    // Scalar tail
    for (; i < limit; i++) {
        float *dest = (float*)((char*)g->coeffs + i * sizeof(float));
        *dest = (float)((unsigned char)seq[i]) / 255.0f;
    }
}

void tsfi_hilbert_batch_avx512(void *ctx, const float *xy_in, float *out, int count) {
    // 1. Setup Profile (Local, no R15 dependency)
    TSFiMathProfile local_prof;
    init_default_profile(&local_prof);
    const TSFiMathProfile *prof = &local_prof;

    // 2. Resolve Context
    WaveSystem *ws = (WaveSystem*)ctx;
    if (!ws) return;

    // Locate Header
    // We try to find if this ctx is part of a LauWiredHeader.
    // If it was allocated via lau_malloc_wired, the payload starts at offsetof(LauWiredHeader, payload).
    LauWiredHeader *h = (LauWiredHeader *)((char *)ws - offsetof(LauWiredHeader, payload));
    
    // Safety check: is the footer magic correct?
    // Using a more robust check for embedded glyphs.
    const TSFiHilbertGlyph *g = NULL;
    
    if (h->footer.magic == LAU_MAGIC || h->footer.magic == 0x42) {
        g = (const TSFiHilbertGlyph*)h->regs.sgpr;
    } else {
        // Fallback: check if active_hilbert_glyph is set in the struct itself
        if (ws->active_hilbert_glyph) {
            g = (const TSFiHilbertGlyph*)ws->active_hilbert_glyph;
        } else {
            return; // No glyph found
        }
    }

    // Basic validity check of glyph
    if (!g || (g->coeffs[0][0].real == 0.0f && g->coeffs[0][0].imag == 0.0f)) {
        if (ws->active_hilbert_glyph) {
            g = (const TSFiHilbertGlyph*)ws->active_hilbert_glyph;
        }
        if (!g || (g->coeffs[0][0].real == 0.0f && g->coeffs[0][0].imag == 0.0f)) {
             return; // Empty glyph
        }
    }

    // 3. Constants
    wave512 tp = wave512_set1(prof->domain_scaler);
    wave512 c0 = wave512_set1(prof->cos_coeffs[0]);
    wave512 c2 = wave512_set1(prof->cos_coeffs[1]);
    wave512 c4 = wave512_set1(prof->cos_coeffs[2]);
    wave512 c6 = wave512_set1(prof->cos_coeffs[3]);
    
    wave512 s1 = wave512_set1(prof->sin_coeffs[0]);
    wave512 s3 = wave512_set1(prof->sin_coeffs[1]);
    wave512 s5 = wave512_set1(prof->sin_coeffs[2]);
    wave512 s7 = wave512_set1(prof->sin_coeffs[3]);

    int i = 0;
    for (; i <= count - 128; i += 128) {
        // Load 128 pairs via scalar temp buffer (reliable)
        float buf_x[128];
        float buf_y[128];
        for(int k=0; k<128; k++) {
            buf_x[k] = xy_in[(i+k)*2];
            buf_y[k] = xy_in[(i+k)*2+1];
        }
        wave512 vx = VMOVUPS(buf_x);
        wave512 vy = VMOVUPS(buf_y);
        
        wave512 sum = VZERO();

        for (int n = -TSFI_HILBERT_ORDER; n <= TSFI_HILBERT_ORDER; n++) {
            for (int m = -TSFI_HILBERT_ORDER; m <= TSFI_HILBERT_ORDER; m++) {
                int row = n + TSFI_HILBERT_ORDER;
                int col = m + TSFI_HILBERT_ORDER;
                
                float cr = g->coeffs[row][col].real;
                float ci = g->coeffs[row][col].imag;
                if (__builtin_fabsf(cr) < 1e-6 && __builtin_fabsf(ci) < 1e-6) continue;

                wave512 vn = wave512_set1((float)n);
                wave512 vm = wave512_set1((float)m);
                
                // phase = n*vx + m*vy
                wave512 phase = VFMADD213PS(vn, vx, VMULPS(vm, vy));
                
                // Range reduce: x * tp
                wave512 x = VMULPS(phase, tp);
                
                // x -= round(x)
                wave512 rnd;
                for(int k=0; k<8; k++) {
                    __asm__ volatile ("vrndscaleps $8, %1, %0" : "=v"(rnd.z[k]) : "v"(x.z[k]));
                }
                x = VSUBPS(x, rnd);
                
                wave512 x2 = VMULPS(x, x);
                
                // Cos: c0 + x2*(c2 + x2*(c4 + x2*c6))
                wave512 vcos = VFMADD213PS(c6, x2, c4);
                vcos = VFMADD213PS(vcos, x2, c2);
                vcos = VFMADD213PS(vcos, x2, c0);
                
                // Sin: x * (s1 + x2*(s3 + x2*(s5 + x2*s7)))
                wave512 vsin = VFMADD213PS(s7, x2, s5);
                vsin = VFMADD213PS(vsin, x2, s3);
                vsin = VFMADD213PS(vsin, x2, s1);
                vsin = VMULPS(vsin, x);
                
                wave512 vcr = wave512_set1(cr);
                wave512 vci = wave512_set1(ci);
                
                // term = cr*cos - ci*sin
                wave512 term = VSUBPS(VMULPS(vcr, vcos), VMULPS(vci, vsin));
                sum = VADDPS(sum, term);
            }
        }
        
        // Store
        __asm__ volatile (
            "vmovups 0(%1), %%zmm0; vmovups %%zmm0, 0(%0) \n\t"
            "vmovups 64(%1), %%zmm0; vmovups %%zmm0, 64(%0)"
            :: "r"(&out[i]), "r"(&sum) : "zmm0", "memory"
        );
    }
}

void tsfi_hilbert_batch_wide_avx512(void *ctx, const float *xy_in, float *out, int count) {
    tsfi_hilbert_batch_avx512(ctx, xy_in, out, count);
}
