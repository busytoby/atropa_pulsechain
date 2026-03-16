#ifndef TSFI_WAVE_INTEROP_H
#define TSFI_WAVE_INTEROP_H

#include "tsfi_wave512.h"

// Deny Simulation: Ensure we are using GCC vector extensions or inline asm
#if !defined(__x86_64__) && !defined(__i386__)
#error "WAVE512 requires x86-64 AVX-512 hardware. Simulation denied."
#endif

// Mapping: Neural Sample (16 floats) -> Fits in 1st ZMM of wave512
// But user wants "entire data set in single WAVE512".
// Let's define a 'WaveletParticipant' that uses the full 512 bytes.
// 8 Banks of 16 floats (512 bytes total)
// Bank 0: Weights 
// Bank 1: Gradients 
// Bank 2: Momentum 
// Bank 3: Mask 
// Bank 4-7: Scratch / History

typedef union {
    wave512 raw;
    struct {
        float weights[16];
        float gradients[16];
        float momentum[16];
        float mask[16];
        float history[4][16];
    } named;
} WaveletParticipant;

// Zero Copy Transfer: Just a pointer cast
#define TO_WAVE(p) ((wave512*)(p))
#define TO_PARTICIPANT(w) ((WaveletParticipant*)(w))

// Thunk Type: Operates on Wavelets (A + B -> Res)
typedef void (*WaveletThunk)(wave512*, const wave512*, const wave512*);

#endif
