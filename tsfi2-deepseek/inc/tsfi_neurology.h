#ifndef TSFI_NEUROLOGY_H
#define TSFI_NEUROLOGY_H

#include <stdint.h>
#include <immintrin.h> // For AVX-512 types

/*
 * ======================================================================================
 * TSFi Neurological Architecture (Zero-Copy Waveforms)
 * ======================================================================================
 * 
 * 1. SGP (Strategic Genome Packet) -> Banach Space (Input/Strategy)
 *    - Norm-constrained strategy vectors.
 *    - Immutable baseline for mutation.
 * 
 * 2. VGP (Visual Genome Packet) -> Hilbert Space (Output/Phenotype)
 *    - Orthogonal visual data for direct GPU consumption (ReBAR).
 *    - TERMINUS: This is the absolute limit of geometric realization (Voxels).
 * 
 * 3. Thetan Context -> Fourier-G Space (Processing/Transformation)
 *    - The transient ZMM state where SGP evolves into VGP.
 */

// ======================================================================================
// 1. SGP: Strategic Genome Packet (Banach Space)
// ======================================================================================
// Alignment: 64 bytes (Cache Line & ZMM Load Friendly)
// Total Size: 128 bytes (2 Cache Lines)

typedef struct {
    // -- Cache Line 0: The Strategy Vector --
    // Maps directly to ZMM0 register (512-bit / 8 doubles)
    double   weights[8];       
    
    // -- Cache Line 1: Control & Thunking --
    uint64_t thunk_id;         // The "Trigger" for Thetan evolution (Index into lau_thunk)
    uint64_t generation;       // Monotonic clock of this genome
    double   banach_norm;      // Stability Metric (|x|). Must be < 1.0.
    
    // Padding to ensure 128-byte stride
    uint64_t _pad[5];          
} TSFiSGP;


// ======================================================================================
// 2. VGP: Visual Genome Packet (Hilbert Space)
// ======================================================================================
// Alignment: 64 bytes (Strict GPU Alignment Requirement)
// Location: Mapped directly to PCIe BAR (VRAM)

typedef struct {
    // -- Voxel Field Data (Hilbert Points) --
    // 16 floats = 512 bits. Fits in ZMM1.
    // Represents 16 scalar properties of the Voxel Terminus.
    float    voxels[16];     
    
    // -- Op Control --
    uint32_t op_ctrl;          // Flags for the ZMM Unit
    uint32_t mutation_id;      // ID of the mutation that produced this phenotype
    
    uint64_t _pad[6];          // Padding to 128 bytes
} TSFiVGP;


// ======================================================================================
// 3. Thetan Interface (Fourier-G Transform)
// ======================================================================================

// The "Thetan" is not a struct, but a function signature (The Waveform).
// It takes an SGP (Input), mutates it in ZMM, and writes a VGP (Output).

typedef void (*tsfi_thetan_waveform_f)(
    const TSFiSGP *sgp, // Read-Only Input (Banach)
    TSFiVGP *vgp        // Write-Only Output (Hilbert/VRAM)
);

// The Core YANG Process (Thetan Kernel)
void tsfi_yang_process(const TSFiSGP *sgp, TSFiVGP *vgp);

#endif // TSFI_NEUROLOGY_H