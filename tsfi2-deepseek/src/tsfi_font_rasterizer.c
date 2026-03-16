#include "tsfi_c_math.h"
#include "tsfi_font_vectors.h"
#include "tsfi_fourier.h"
#include "tsfi_wave_any.h"
#include <immintrin.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include <stdalign.h>

#define TILE_SIZE 64
#define PI ((double)TSFI_SECRET_CORE)

// --- Math Helpers ---

static inline __m512 _mm512_cos_ps_approx(__m512 x) {
    __m512 tp = _mm512_set1_ps(0.15915494309189533576888f);
    x = _mm512_mul_ps(x, tp);
    x = _mm512_sub_ps(x, _mm512_roundscale_ps(x, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC));
    __m512 c0 = _mm512_set1_ps(0.9999932946f);
    __m512 c1 = _mm512_set1_ps(-4.9348022005f);
    __m512 c2 = _mm512_set1_ps(4.0587121264f);
    __m512 x2 = _mm512_mul_ps(x, x);
    __m512 res = _mm512_fmadd_ps(c2, x2, c1);
    res = _mm512_fmadd_ps(res, x2, c0);
    return res;
}

static inline __m512 _mm512_sin_ps_approx(__m512 x) {
    __m512 tp = _mm512_set1_ps(0.15915494309189533576888f);
    x = _mm512_mul_ps(x, tp);
    x = _mm512_sub_ps(x, _mm512_roundscale_ps(x, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC));
    __m512 s1 = _mm512_set1_ps(6.283185307f);
    __m512 s3 = _mm512_set1_ps(-41.3417f);
    __m512 s5 = _mm512_set1_ps(81.6052f);
    __m512 x2 = _mm512_mul_ps(x, x);
    __m512 res = _mm512_fmadd_ps(s5, x2, s3);
    res = _mm512_fmadd_ps(res, x2, s1);
    res = _mm512_mul_ps(res, x);
    return res;
}

// --- Core Logic (Raw Buffer) ---
// This function assumes 'buffer' is a 64x64 contiguous block (4096 bytes).
// It performs the pure mathematical rasterization.
static void tsfi_font_rasterize_logical_avx512_internal(uint8_t *buffer, const TSFiFourierGlyph *glyph) {
    if (!buffer || !glyph) return;

    float intensity_x[TILE_SIZE * TILE_SIZE];
    float intensity_y[TILE_SIZE * TILE_SIZE];
    memset(intensity_x, 0, sizeof(intensity_x));
    memset(intensity_y, 0, sizeof(intensity_y));

    float u_coords[TILE_SIZE * TILE_SIZE];
    float v_coords[TILE_SIZE * TILE_SIZE];
    for (int y = 0; y < TILE_SIZE; y++) {
        for (int x = 0; x < TILE_SIZE; x++) {
            u_coords[y * TILE_SIZE + x] = ((float)x / (float)TILE_SIZE) * 2.0f * (float)PI - (float)PI;
            v_coords[y * TILE_SIZE + x] = ((float)(TILE_SIZE - 1 - y) / (float)TILE_SIZE) * 2.0f * (float)PI - (float)PI;
        }
    }

    for (int n = 0; n < TSFI_FOURIER_HARMONICS; n++) {
        __m512 v_an = _mm512_set1_ps(glyph->coeffs[n][0]);
        __m512 v_bn = _mm512_set1_ps(glyph->coeffs[n][1]);
        __m512 v_cn = _mm512_set1_ps(glyph->coeffs[n][2]);
        __m512 v_dn = _mm512_set1_ps(glyph->coeffs[n][3]);
        
        float h = (float)(n + 1);
        __m512 v_h = _mm512_set1_ps(h);

        for (int i = 0; i < TILE_SIZE * TILE_SIZE; i += 16) {
            __m512 v_u = _mm512_loadu_ps(&u_coords[i]);
            __m512 v_v = _mm512_loadu_ps(&v_coords[i]);
            
            __m512 v_phase_u = _mm512_mul_ps(v_h, v_u);
            __m512 v_phase_v = _mm512_mul_ps(v_h, v_v);
            
            __m512 v_cos_u = _mm512_cos_ps_approx(v_phase_u);
            __m512 v_sin_u = _mm512_sin_ps_approx(v_phase_u);
            __m512 v_cos_v = _mm512_cos_ps_approx(v_phase_v);
            __m512 v_sin_v = _mm512_sin_ps_approx(v_phase_v);

            __m512 v_ix = _mm512_loadu_ps(&intensity_x[i]);
            __m512 v_iy = _mm512_loadu_ps(&intensity_y[i]);

            v_ix = _mm512_fmadd_ps(v_an, v_cos_u, v_ix);
            v_ix = _mm512_fmadd_ps(v_bn, v_sin_u, v_ix);
            
            v_iy = _mm512_fmadd_ps(v_cn, v_cos_v, v_iy);
            v_iy = _mm512_fmadd_ps(v_dn, v_sin_v, v_iy);

            _mm512_storeu_ps(&intensity_x[i], v_ix);
            _mm512_storeu_ps(&intensity_y[i], v_iy);
        }
    }

    for (int i = 0; i < TILE_SIZE * TILE_SIZE; i += 16) {
        __m512 v_ix = _mm512_loadu_ps(&intensity_x[i]);
        __m512 v_iy = _mm512_loadu_ps(&intensity_y[i]);
        
        __m512 v_field = _mm512_sqrt_ps(_mm512_add_ps(_mm512_mul_ps(v_ix, v_ix), _mm512_mul_ps(v_iy, v_iy)));
        
        __m512 v_edge0 = _mm512_set1_ps(4000.0f);
        __m512 v_edge1 = _mm512_set1_ps(4200.0f);
        __m512 v_range = _mm512_sub_ps(v_edge1, v_edge0);
        
        __m512 v_alpha = _mm512_div_ps(_mm512_sub_ps(v_field, v_edge0), v_range);
        v_alpha = _mm512_max_ps(_mm512_setzero_ps(), _mm512_min_ps(_mm512_set1_ps(1.0f), v_alpha));
        
        __m512 v_scale = _mm512_set1_ps(255.0f);
        __m512i v_u8 = _mm512_cvtps_epi32(_mm512_mul_ps(v_alpha, v_scale));
        
        __m128i v_packed = _mm512_cvtepi32_epi8(v_u8);
        _mm_storeu_si128((__m128i*)&buffer[i], v_packed);
    }
}

