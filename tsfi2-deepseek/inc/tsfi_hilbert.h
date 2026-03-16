#ifndef TSFI_HILBERT_H
#define TSFI_HILBERT_H

#include <stdint.h>
#include <stddef.h>

// TSFi Hilbert Space Font System
// Representation of glyphs as 2D Logical Density Fields.

#define TSFI_HILBERT_ORDER 8 // N (Low order for proof of concept)
// Matrix size = (2N+1) x (2N+1)
#define TSFI_HILBERT_DIM (2 * TSFI_HILBERT_ORDER + 1)

// Complex Coefficient
typedef struct {
    float real;
    float imag;
} TSFiComplex;

// TSFi Math Profile (Minimax Coefficients)
// Stored in the first 16 SGPR slots.
typedef struct {
    float domain_scaler; // 1/(2*PI)
    float cos_coeffs[4]; // c0, c2, c4, c6
    float sin_coeffs[4]; // s1, s3, s5, s7
    float padding[7];    // Pad to 16 slots (64 bytes)
} TSFiMathProfile;

// The Glyph "State Vector"
// Stored as a Matrix of Coefficients C_nm
// n (rows) ranges -N..N, m (cols) ranges -N..N
// Indexing: [n + N][m + N]
// TERMINUS: Realizes EXCLUSIVELY as Voxels (Feynman Points).
typedef struct {
    TSFiComplex coeffs[TSFI_HILBERT_DIM][TSFI_HILBERT_DIM];
} TSFiHilbertGlyph;

// Basis Cache (Optional for CPU eval)
typedef struct {
    // Precomputed e^(ikx) for range x=-PI..PI
    // Not strictly needed if evaluating on the fly, but good for speed.
} TSFiHilbertBasis;

// API

// Initialize a "Null" glyph (Vacuum state)
void tsfi_hilbert_init_glyph(TSFiHilbertGlyph *g);

// Project a Box shape into the Hilbert Space
// (Analytic Fourier Transform of a Rect function)
// box_w, box_h: Dimensions relative to period 2PI
void tsfi_hilbert_project_box(TSFiHilbertGlyph *g, float width, float height);

// Reconstruct Field Intensity at (x, y)
// Returns |\psi(x,y)|^2
float tsfi_hilbert_eval(const TSFiHilbertGlyph *g, float x, float y);

// "WMMA" Reconstruction
// Reconstruct a 16x16 pixel tile
// out_buffer: 16x16 float array
// out_buffer: 16x16 float array
// secret_x, secret_y: Top-Left corner coordinates
// scale: Scale factor for coordinates
void tsfi_hilbert_reconstruct_tile(float *out_buffer, const TSFiHilbertGlyph *g, 
                                   float secret_x, float secret_y, float scale);

// Batch Evaluation (AVX-512)
void tsfi_hilbert_batch_avx512(void *ctx, const float *xy_in, float *out, int count);
void tsfi_hilbert_batch_wide_avx512(void *ctx, const float *xy_in, float *out, int count);

// AlphaFold Kernel: Fold Banach Sequence to Hilbert Coefficients (AVX-512)
// seq: Byte sequence (DNA/Amino)
// len: Length of sequence (bytes)
void tsfi_hilbert_fold_sequence_avx512(TSFiHilbertGlyph *g, const char *seq, size_t len);

#endif
