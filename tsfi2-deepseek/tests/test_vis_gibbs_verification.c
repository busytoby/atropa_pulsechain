#include "tsfi_fourier.h"
#include "tsfi_font_vectors.h"
#include "lau_memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <string.h>

// Helper: Distance from point P to line segment AB
float dist_to_segment(float px, float py, float ax, float ay, float bx, float by) {
    float dx = bx - ax;
    float dy = by - ay;
    float l2 = dx*dx + dy*dy;
    if (l2 == 0.0f) return sqrtf((px-ax)*(px-ax) + (py-ay)*(py-ay));
    float t = ((px - ax) * dx + (py - ay) * dy) / l2;
    t = fmaxf(0.0f, fminf(1.0f, t));
    float closest_x = ax + t * dx;
    float closest_y = ay + t * dy;
    return sqrtf((px - closest_x)*(px - closest_x) + (py - closest_y)*(py - closest_y));
}

int main() {
    printf("[TEST] Visual Gibbs Verification (Banach-Hilbert Isometry)...\n");

    // 1. Define a Sharp Banach Shape (Square)
    // Coords in verified TSFi range (-16k to 16k)
    // Perfectly centered.
    float original_poly[] = {
        -10000.0f, -10000.0f,
         10000.0f, -10000.0f,
         10000.0f,  10000.0f,
        -10000.0f,  10000.0f
    };
    size_t poly_count = 4;

    // Resample poly to more points to help the simple DFT fallback
    // Must be at least 2x Harmonics (Nyquist)
    float resampled_poly[4096 * 2];
    int resample_n = 4096;
    for(int i=0; i<resample_n; i++) {
        float t = (float)i / resample_n;
        // Simple linear interpolation for square sides
        if (t < 0.25f) { // Bottom
            float s = t / 0.25f;
            resampled_poly[i*2+0] = -10000.0f + s * 20000.0f;
            resampled_poly[i*2+1] = -10000.0f;
        } else if (t < 0.5f) { // Right
            float s = (t - 0.25f) / 0.25f;
            resampled_poly[i*2+0] = 10000.0f;
            resampled_poly[i*2+1] = -10000.0f + s * 20000.0f;
        } else if (t < 0.75f) { // Top
            float s = (t - 0.5f) / 0.25f;
            resampled_poly[i*2+0] = 10000.0f - s * 20000.0f;
            resampled_poly[i*2+1] = 10000.0f;
        } else { // Left
            float s = (t - 0.75f) / 0.25f;
            resampled_poly[i*2+0] = -10000.0f;
            resampled_poly[i*2+1] = 10000.0f - s * 20000.0f;
        }
    }

    // 2. Map to Hilbert Space (Analysis)
    TSFiFourierGlyph glyph;
    tsfi_fourier_from_points(&glyph, resampled_poly, resample_n);
    printf("[INFO] Banach -> Hilbert Conversion Complete (N=%d Harmonics).\n", TSFI_FOURIER_HARMONICS);

    // 3. Map back to Banach Space (Synthesis)
    TSFiFourierBasis basis;
    tsfi_fourier_init_basis(&basis);
    float reconstructed[TSFI_FOURIER_SAMPLES * 2];
    tsfi_fourier_reconstruct_avx512(reconstructed, &basis, &glyph);
    printf("[INFO] Hilbert -> Banach Reconstruction Complete (K=%d Samples).\n", TSFI_FOURIER_SAMPLES);

    // 4. Quantify Error (The Gibbs Gap)
    // For each reconstructed point, find min distance to original polygon edges.
    float max_error = 0.0f;
    for (int i = 0; i < TSFI_FOURIER_SAMPLES; i++) {
        float px = reconstructed[i*2 + 0];
        float py = reconstructed[i*2 + 1];
        
        float min_dist = 1e9f;
        for (size_t j = 0; j < poly_count; j++) {
            size_t next = (j + 1) % poly_count;
            float d = dist_to_segment(px, py, 
                                      original_poly[j*2+0], original_poly[j*2+1],
                                      original_poly[next*2+0], original_poly[next*2+1]);
            if (d < min_dist) min_dist = d;
        }
        if (min_dist > max_error) max_error = min_dist;
    }

    printf("[INFO] Detected Peak Gibbs Error: %.2f units\n", max_error);

    // 5. Verification against Rasterizer Thresholds
    // Default Style: Thickness=400, Softness=200. Total Window=600.
    float tolerance = 400.0f + 200.0f;
    printf("[CHECK] Tolerance (Thickness + Softness): %.2f units\n", tolerance);

    if (max_error <= tolerance) {
        printf("[PASS] Gibbs Ringing is within visual blending bounds.\n");
    } else {
        printf("[FAIL] Gibbs Ringing (%.2f) exceeds visual threshold (%.2f)!\n", max_error, tolerance);
        printf("[FAIL] Aesthetics Compromised: Ringing will be visible outside the edge blur.\n");
        return 1;
    }

    printf("=== GIBBS VERIFICATION PASSED ===\n");
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}