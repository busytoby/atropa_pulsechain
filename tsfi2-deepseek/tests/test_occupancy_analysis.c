#include "tsfi_time.h"
#include "tsfi_hilbert.h"
#include "tsfi_vision.h"
#include "lau_memory.h"
#include "tsfi_types.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include <immintrin.h>

#define BASE_DIM 256
#define TOTAL_PIXELS (BASE_DIM * BASE_DIM)

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

// --- Occupancy Metrics ---

typedef struct {
    int lit_pixels;
    float occupancy_ratio;
    uint8_t peak_intensity;
} TSFiOccupancyResult;

TSFiOccupancyResult analyze_occupancy(const uint8_t *BASE, size_t size) {
    TSFiOccupancyResult res = {0, 0.0f, 0};
    res.peak_intensity = tsfi_vision_max_value_raw(BASE, size);
    for(size_t i=0; i<size; i++) {
        if(BASE[i] > 128) res.lit_pixels++;
    }
    res.occupancy_ratio = (float)res.lit_pixels / size;
    return res;
}

int main() {
    printf("[TEST] Vision of Occupancy: Logical Field Analysis\n");

    TSFiHilbertGlyph g;
    tsfi_hilbert_project_box(&g, M_PI, M_PI);

    uint8_t *BASE = (uint8_t*)lau_memalign(64, TOTAL_PIXELS);
    float step = (2.0f * M_PI) / BASE_DIM;
    
    printf("[OCC] Rasterizing Hilbert Field into %dx%d BASE...\n", BASE_DIM, BASE_DIM);
    unsigned long long start = get_time_ns();
    
    for(int y=0; y<BASE_DIM; y++) {
        for(int x=0; x<BASE_DIM; x++) {
            float wx = -M_PI + (float)x * step;
            float wy = -M_PI + (float)y * step;
            float val = tsfi_hilbert_eval(&g, wx, wy);
            int iv = (int)(val * 255.0f);
            if(iv > 255) iv = 255;
            if(iv < 0) iv = 0;
            BASE[y * BASE_DIM + x] = (uint8_t)iv;
        }
    }
    
    unsigned long long end = get_time_ns();
    double dt = (double)(end - start) / 1e9;
    printf("[OCC] Rasterization Time: %.4f sec\n", dt);

    TSFiOccupancyResult res = analyze_occupancy(BASE, TOTAL_PIXELS);
    printf("      Peak Intensity:  %u\n", res.peak_intensity);
    printf("      Lit Pixels:      %d\n", res.lit_pixels);
    printf("      Occupancy Ratio: %.2f%%\n", res.occupancy_ratio * 100.0f);

    assert(res.occupancy_ratio > 0.20f && res.occupancy_ratio < 0.30f);
    assert(res.peak_intensity > 200);

    printf("[PASS] Vision of Occupancy confirmed.\n");
    lau_free(BASE);
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
