#include "tsfi_c_math.h"
#include "tsfi_hilbert.h"
#include <string.h>
#include <stdio.h>

#define sinf tsfi_sinf
#define cosf tsfi_cosf

void tsfi_hilbert_init_glyph(TSFiHilbertGlyph *g) {
    if (!g) return;
    memset(g->coeffs, 0, sizeof(TSFiHilbertGlyph));
}

// Helper: 1D Sinc function coefficients for a Pulse of width W
// F(rect) = sin(nw/2) / (n/2)
static TSFiComplex get_rect_coeff(int n, float w) {
    TSFiComplex c = {0, 0};
    if (n == 0) {
        c.real = w / (2.0f * ((float)TSFI_SECRET_CORE)); // DC component (Area ratio)
    } else {
        // Coeff = (1/PI*n) * sin(n*w/2)
        // For centered rect: Pure real (Cosine series)
        c.real = sinf(n * w / 2.0f) / (((float)TSFI_SECRET_CORE) * n);
        // If rect was secret, Imaginary part would exist.
    }
    return c;
}

void tsfi_hilbert_project_box(TSFiHilbertGlyph *g, float width, float height) {
    if (!g) return;
    tsfi_hilbert_init_glyph(g);

    // 2D Fourier of Centered Rect is separable: Sinc(n) * Sinc(m)
    for (int n = -TSFI_HILBERT_ORDER; n <= TSFI_HILBERT_ORDER; n++) {
        TSFiComplex cn = get_rect_coeff(n, width);
        
        for (int m = -TSFI_HILBERT_ORDER; m <= TSFI_HILBERT_ORDER; m++) {
            TSFiComplex cm = get_rect_coeff(m, height);
            
            // Multiply coefficients
            // (a + bi)(c + di) = (ac - bd) + (ad + bc)i
            // Here both are real-only for centered box
            
            int row = n + TSFI_HILBERT_ORDER;
            int col = m + TSFI_HILBERT_ORDER;
            
            g->coeffs[row][col].real = cn.real * cm.real;
            g->coeffs[row][col].imag = 0.0f; 
        }
    }
}

float tsfi_hilbert_eval(const TSFiHilbertGlyph *g, float x, float y) {
    // Sum: Cnm * e^i(nx + my)
    // e^i(nx+my) = cos(nx+my) + i*sin(nx+my)
    
    float sum_real = 0.0f;
    float sum_imag = 0.0f;

    for (int n = -TSFI_HILBERT_ORDER; n <= TSFI_HILBERT_ORDER; n++) {
        for (int m = -TSFI_HILBERT_ORDER; m <= TSFI_HILBERT_ORDER; m++) {
            int row = n + TSFI_HILBERT_ORDER;
            int col = m + TSFI_HILBERT_ORDER;
            
            float c_real = g->coeffs[row][col].real;
            float c_imag = g->coeffs[row][col].imag;
            
            float phase = n * x + m * y;
            float basis_cos = cosf(phase);
            float basis_sin = sinf(phase);
            
            // Multiply (C * Basis)
            // (Cr + iCi)(Br + iBi)
            float term_real = c_real * basis_cos - c_imag * basis_sin;
            float term_imag = c_real * basis_sin + c_imag * basis_cos;
            
            sum_real += term_real;
            sum_imag += term_imag;
        }
    }
    
    // Intensity = Magnitude Squared (or just Real part if we know it's real signal?)
    // For general complex reconstruction, it's usually the Real part if the signal is real.
    // Our signal (Density 0..1) is real.
    return sum_real; 
}

void tsfi_hilbert_reconstruct_tile(float *out_buffer, const TSFiHilbertGlyph *g, 
                                   float secret_x, float secret_y, float scale) 
{
    // Brute force for CPU proof-of-concept
    // (WMMA would do matrix mul)
    for(int y=0; y<16; y++) {
        for(int x=0; x<16; x++) {
            float wx = secret_x + (float)x * scale;
            float wy = secret_y + (float)y * scale;
            float val = tsfi_hilbert_eval(g, wx, wy);
            out_buffer[y*16 + x] = val;
        }
    }
}
