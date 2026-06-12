#include "tsfi_fourier.h"
#include "tsfi_c_math.h"
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "lau_memory.h"
#include "lau_thunk.h"

int main() {
    printf("[TEST] Walsh-G (Digital Wave Glyph) System...\n");

    // 1. Init Walsh Basis
    TSFiFourierBasis basis;
    tsfi_walsh_init_basis(&basis);

    // 2. Define flat coordinate set
    float poly[TSFI_FOURIER_SAMPLES * 2];
    for(int i=0; i<TSFI_FOURIER_SAMPLES; i++) {
        poly[i*2+0] = (i < TSFI_FOURIER_SAMPLES / 2) ? 10.0f : -10.0f; // Square wave
        poly[i*2+1] = (i % 4 < 2) ? 5.0f : -5.0f;
    }

    // 3. Convert to Spectrum
    TSFiFourierGlyph glyph;
    tsfi_fourier_from_points(&glyph, poly, TSFI_FOURIER_SAMPLES);
    
    printf("      Walsh Coefficient 1: %.2f\n", glyph.coeffs[0][0]);

    // 4. Reconstruct using AVX-512 Walsh basis
    float reconstructed[TSFI_FOURIER_SAMPLES * 2];
    tsfi_fourier_reconstruct_avx512(reconstructed, &basis, &glyph);

    // 5. Verify bounds and output
    float min_x = 1e9, max_x = -1e9;
    for(int i=0; i<TSFI_FOURIER_SAMPLES; i++) {
        float x = reconstructed[i*2+0];
        if(x < min_x) min_x = x;
        if(x > max_x) max_x = x;
    }
    printf("      Walsh Reconstructed Bounds X: %.2f to %.2f\n", min_x, max_x);

    assert(max_x > 0.0f);
    assert(min_x < 0.0f);
    printf("[TEST] Walsh Synthesis verified successfully.\n");
    
    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    return 0;
}
