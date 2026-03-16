#ifndef TSFI_THUNKS_H
#define TSFI_THUNKS_H

#include "tsfi_wave_any.h"
#include "tsfi_dysnomia.h"
#include "tsfi_dysnomia_crc.h"

// --- Uniform Structures ---

typedef struct {
    float intensity;
    float decay;
} WaveShaderUniforms;

typedef struct {
    float t;
    float secret;
    uint32_t atom_count;
    uint32_t pad;
} WaveMorphUniforms;

// --- Thunk Entry Points (CPU) ---

// thunks/wave_shader.c
// Implements: v = v * intensity - decay
void __attribute__((weak)) wave_shader_main(const WaveStream* stream, const WaveShaderUniforms* u);

// thunks/wave_morph.c
// Implements: v = v * t + secret
void __attribute__((weak)) wave_morph_main(const WaveStream* stream, const WaveMorphUniforms* u);

// thunks/dysnomia_reactive_crc.c
// Implements: Dysnomia CRC (State Mutation)
// Note: declared in tsfi_dysnomia_crc.h, but re-listed here for completeness if needed.

#endif // TSFI_THUNKS_H
