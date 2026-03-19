#ifndef TSFI_FOURIER_H
#define TSFI_FOURIER_H

#include <stdint.h>
#include <stddef.h>

// TSFi Fourier-G (Wave Glyph) Format
// Represents a glyph contour as a sum of N frequencies.
// Optimized for WMMA reconstruction: V = Basis * Coeffs.

#define TSFI_FOURIER_HARMONICS 1024 // Number of frequencies (N) - Ultra-High Res
#define TSFI_FOURIER_SAMPLES 512   // Resolution of reconstruction (K) - Scaled for Wave512

// The Glyph Data (The "Spectrum")
// 16 Harmonics * 4 coeffs (Ax, Bx, Ay, By) * float = 256 bytes per glyph.
// Fits in 4 cache lines.
typedef struct {
    float coeffs[TSFI_FOURIER_HARMONICS][4]; // [n][0]=an, [1]=bn (for X), [2]=cn, [3]=dn (for Y)
} TSFiFourierGlyph;

// The Basis Matrix (Constant ROM)
// K rows x (2*N + 1) cols. 
// Precomputed cos(nt), sin(nt).
typedef struct {
    float data[TSFI_FOURIER_SAMPLES][TSFI_FOURIER_HARMONICS * 2];
} TSFiFourierBasis;

// API
void tsfi_fourier_init_basis(TSFiFourierBasis *basis);

// Generate Voxel Field from Spectrum (Simulating WMMA)
// output_voxels: float buffer [SAMPLES * 2] (x,y interleaved)
void tsfi_fourier_reconstruct_avx512(float *output_voxels, 
                                     const TSFiFourierBasis *basis, 
                                     const TSFiFourierGlyph *glyph);

// Converter: Coordinate Set -> Fourier
// Approximates a closed loop of points with Fourier descriptors
void tsfi_fourier_from_points(TSFiFourierGlyph *out, const float *xy, size_t count);

// Spectral Puppetry / Kinematics
// Applies a spatial affine translation (shifting all points uniformly by x_offset, y_offset)
void tsfi_fourier_translate(TSFiFourierGlyph *glyph, float x_offset, float y_offset);

// Applies a high-frequency phase shift to simulate local articulation (e.g. moving a beak)
void tsfi_fourier_articulate(TSFiFourierGlyph *glyph, float phase_shift, int start_harmonic, int end_harmonic);

// Applies a low-pass filter to simulate sickness or melting (attenuates high frequencies)
void tsfi_fourier_melt(TSFiFourierGlyph *glyph, float sickness_factor);

#include "tsfi_svdag.h"

// Smoothly interpolates between two spectral states (for complex animations like flying)
void tsfi_fourier_interpolate(TSFiFourierGlyph *out, const TSFiFourierGlyph *a, const TSFiFourierGlyph *b, float t);

// Autonomy: Fault Heuristics and Energy Growth
void tsfi_grow_fault_energy(void *manifold_shm, float weight);

/**
 * @brief Solidifies a Fourier Isomorphism into a Helmholtz Working Model.
 * 
 * Logic:
 * 1. Reconstructs the Voxel Field (Banach) from the Spectrum (Hilbert).
 * 2. Compiles the resulting coordinates into an SVDAG (Taste Tree).
 * 3. Uses the XNOR Trap to verify bitwise isomorphism against a target manifold.
 * 4. On success, anchors the SVDAG as an active Helmholtz Wave System.
 */
void tsfi_fourier_solidify_helmholtz(TSFiHelmholtzSVDAG *dag, 
                                     const TSFiFourierBasis *basis, 
                                     const TSFiFourierGlyph *spectrum,
                                     void *manifold_shm,
                                     const uint8_t *target_isomorphism_hash);

#endif