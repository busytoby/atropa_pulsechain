#include "tsfi_fourier.h"
#include "tsfi_c_math.h"
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "lau_memory.h"
#include "lau_thunk.h"
#include "tsfi_resonance.h"
#include <sys/mman.h>

static void fourier_reconstruct_stub(const TSFiFourierGlyph *g, float *out, const TSFiFourierBasis *b) {
    tsfi_fourier_reconstruct_avx512(out, b, g);
}

int main() {
    printf("[TEST] Fourier-G (Wave Glyph) System...\n");

    // 1. Init Basis
    TSFiFourierBasis basis;
    tsfi_fourier_init_basis(&basis);

    // 2. Define Coordinate Set (High-Resolution Signal)
    // Must meet Nyquist criteria for TSFI_FOURIER_HARMONICS (32) to avoid aliasing.
    float poly[TSFI_FOURIER_SAMPLES * 2];
    for(int i=0; i<TSFI_FOURIER_SAMPLES; i++) {
        float t = (2.0f * 3.1415926535f * i) / TSFI_FOURIER_SAMPLES;
        poly[i*2+0] = 50.0f * tsfi_cosf(t) + 10.0f * tsfi_cosf(3.0f * t);
        poly[i*2+1] = 50.0f * tsfi_sinf(t) - 5.0f * tsfi_sinf(2.0f * t);
    }

    // 3. Convert to Spectrum
    // Allocate the glyph as a Zero-Copy Executed Vector Matrix using the wired registry
    TSFiFourierGlyph *glyph = (TSFiFourierGlyph*)lau_malloc_wired(sizeof(TSFiFourierGlyph));
    assert(glyph);
    
    // Extract the zero-copy proxy header
    LauWiredHeader *h = (LauWiredHeader *)((char *)glyph - offsetof(LauWiredHeader, payload));
    if (!h->proxy) h->proxy = ThunkProxy_create();
    
    // Bind JIT synthesis execution hook for autonomous processing
    // We use a baked mixed emit to map the reconstruction directly to the memory bounds
    void (*jit_reconstruct)(float*, const TSFiFourierBasis*) = (void(*)(float*, const TSFiFourierBasis*))ThunkProxy_emit_forwarding(h->proxy, (void*)fourier_reconstruct_stub, glyph);
    lau_mprotect(h->proxy->thunk_pool, PROT_READ | PROT_EXEC);

    tsfi_fourier_from_points(glyph, poly, TSFI_FOURIER_SAMPLES);
    
    printf("      Harmonic 1: A=%.2f B=%.2f\n", glyph->coeffs[0][0], glyph->coeffs[0][1]);

    // 4. Reconstruct via JIT synthesis (Zero-Copy)
    float reconstructed[TSFI_FOURIER_SAMPLES * 2];
    jit_reconstruct(reconstructed, &basis); // This implicitly passes the captured 'glyph' as the third param

    // 5. Verify bounds
    float min_x = 1e9, max_x = -1e9;
    for(int i=0; i<TSFI_FOURIER_SAMPLES; i++) {
        float x = reconstructed[i*2+0];
        if(x < min_x) min_x = x;
        if(x > max_x) max_x = x;
    }
    printf("      Reconstructed Voxel Bounds X: %.2f to %.2f\n", min_x, max_x);
    
    // Vetting Bijective Parity: Encode back from Reconstruction
    TSFiFourierGlyph bijective_glyph;
    tsfi_fourier_from_points(&bijective_glyph, reconstructed, TSFI_FOURIER_SAMPLES);
    printf("      Bijection Harmonic 1: A=%.2f B=%.2f\n", bijective_glyph.coeffs[0][0], bijective_glyph.coeffs[0][1]);
    
    // Calculate mathematical variance (loss) between initial frequency spectrum and bijective reconstructed spectrum
    float error_acc = 0.0f;
    for(int n=0; n<TSFI_FOURIER_HARMONICS; n++) {
        for(int c=0; c<4; c++) {
            float diff = glyph->coeffs[n][c] - bijective_glyph.coeffs[n][c];
            error_acc += diff * diff;
        }
    }
    
    float mse = error_acc / (TSFI_FOURIER_HARMONICS * 4);
    printf("      Bijective Matrix Loss (MSE): %f\n", mse);

    // Fourier approximation of square usually overshoots slightly (Gibbs) or undershoots.
    assert(max_x > 40.0f); 
    assert(min_x < -40.0f);

    printf("[TEST] Passed.\n");
    
    lau_unseal_object(glyph);
    lau_free(glyph);

    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