// --- Safe Wrapper (WaveStream) ---
// This is the primary API now.
void tsfi_font_rasterize_logical_avx512(const WaveStream *stream, const TSFiFourierGlyph *glyph) {
    if (!stream || !glyph) return;
    
    // 1. Rasterize to local stack buffer (Contiguous, Fast)
    alignas(64) uint8_t local_buffer[TILE_SIZE * TILE_SIZE];
    tsfi_font_rasterize_logical_avx512_internal(local_buffer, glyph);
    
    // 2. Safe Transport to Target Stream (Bijective Validation)
    // This handles splitting, stride, and checksumming automatically.
    wave_store_any(stream, 0, local_buffer, TILE_SIZE * TILE_SIZE);
}

// --- BDA Wrapper ---
// Adapts the raw BDA (Big Data Area) call to the Safe Wrapper.
void tsfi_font_rasterize_logical_bda(uint64_t target_addr, uint64_t map_addr, uint64_t logical_addr, uint32_t codepoint) {
    uint32_t *unicode_map = (uint32_t *)map_addr;
    TSFiFourierGlyph *logical_data = (TSFiFourierGlyph *)logical_addr;
    
    uint32_t gid = unicode_map[codepoint];
    if (gid == 0) return;
    
    // Construct Transient Stream Wrapper
    WaveStream stream = {
        .data = (void*)target_addr,
        .count = (TILE_SIZE * TILE_SIZE) / 64, // Assume standard 64-byte atom
        .atom_size = 64,
        .stride = 64,
        .flags = 0 // No checksum for legacy BDA, unless we want strict mode
    };
    
    // Use the safe path
    tsfi_font_rasterize_logical_avx512(&stream, &logical_data[gid]);
}

// --- Compatibility Stubs ---

void tsfi_font_rasterize_glyph_bda(uint64_t target_addr, uint64_t registry_addr, uint64_t segments_base_addr, uint64_t style_addr) {
    (void)target_addr; (void)registry_addr; (void)segments_base_addr; (void)style_addr;
}

void tsfi_font_rasterize_glyph_avx512(uint8_t *buffer, TSFiGlyphRegistryPacked *reg, TSFiPackedSegment *segments) {
    if (!buffer || !reg || !segments) return;
    // Legacy support, kept minimal
    memset(buffer, 0, 64*64);
}
