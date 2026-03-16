#ifndef TSFI_SPECTRUM_H
#define TSFI_SPECTRUM_H

#include "tsfi_wave_any.h"

// --- The Logical Atom ---
// A 16-byte point in the wave field. Edges are encoded as relative secrets.
typedef struct {
    float value;      // Local Signal
    float secret;     // Intrinsic Modulation
    int16_t hop_1;    // Relative Harmonic secret A (Parent 1)
    int16_t hop_2;    // Relative Harmonic secret B (Parent 2)
    uint16_t mode;    // Logical Op Mode
    uint16_t pad;     // Entropy/Padding
} SpectrumAtom;

// --- The Wave Field ---
typedef WaveStream WaveField;

#endif
