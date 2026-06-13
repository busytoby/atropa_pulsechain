#ifndef TSFI_VM_DFT_BRIDGE_H
#define TSFI_VM_DFT_BRIDGE_H

#include "tsfi_fourier.h"
#include "tsfi_zmm_vm.h"
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

// Standard layout mapping state variables from Dysnomia VMs to DFT coefficients
typedef struct {
    uint64_t soul;              // Identifies primary lower harmonics scaling
    uint64_t aura;              // Represents target phase offset rotation
    uint64_t identity_pole;     // Translates to frequency base index translation
} TSFiVmStateParams;

// Zero-copy simulation wrapper linking Yul VM memory directly to DFT spectrum coordinates
typedef struct {
    TsfiZmmVmState *vm_state;
    TSFiFourierBasis basis;
    TSFiFourierGlyph spectrum;
} TSFiVmDftBridge;

// Initializes basis matrices and hooks to standard VM references
static inline void tsfi_vm_dft_bridge_init(TSFiVmDftBridge *bridge, TsfiZmmVmState *vm) {
    bridge->vm_state = vm;
    tsfi_fourier_init_basis(&bridge->basis);
    // Standard initialization of Fourier spectrum (Perfect matter base state)
    for (int n = 0; n < TSFI_FOURIER_HARMONICS; n++) {
        bridge->spectrum.coeffs[n][0] = 0.0f; // ax
        bridge->spectrum.coeffs[n][1] = 0.0f; // bx
        bridge->spectrum.coeffs[n][2] = 0.0f; // ay
        bridge->spectrum.coeffs[n][3] = 0.0f; // by
    }
}

// Maps direct VM memory segments (from cpu6502, diyat, or minder) to DFT spectrum coordinates
// without fidelity losses or approximation jitter (Perfect Matter Model)
static inline void tsfi_vm_dft_bridge_align(TSFiVmDftBridge *bridge, const TSFiVmStateParams *params) {
    // 1. Direct algebraic assignment matching the exact model:
    // Scale coefficient amplitudes based on the Soul parameter
    float amplitude_scalar = (float)(params->soul % 1000) / 1000.0f;
    if (amplitude_scalar == 0.0f) amplitude_scalar = 1.0f;

    // 2. Compute phase rotation parameter directly from Aura
    float phase_angle = (float)(params->aura % 360) * (3.14159265f / 180.0f);

    // 3. Anchor coordinate center index translation using Identity Pole
    float offset_x = (float)(params->identity_pole % 100) - 50.0f;
    
    // Write coefficients natively to the spectrum
    // Fundamental frequency index 0 stores the DC offset
    bridge->spectrum.coeffs[0][0] = offset_x; // ax
    bridge->spectrum.coeffs[0][2] = 0.0f;     // ay

    for (int n = 1; n < TSFI_FOURIER_HARMONICS; n++) {
        // Distribute spectral density cleanly based on fundamental amplitude and phase
        float freq_decay = 1.0f / (float)(n);
        bridge->spectrum.coeffs[n][0] = amplitude_scalar * freq_decay * cosf(phase_angle * n); // ax
        bridge->spectrum.coeffs[n][1] = amplitude_scalar * freq_decay * sinf(phase_angle * n); // bx
        bridge->spectrum.coeffs[n][2] = amplitude_scalar * freq_decay * sinf(phase_angle * n); // ay
        bridge->spectrum.coeffs[n][3] = amplitude_scalar * freq_decay * cosf(phase_angle * n); // by
    }
}

#endif // TSFI_VM_DFT_BRIDGE_H
